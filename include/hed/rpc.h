/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#ifndef _HED_RPC_H
#define _HED_RPC_H

#include <hed/cdefs.h>
#include <hed/codec.h>
#include <galv/session.h>

struct hed_rpc_msg {
	struct galv_sess_msg super; /* first */
	uint32_t id;
	void * ctx;
};

typedef int hed_rpc_fn(struct hed_rpc_msg * msg);

struct hed_rpc_conn {
	struct galv_sess_conn super; /* first */
	const size_t rcp_nb;
	hed_rpc_fn * const rpc;
};


static inline struct hed_rpc_msg * __hed_nonull(1)
hed_rpc_alloc_req(struct hed_rpc_conn * conn, void * ctx)
{
	brg_rpc_assert(conn);

	struct hed_rpc_msg *msg;

	msg = glav_sess_alloc_req(&conn->super);
	if (!msg)
		return NULL;

	msg->ctx = ctx;
	return msg;
}

static inline struct hed_rpc_msg * __hed_nonull(1)
hed_rpc_alloc_ntfy(struct hed_rpc_conn * conn)
{
	brg_rpc_assert(conn);

	return glav_sess_alloc_ntfy(&conn->super);
}


static inline void __hed_nonull(1)
hed_rpc_drop_msg(struct hed_rpc_msg *msg)
{
	glav_sess_drop_msg(&msg->super);
}

struct hed_rpc_auth {
	uint32_t id;
	gid_t    gid;
	hed_rpc_fn * rpc;
};

struct hed_rpc_accept {
	struct glav_sess_accept super; /* first */
	const uint32_t id_max;
	const unsigned int rcp_nb;
	const struct hed_rpc_auth * const rpc;
};

struct hed_rpc_accept_conf {
	struct galv_sess_accept_conf super;
	uint32_t id_max;
	unsigned int rpc_nb;
	const struct hed_sess_rpc_fn * rpc;
};

#define HED_RPC_ACCEPT_CONF(_backlog, _conn_flags, _max_pload, _buff_capa, \
                            _id_max, _rpc_nb, _rpc) \
	{ \
		.super = GALV_SESS_ACCEPT_CONF(_backlog, _conn_flags, \
		                               _max_pload, _buff_capa), \
		.id_max = id_max, \
		.rpc_nb = _rpc_nb, \
		.rpc = _rpc, \
	}

extern int
hed_rpc_open_accept(struct hed_rpc_accept            * acceptor,
                    struct glav_repo                 * repository,
                    struct galv_adopt                * adopter,
                    const struct upoll               * poller,
                    const struct hed_rpc_accept_conf * conf)
	__hed_nonull(1, 2, 3, 4, 5);

#endif /* _HED_RPC_H */
