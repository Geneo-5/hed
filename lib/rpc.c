/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#include "hed/rpc.h"

static int __hed_nonull(1)
hed_rpc_xfer(struct galv_sess_conn * ctx)
{
	hed_assert_intern(ctx);

	struct hed_rpc_conn  *conn = (struct hed_rpc_conn *)ctx;
	struct hed_rpc_msg   *msg;
	struct dpack_decoder  decoder;
	uint32_t              id;
	int                   ret = 0;

	while(galv_sess_may_pull_msg(conn)) {
		msg = (struct hed_rpc_msg *)galv_sess_pull_msg(conn);
		hed_assert_intern(msg);

		hed_decoder_init(&decoder, msg);
		ret = dpack_decode_uint32(&decoder, &id);
		hed_decoder_fini(&decoder);
		if (ret) {
			hed_rpc_drop_msg(msg);
			break;
		}

		if (hed_rpc_msg_get_type(msg) == GALV_SESS_HEAD_REPLY_TYPE) {
			if (msg->id != id) {
				ret = -EINVAL;
				hed_rpc_drop_msg(msg);
				break;
			}

		} else {
			msg->id = id;
		}

		if ((msg->id >= conn->rpc_nb) || (conn->rpc[msg->id] == NULL) {
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

static const struct galv_sess_ops hed_rpc_ops = {
	.xfer = hed_rpc_xfer
};

extern int __hed_nonull(1, 2, 3, 4, 5)
hed_rpc_open_accept(struct hed_rpc_accept      * acceptor,
                    struct glav_repo           * repository,
                    struct galv_adopt          * adopter,
                    const struct upoll         * poller,
                    struct hed_rpc_accept_conf * conf)
{
	hed_assert(acceptor);
	hed_assert(repository);
	hed_assert(adopter);
	hed_assert(poller);
	hed_assert(conf);

	int ret;

	ret = galv_sess_open_accept(&acceptor->super,
	                            &hed_rpc_ops,
	                            repository,
	                            adopter,
	                            poller,
	                            &conf->super);
	if (ret)
		return ret;

	acceptor->id_max = conf->id_max;
	acceptor->rpc_nb = conf->rpc_nb;
	acceptor->rpc    = conf->rpc;
	return 0;
}
