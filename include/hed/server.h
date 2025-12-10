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

struct hed_server {
	struct hed_rpc_accept             accept;
	struct galv_unix_adopt            adopt;
	struct upoll                      poll;
	struct galv_repo                  repo;
	struct upoll_worker               sig_worker;
	int                               sig_fd;
};

extern int
hed_srv_init(struct hed_server                *srv,
             char                             *path,
             const struct hed_rpc_accept_conf *conf)
	__hed_nonull(1, 2, 3);

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
	hed_assert(srv);

	return &srv->poll;
}

#endif /* _HED_SRV_H */
