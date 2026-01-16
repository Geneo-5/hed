/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include "hed/server.h"

#include <utils/signal.h>
#include <utils/timer.h>

static int __hed_nonull(1, 3)
hed_srv_dispatch_sigchan(struct upoll_worker * work,
                         uint32_t              state __unused,
                         const struct upoll *  poll __unused)
{
	hed_assert_intern(work);
	hed_assert_intern(state);
	hed_assert_intern(!(state & EPOLLOUT));
	hed_assert_intern(!(state & EPOLLRDHUP));
	hed_assert_intern(!(state & EPOLLPRI));
	hed_assert_intern(!(state & EPOLLHUP));
	hed_assert_intern(!(state & EPOLLERR));
	hed_assert_intern(state & EPOLLIN);
	hed_assert_intern(poll);

	const struct hed_server * srv;
	struct signalfd_siginfo   info;
	int                       ret;

	srv = containerof(work, struct hed_server, sig_worker);
	hed_assert_intern(srv);

	ret = usig_read_fd(srv->sig_fd, &info, 1);
	hed_assert_intern(ret);
	if (ret < 0)
		return (ret == -EAGAIN) ? 0 : ret;

	switch (info.ssi_signo) {
	case SIGHUP:
	case SIGINT:
	case SIGQUIT:
	case SIGTERM:
		/* Tell caller we were requested to terminate. */
		return -ESHUTDOWN;

	case SIGUSR1:
	case SIGUSR2:
		/* Silently ignore these... */
		return 0;

	default:
		hed_assert_intern(0);
	}

	return ret;
}

static int  __hed_nonull(1)
hed_srv_open_sigchan(struct hed_server *srv)
{
	hed_assert_intern(srv);

	sigset_t     msk = *usig_empty_msk;
	sigset_t     blk = *usig_full_msk;
	int          err;

	usig_addset(&msk, SIGHUP);
	usig_addset(&msk, SIGINT);
	usig_addset(&msk, SIGQUIT);
	usig_addset(&msk, SIGTERM);
	usig_addset(&msk, SIGUSR1);
	usig_addset(&msk, SIGUSR2);
	srv->sig_fd = usig_open_fd(&msk, SFD_NONBLOCK | SFD_CLOEXEC);
	if (srv->sig_fd < 0)
		return srv->sig_fd;

	srv->sig_worker.dispatch = hed_srv_dispatch_sigchan;
	err = upoll_register(&srv->poll, srv->sig_fd, EPOLLIN, &srv->sig_worker);
	if (err)
		goto close;

	usig_delset(&blk, SIGCONT);
	usig_delset(&blk, SIGTSTP);
	usig_delset(&blk, SIGTRAP);
	usig_delset(&blk, SIGTTIN);
	usig_delset(&blk, SIGTTOU);
	usig_procmask(SIG_SETMASK, &blk, NULL);

	return 0;

close:
	usig_close_fd(srv->sig_fd);
	return err;
}

static void  __hed_nonull(1)
hed_srv_close_sigchan(struct hed_server *srv)
{
	hed_assert_api(srv);

	upoll_unregister(&srv->poll, srv->sig_fd);
	usig_close_fd(srv->sig_fd);
}


int
hed_srv_init(struct hed_server                 *srv,
             char                              *path,
             const struct galv_rpc_accept_conf *conf,
             const struct hed_rpc_factory      *factory)
{
	hed_assert_api(srv);
	hed_assert_api(path);
	hed_assert_api(conf);

	int ret;
	struct galv_unix_adopt_conf unix_conf;

	galv_unix_adopt_config(&unix_conf, SOCK_STREAM, SOCK_CLOEXEC,
	                       path, CONFIG_HED_CONN_NR);
	galv_repo_init(&srv->repo, CONFIG_HED_CONN_NR);

	ret = galv_unix_adopt_open(&srv->adopt, GALV_GATE_DUMMY, &unix_conf);
	if (ret)
		goto fini;

	ret = upoll_open(&srv->poll, CONFIG_HED_CONN_NR + 1);
	if (ret)
		goto close_adopt;

	ret = galv_rpc_open_accept(&srv->accept,
	                           &factory->base,
	                           &srv->repo,
	                           (struct galv_adopt *)&srv->adopt,
	                           &srv->poll,
	                           conf);
	if (ret)
		goto close_poll;

	ret = hed_srv_open_sigchan(srv);
	if (!ret)
		return 0;

	hed_srv_close_sigchan(srv);
close_poll:
	upoll_close(&srv->poll);
close_adopt:
	galv_unix_adopt_close(&srv->adopt);
fini:
	galv_repo_fini(&srv->repo);
	return ret;
}

int
hed_srv_process(struct hed_server *srv)
{
	hed_assert_api(srv);

	int ret;
	int tmout;

	tmout = etux_timer_issue_msec();
	ret = upoll_wait(&srv->poll, tmout);
	if ((ret == -ETIME) || !tmout) {
		/* Expire timers. */
		etux_timer_run();
		return 0;
	}

	if (ret <= 0)
		return ret;

	return upoll_dispatch(&srv->poll, (unsigned int)ret);
}

int
hed_srv_run(struct hed_server *srv)
{
	hed_assert_api(srv);

	int ret;

	do {
		ret = hed_srv_process(srv);
	} while (!ret || (ret == -EINTR));
	switch (ret) {
	case -ESHUTDOWN:
	case -EINTR:
		ret = 0;
		break;
	}

	return ret;
}

int
hed_srv_halt(struct hed_server *srv)
{
	hed_assert_api(srv);

	int ret;

	galv_accept_suspend((struct galv_accept *)&srv->accept, &srv->poll);
	galv_conn_repo_halt(&srv->repo, &srv->poll);
	while (!galv_repo_empty(&srv->repo)) {
		/*
		 * To be safe, a timer should be armed here to prevent from
		 * blocking into epoll_wait() forever...
		 */
		ret = upoll_process(&srv->poll, -1);
		if (ret)
			break;
	}
	if (ret == -ESHUTDOWN)
		ret = -EINTR;

	galv_conn_repo_close(&srv->repo, &srv->poll);
	return ret;
}


void
hed_srv_fini(struct hed_server *srv)
{
	hed_assert_api(srv);

	hed_srv_close_sigchan(srv);
	galv_rpc_close_accept(&srv->accept, &srv->poll);
	upoll_close(&srv->poll);
	galv_unix_adopt_close(&srv->adopt);
	galv_repo_fini(&srv->repo);
}

