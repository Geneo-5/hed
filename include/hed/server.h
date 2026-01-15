/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#ifndef _HED_SRV_H
#define _HED_SRV_H

#include <hed/cdefs.h>
#include <hed/rpc.h>
#include <galv/session.h>
#include <galv/unix.h>

struct hed_server {
	struct galv_rpc_accept            accept;
	struct galv_unix_adopt            adopt;
	struct upoll                      poll;
	struct galv_repo                  repo;
	struct upoll_worker               sig_worker;
	int                               sig_fd;
};

extern int
hed_srv_init(struct hed_server                 *srv,
             char                              *path,
             const struct galv_rpc_accept_conf *conf,
	     const struct hed_rpc_factory      *factory)
	__hed_nonull(1, 2, 3, 4);

extern int
hed_srv_process(struct hed_server *srv)
	__hed_nonull(1);

extern int
hed_srv_run(struct hed_server *srv)
	__hed_nonull(1);

extern int
hed_srv_halt(struct hed_server *srv)
	__hed_nonull(1);

extern void
hed_srv_fini(struct hed_server *srv)
	__hed_nonull(1);

static inline struct upoll * __hed_nonull(1)
hed_srv_get_upoll(struct hed_server *srv)
{
	hed_assert_api(srv);

	return &srv->poll;
}

#endif /* _HED_SRV_H */
