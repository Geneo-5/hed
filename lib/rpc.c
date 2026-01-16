/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#include "hed/rpc.h"

ssize_t
hed_rpc_create(const struct galv_rpc_factory * __restrict factory,
               const struct galv_rpc_conn *    __restrict rpc __unused,
               galv_rpc_fn * const ** meth)
{
	hed_assert_intern(factory);
	hed_assert_intern(rpc);
	hed_assert_intern(meth);

	const struct hed_rpc_factory * auth_factory;
	galv_rpc_fn * * fn;
	bool permit = false;
	size_t nr;

	auth_factory = (const struct hed_rpc_factory *)factory;
	nr = auth_factory->max_id + 1;
	fn = calloc(nr, sizeof(galv_rpc_fn *));
	if (!fn)
		return -ENOMEM;

	for (size_t i = 0; i < auth_factory->auth_nr; i++) {
		hed_assert_intern(i < nr);

		const struct hed_rpc_auth *auth = &auth_factory->auth[i];

		fn[auth->id] = auth->meth;
		permit = true;
	}

	if (permit) {
		*meth = fn;
		return (ssize_t)nr;
	}

	free(fn);
	return -EPERM;
}

void
hed_rpc_destroy(const struct galv_rpc_factory * __restrict factory __unused,
                const struct galv_rpc_conn * __restrict rpc __unused,
                galv_rpc_fn ** meth)
{
	hed_assert_intern(factory);
	hed_assert_intern(rpc);

	free(meth);
}

