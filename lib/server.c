/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#include "hed/server.h"


static int
hed_srv_dispatch_sigchan(struct upoll_worker * work,
                         uint32_t              state __unused,
                         const struct upoll *  poll __unused)
{
	hed_assert(work);
	hed_assert(state);
	hed_assert(!(state & EPOLLOUT));
	hed_assert(!(state & EPOLLRDHUP));
	hed_assert(!(state & EPOLLPRI));
	hed_assert(!(state & EPOLLHUP));
	hed_assert(!(state & EPOLLERR));
	hed_assert(state & EPOLLIN);
	hed_assert(poll);

	const struct hed_server * srv;
	struct signalfd_siginfo   info;
	int                       ret;

	srv = containerof(work, struct hed_server, sig_work);
	hed_assert(chan);

	ret = usig_read_fd(srv->sig_fd, &info, 1);
	hed_assert(ret);
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
		galvsmpl_assert(0);
	}

	return ret;
}

static int  __hed_nonull(1)
hed_srv_open_sigchan(struct hed_server *srv)
{
	hed_assert(srv);

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

	srv->sig_work.dispatch = hed_srv_dispatch_sigchan;
	err = upoll_register(&srv->poll, srv->sig_fd, EPOLLIN, &srv->sig_work);
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

static int  __hed_nonull(1)
hed_srv_close_sigchan(struct hed_server *srv)
{
	hed_assert(srv);

	upoll_unregister(&srv->poll, srv->sig_fd);
	usig_close_fd(srv->sig_fd);
}


int __hed_nonull(1, 2, 3)
hed_srv_init(struct hed_server                *srv,
             char                             *path,
             const struct hed_rpc_accept_conf *conf)
{
	hed_assert(srv);
	hed_assert(path);
	hed_assert(conf);

	int ret;
	struct galv_unix_adopt_conf unix_conf = \
		GALV_UNIX_ADOPT_CONF(SOCK_STREAM, SOCK_CLOEXEC,
		                     path, CONFIG_HED_CONN_NR);

	galv_repo_init(&srv->repo, CONFIG_HED_CONN_NR)
 
	ret = galv_unix_adopt_open(&srv->adopt, GALV_GATE_DUMMY, &unix_conf);
	if (ret)
		goto fini;

	ret = upoll_open(&srv->poll, CONFIG_HED_CONN_NR + 1);
	if (ret)
		goto close_adopt;

	ret = hed_rpc_open_accept(&srv->accept, &srv->repo, &srv->adopt
				  &srv->poll, conf);
	if (ret)
		goto close_poll;

	ret = hed_srv_open_sigchan(srv);
	if (!ret)
		return 0;
	
	hed_srv_close_sigchan(srv);
close_poll:
	upoll_close(&srv->upoll);
close_adopt:
	galv_unix_adopt_close(&srv->adopt);
fini:
	galv_repo_fini(&srv->repo);
	return ret;
}

int __hed_nonull(1)
hed_srv_run(struct hed_server *srv)
{
	hed_assert(srv);

	int ret;
	int tmout;

	do {
		tmout = etux_timer_issue_msec();

		ret = upoll_wait(&health_s->poll, tmout);
		if ((ret == -ETIME) || !tmout) {
			/* Expire timers. */
			etux_timer_run();
			ret = 0;
		}

		if (ret > 0)
			ret = upoll_dispatch(&srv->poll, (unsigned int)ret);
		
	} while (!ret || (ret == -EINTR));
	switch (ret) {
	case -ESHUTDOWN:
	case -EINTR:
		ret = 0;
		break;
	}

	return ret;
}

int __hed_nonull(1)
hed_srv_halt(struct hed_server *srv)
{
	hed_assert(srv);

	int ret;

	galv_accept_suspend((struct galv_accept *)&srv->accept, &srv->poller);
	galv_conn_repo_halt(&srv->repo, &srv->poll);
	err = 0;
	while (!galv_repo_empty(&srv->repo)) {
		/*
		 * To be safe, a timer should be armed here to prevent from
		 * blocking into epoll_wait() forever...
		 */
		ret = upoll_process(srv->poll, -1);
		if (ret)
			break;
	}
	if (ret == -ESHUTDOWN)
		ret = -EINTR;

	galv_conn_repo_close(&srv->repo, &srv->poll);
	return ret;
}


void __hed_nonull(1)
hed_srv_fini(struct hed_server *srv)
{
	hed_assert(srv);

	hed_srv_close_sigchan(srv);
	galv_sess_close_accept(&srv->accept, &srv->poll);
	upoll_close(&srv->upoll);
	galv_unix_adopt_close(&srv->adopt);
	galv_repo_fini(&srv->repo);
}

