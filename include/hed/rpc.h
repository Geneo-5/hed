/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#ifndef _HED_RPC_H
#define _HED_RPC_H

#include <hed/cdefs.h>
#include <galv/repo.h>
#include <galv/session.h>

struct hed_rpc_msg {
	struct galv_sess_msg super; /* first */
	uint32_t id;
	void * ctx;
};

typedef int hed_rpc_fn(struct hed_rpc_msg * msg);

struct hed_rpc_conn {
	struct galv_sess_conn super; /* first */
	const size_t rpc_nb;
	hed_rpc_fn * const * const rpc;
};
#define galv_to_rpc_conn(_conn) containerof(_conn, struct hed_rpc_conn, super)

static inline struct hed_rpc_msg * __hed_nonull(1)
hed_rpc_create_request(struct hed_rpc_conn * conn, void * ctx)
{
	hed_assert_api(conn);

	struct hed_rpc_msg *msg;

	msg =(struct hed_rpc_msg *)galv_sess_create_request(&conn->super);
	if (!msg)
		return NULL;

	msg->ctx = ctx;
	return msg;
}

static inline struct hed_rpc_msg * __hed_nonull(1)
hed_rpc_create_notif(struct hed_rpc_conn * conn)
{
	hed_assert_api(conn);

	return (struct hed_rpc_msg *)galv_sess_create_notif(&conn->super);
}

static inline int __hed_nonull(1)
hed_rpc_make_reply(struct hed_rpc_msg * msg)
{
	hed_assert_api(msg);

	return galv_sess_make_reply(&msg->super);
}

static inline int __hed_nonull(1)
hed_rpc_push_msg(struct hed_rpc_msg *msg)
{
	hed_assert_api(msg);

	return galv_sess_push_msg(&msg->super);
}

static inline void __hed_nonull(1)
hed_rpc_drop_msg(struct hed_rpc_msg *msg)
{
	hed_assert_api(msg);

	galv_sess_drop_msg(&msg->super);
}

static inline bool __hed_nonull(1)
hed_rpc_may_pull_msg(const struct hed_rpc_conn * conn)
{
	hed_assert_api(conn);

	return galv_sess_may_pull_msg(&conn->super);
}

static inline struct hed_rpc_msg * __hed_nonull(1)
hed_rpc_pull_msg(struct hed_rpc_conn * conn)
{
	hed_assert_api(conn);

	return (struct hed_rpc_msg *)galv_sess_pull_msg(&conn->super);
}

static inline size_t __hed_nonull(1)
hed_rpc_msg_size(const struct hed_rpc_msg * msg)
{
	hed_assert_api(msg);

	return galv_sess_msg_size(&msg->super);
}

static inline  enum galv_sess_head_type __hed_nonull(1)
hed_rpc_msg_type(const struct hed_rpc_msg * msg)
{
	hed_assert_api(msg);

	return galv_sess_msg_type(&msg->super);
}


struct hed_rpc_auth {
	uint32_t id;
	gid_t    gid;
	hed_rpc_fn * rpc;
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
hed_rpc_conn_acceptor(const struct hed_rpc_conn * __restrict session)
{
	hed_assert_api(session);

	return (struct hed_rpc_accept *)galv_sess_conn_acceptor(&session->super);
}

#define HED_RPC_ACCEPT_CONF(_backlog, _conn_flags, _max_pload, _buff_capa, \
                            _id_max, _rpc_nb, _rpc) \
	{ \
		.super = GALV_SESS_ACCEPT_CONF(_backlog, _conn_flags, \
		                               _max_pload, \
		                               sizeof(struct hed_rpc_msg), \
		                               _buff_capa, \
		                               sizeof(struct hed_rpc_conn)), \
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

#endif /* _HED_RPC_H */
