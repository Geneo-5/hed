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
	galv_rpc_fn * meth;
};

struct hed_rpc_factory {
	struct galv_rpc_factory base;
	uint32_t max_id;
	size_t auth_nr;
	struct hed_rpc_auth * auth;
};

extern ssize_t
hed_rpc_create(const struct galv_rpc_factory * __restrict factory,
               const struct galv_rpc_conn *    __restrict rpc __unused,
               galv_rpc_fn * const ** meth)
	__hed_nonull(1, 2, 3);

extern void
hed_rpc_destroy(const struct galv_rpc_factory * __restrict factory __unused,
                const struct galv_rpc_conn * __restrict rpc __unused,
                galv_rpc_fn ** meth)
	__hed_nonull(1, 2);

#define HED_RPC_FACTORY(_max, _nr, _auth) { \
	.base.create = hed_rpc_create, \
	.base.destroy = hed_rpc_destroy, \
	.max_id = _max, \
	.auth_nr = _nr, \
	.auth = _auth \
}

struct hed_rpc_connect_conf {
};

#define HED_RPC_CONNECT_CONF(...) {}

#endif /* _HED_RPC_H */
