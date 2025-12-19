/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#include "hed/rpc.h"
#include "hed/codec.h"
#include <dpack/scalar.h>

static int __hed_nonull(1)
hed_rpc_xfer(struct galv_sess_conn * ctx)
{
	hed_assert_intern(ctx);

	struct hed_rpc_conn  *conn = (struct hed_rpc_conn *)ctx;
	struct hed_rpc_msg   *msg;
	struct dpack_decoder  decoder;
	uint32_t              id;
	int                   ret = 0;

	while(hed_rpc_may_pull_msg(conn)) {
		msg = hed_rpc_pull_msg(conn);
		hed_assert_intern(msg);

		hed_decoder_init(&decoder, msg);
		ret = dpack_decode_uint32(&decoder, &id);
		hed_decoder_fini(&decoder);
		if (ret) {
			hed_rpc_drop_msg(msg);
			break;
		}

		if (hed_rpc_msg_type(msg) == GALV_SESS_HEAD_REPLY_TYPE) {
			if (msg->id != id) {
				ret = -EINVAL;
				hed_rpc_drop_msg(msg);
				break;
			}

		} else {
			msg->id = id;
		}

		if ((msg->id >= conn->rpc_nb) || !conn->rpc[msg->id]) {
			ret = -EPERM;
			hed_rpc_drop_msg(msg);
			break;
		}

		ret = conn->rpc[msg->id](msg);
		if (ret)
			break;
	}
	return ret;
}

static int __hed_nonull(1)
hed_rpc_connect(struct galv_sess_conn * ctx)
{
	hed_assert_intern(ctx);

	struct hed_rpc_conn   *conn = galv_to_rpc_conn(ctx);
	struct hed_rpc_accept *acceptor = hed_rpc_conn_acceptor(conn);

STROLL_IGNORE_WARN("-Wcast-qual")
	*(size_t *)&conn->rpc_nb = acceptor->id_max + 1;
	*(hed_rpc_fn ***)&conn->rpc = calloc(conn->rpc_nb, sizeof(hed_rpc_fn *));
STROLL_RESTORE_WARN
	if (!conn->rpc)
		return -ENOMEM;

	for (unsigned int i = 0; i < acceptor->rpc_nb; i++) {
		const struct hed_rpc_auth *auth = &acceptor->rpc[i];

		// TODO: check user group
STROLL_IGNORE_WARN("-Wcast-qual")
		*(hed_rpc_fn **)&conn->rpc[auth->id] = auth->rpc;
STROLL_RESTORE_WARN
	}

	galv_sess_establish(ctx);	
	return 0;
}

static void __hed_nonull(1)
hed_rpc_close(struct galv_sess_conn * ctx)
{
	hed_assert_intern(ctx);

	struct hed_rpc_conn *conn = galv_to_rpc_conn(ctx);

STROLL_IGNORE_WARN("-Wcast-qual")
	free((hed_rpc_fn **)conn->rpc);
	*(hed_rpc_fn ***)&conn->rpc = NULL;
STROLL_RESTORE_WARN
}

static const struct galv_sess_ops hed_rpc_ops = {
	.connect = hed_rpc_connect,
	.xfer    = hed_rpc_xfer,
	.close   = hed_rpc_close,
};

extern int __hed_nonull(1, 2, 3, 4, 5)
hed_rpc_open_accept(struct hed_rpc_accept            * acceptor,
                    struct galv_repo                 * repository,
                    struct galv_adopt                * adopter,
                    const struct upoll               * poller,
                    const struct hed_rpc_accept_conf * conf)
{
	hed_assert_api(acceptor);
	hed_assert_api(repository);
	hed_assert_api(adopter);
	hed_assert_api(poller);
	hed_assert_api(conf);

	int ret;

	ret = galv_sess_open_accept(&acceptor->super,
	                            &hed_rpc_ops,
	                            repository,
	                            adopter,
	                            poller,
	                            &conf->super);
	if (ret)
		return ret;

STROLL_IGNORE_WARN("-Wcast-qual")
	*(uint32_t *)&acceptor->id_max = conf->id_max;
	*(unsigned int *)&acceptor->rpc_nb = conf->rpc_nb;
	*(struct hed_rpc_auth ** const )&acceptor->rpc = conf->rpc;
STROLL_RESTORE_WARN
	return 0;
}
