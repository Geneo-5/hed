#ifndef _HED_SYNC_RPC_H
#define _HED_SYNC_RPC_H

#include <galv/rpc_clnt.h>

struct hed_rpc_clnt_msg {
	struct galv_rpc_clnt_msg base;
	void *                   cb;
	void *                   ctx;
};

static inline int
hed_rpc_clnt_connect(struct galv_rpc_clnt *     client,
                     const struct sockaddr_un * peer,
                     socklen_t                  size)
{
	return galv_rpc_clnt_connect(client, peer, size);
}

static inline int
hed_rpc_clnt_open(struct galv_rpc_clnt * client, int flags)
{
	return galv_rpc_clnt_open(client, flags, sizeof(struct hed_rpc_clnt_msg));
}

static inline void
hed_rpc_clnt_close(struct galv_rpc_clnt * client)
{
	galv_rpc_clnt_close(client);
}

#endif /* _HED_SYNC_RPC_H */
