/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#ifndef _HED_REPO_H
#define _HED_REPO_H

#include <hed/cdefs.h>
#include <errno.h>
#include <fcntl.h>
#include <lmdb.h>
#include <stdio.h>
#include <stroll/lvstr.h>
#include <utils/file.h>

struct hed_repo {
	MDB_env             *env;
	MDB_txn             *txn;
	struct stroll_lvstr  path;
	struct stroll_lvstr  backup;
	const int            flags;
	const mode_t         mode;
	const char  * const *table;
	const size_t         nb;
};

struct hed_repo_iter {
	MDB_cursor *cursor;
	struct hed_repo *repo;
};

extern int
hed_repo_open(struct hed_repo    *repo,
              const char         *path,
              const char * const *table,
              size_t              nb,
              int                 flags,
              mode_t              mode)
	__hed_nonull(1, 2) __warn_result;

extern void
hed_repo_close(struct hed_repo * repo)
	__hed_nonull(1);

extern int
hed_repo_reload(struct hed_repo * repo)
	__hed_nonull(1) __warn_result;

extern int
hed_repo_start(struct hed_repo * repo)
	__hed_nonull(1) __warn_result;

extern int
hed_repo_commit(struct hed_repo * repo)
	__hed_nonull(1) __warn_result;

#if defined(CONFIG_HED_REPO_3PC)
extern int
hed_repo_rollback(struct hed_repo * repo)
	__hed_nonull(1) __warn_result;
#endif

extern void
hed_repo_abort(struct hed_repo * repo)
	__hed_nonull(1);

extern int
hed_repo_get(struct hed_repo * repo,
             const char * table,
             const uint8_t * key,
             size_t klen,
             uint8_t * * const value,
             size_t * vlen)
	__hed_nonull(1, 2, 3, 5, 6) __warn_result;

extern int
hed_repo_update(struct hed_repo * repo,
                const char * table,
                const uint8_t * key,
                size_t klen,
                const uint8_t * value,
                size_t vlen)
	__hed_nonull(1, 2, 3, 5) __warn_result;

extern int
hed_repo_del(struct hed_repo * repo,
                const char * table,
                const uint8_t * key,
                size_t klen)
	__hed_nonull(1, 2, 3) __warn_result;

extern ssize_t
hed_repo_count(struct hed_repo * repo,
               const char * table)
	__hed_nonull(1, 2) __warn_result;

extern uint32_t
hed_repo_next_seq(struct hed_repo * repo,
                  const char * table)
	__hed_nonull(1, 2) __warn_result;

extern struct hed_repo_iter *
hed_repo_create_iter(struct hed_repo * repo,
                     const char * table)
	__hed_nonull(1, 2) __warn_result;

extern void
hed_repo_destroy_iter(struct hed_repo_iter *iter)
	__hed_nonull(1);

extern int
hed_repo_step(struct hed_repo_iter * iter,
              uint8_t * * const key,
              size_t  * klen,
              uint8_t * * const value,
              size_t * vlen)
	__hed_nonull(1) __warn_result;

static inline int __hed_nonull(1, 2, 3) __warn_result
hed_repo_get_version(struct hed_repo * repo,
                     uint8_t * * const value,
                     size_t * vlen)
{
	hed_assert_api(repo);
	hed_assert_api(value);
	hed_assert_api(vlen);

	return hed_repo_get(repo, ".hed",
	                    (const uint8_t *)".version", 8,
	                    value, vlen);
}

static inline int __hed_nonull(1, 2) __warn_result
hed_repo_set_version(struct hed_repo * repo,
                     const uint8_t * value,
                     size_t vlen)
{
	hed_assert_api(repo);
	hed_assert_api(value);
	hed_assert_api(vlen);

	return hed_repo_update(repo, ".hed",
	                       (const uint8_t *)".version", 8,
	                       value, vlen);
}

#endif /* _HED_REPO_H */
