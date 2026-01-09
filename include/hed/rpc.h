/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#ifndef _HED_RPC_H
#define _HED_RPC_H

#include <hed/cdefs.h>
#include <dpack/codec.h>
#include <galv/repo.h>
#include <galv/rpc.h>

struct hed_rpc_auth {
	uint32_t      id;
	gid_t         gid;
	galv_rpc_fn * rpc;
};

struct hed_rpc_accept {
	struct galv_sess_accept super; /* first */
	const uint32_t id_max;
	const unsigned int rpc_nb;
	const struct hed_rpc_auth * const rpc;
};

struct hed_rpc_accept_conf {
	struct galv_sess_accept_conf super;
	uint32_t id_max;
	unsigned int rpc_nb;
	struct hed_rpc_auth * const rpc;
};

static inline struct hed_rpc_accept *
hed_rpc_conn_acceptor(const struct galv_rpc_conn * __restrict session)
{
	hed_assert_api(session);

	return (struct hed_rpc_accept *)galv_sess_conn_acceptor(&session->base);
}

#define HED_RPC_ACCEPT_CONF(_backlog, _conn_flags, _max_pload, _buff_capa, \
                            _id_max, _rpc_nb, _rpc) \
	{ \
		.super = GALV_SESS_ACCEPT_CONF(_backlog, _conn_flags, \
		                               _max_pload, \
		                               sizeof(struct galv_rpc_msg), \
		                               _buff_capa, \
		                               sizeof(struct galv_rpc_conn)), \
		.id_max = _id_max, \
		.rpc_nb = _rpc_nb, \
		.rpc = _rpc, \
	}

extern int
hed_rpc_open_accept(struct hed_rpc_accept            * acceptor,
                    struct galv_repo                 * repository,
                    struct galv_adopt                * adopter,
                    const struct upoll               * poller,
                    const struct hed_rpc_accept_conf * conf)
	__hed_nonull(1, 2, 3, 4, 5);

static inline void
hed_rpc_close_accept(struct hed_rpc_accept * acceptor,
                     const struct upoll    * poller)
{
	hed_assert_api(acceptor);
	hed_assert_api(poller);

	galv_sess_close_accept(&acceptor->super, poller);
}

struct hed_rpc_connect_conf {
};

#define HED_RPC_CONNECT_CONF(...) {}

#endif /* _HED_RPC_H */
