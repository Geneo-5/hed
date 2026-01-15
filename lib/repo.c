/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#include "hed/repo.h"

static void __hed_nonull(1)
repo_close(struct hed_repo * repo)
{
	hed_assert_intern(repo);

	if (!repo->env)
		return;

	if (repo->txn)
		hed_repo_abort(repo);

	mdb_env_close(repo->env);
	repo->env = NULL;

}

static int __hed_nonull(1, 2)
repo_open_table(struct hed_repo * repo, const char * table, int flags)
{
	hed_assert_intern(repo);
	hed_assert_intern(repo->env);
	hed_assert_intern(repo->txn);
	hed_assert_intern(table);

	int ret;
	MDB_dbi dbi;
	unsigned int f = flags & O_CREAT ? MDB_CREATE : 0;

	ret = mdb_dbi_open(repo->txn, table, f, &dbi);
	if (ret)
		return ret;

	if (!(flags & O_TRUNC))
		return 0;

	return mdb_drop(repo->txn, dbi, 0);
}

static int __hed_nonull(1)
repo_open(struct hed_repo * repo, int flags)
{
	hed_assert_intern(repo);
	hed_assert_intern(!repo->env);
	hed_assert_intern(!repo->txn);

	int ret;
	unsigned int f = flags & O_RDONLY ? MDB_RDONLY : 0;
	size_t i;
	uint32_t seq = 0;

	ret = mdb_env_create(&repo->env);
	if (ret)
		return ret;

	ret = mdb_env_set_maxdbs(repo->env, (MDB_dbi)(repo->nb + 1));
	if (ret)
		goto error;

	ret = mdb_env_open(repo->env, stroll_lvstr_cstr(&repo->path),
			   f | MDB_NOSUBDIR, repo->mode);
	if (ret)
		goto error;

	ret = hed_repo_start(repo);
	if (ret)
		goto error;

	ret = repo_open_table(repo, ".hed", flags);
	if (ret)
		goto error;

	for (i = 0; i < repo->nb; i++) {
		hed_assert_api(repo->table[i]);
		hed_assert_api(repo->table[i][0] != '.');

		ret = repo_open_table(repo, repo->table[i], flags);
		if (ret)
			goto error;
	}

	if (flags & O_TRUNC) {
		for (i = 0; i < repo->nb; i++) {
			ret = hed_repo_update(repo, ".hed",
				(const uint8_t *)repo->table[i],
				strlen(repo->table[i]),
				(const uint8_t *)&seq, sizeof(seq));
			if (ret)
				goto error;
		}
	}

	return hed_repo_commit(repo);
error:
	repo_close(repo);
	return ret;
}

int __hed_nonull(1, 2, 3) __warn_result
hed_repo_open(struct hed_repo    *repo,
              const char         *path,
              const char * const *table,
              size_t              nb,
              int                 flags,
              mode_t              mode)
{
	hed_assert_api(repo);
	hed_assert_api(path);
	hed_assert_api(strlen(path) > 0);
	hed_assert_api(!(flags & (~(O_RDWR | O_RDONLY | O_CREAT | O_TRUNC))));
	hed_assert_api(!(flags & O_CREAT) | (flags & O_RDWR));
	hed_assert_api(!(flags & O_TRUNC) | (flags & O_CREAT));


	int ret;
	char *backup;

	repo->env = NULL;
	repo->txn = NULL;

STROLL_IGNORE_WARN("-Wcast-qual")
	*(int *)&repo->flags = flags & O_ACCMODE;
	*(mode_t *)&repo->mode = mode;
	*(char  ***)&repo->table = (char **)table;
	*(size_t *)&repo->nb = nb;
STROLL_RESTORE_WARN

	backup = malloc((strlen(path) + 8) * sizeof(char));
	if (!backup)
		return -ENOMEM;

	snprintf(backup, (strlen(path) + 8), "%s-backup", path);
	ret = stroll_lvstr_init_cede(&repo->backup, backup);
	if (ret)
		goto free_backup;

	ret = stroll_lvstr_init_dup(&repo->path, path);
	if (ret)
		goto free_backup;
	
	ret = repo_open(repo, flags);
	if (ret)
		goto free_path;

	return 0;

free_path:
	stroll_lvstr_fini(&repo->path);
free_backup:
	stroll_lvstr_fini(&repo->backup);
	return ret;
}

void __hed_nonull(1)
hed_repo_close(struct hed_repo * repo)
{
	hed_assert_api(repo);

	if (!repo->env)
		return;

	repo_close(repo);
	if (repo->flags & O_RDWR)
		remove(stroll_lvstr_cstr(&repo->backup));
	stroll_lvstr_fini(&repo->path);
	stroll_lvstr_fini(&repo->backup);
}

int __hed_nonull(1) __warn_result
hed_repo_reload(struct hed_repo * repo)
{
	hed_assert_api(repo);
	hed_assert_api(repo->env);
	hed_assert_api(!repo->txn);

	repo_close(repo);
	return repo_open(repo, repo->flags);
}

int __hed_nonull(1) __warn_result
hed_repo_start(struct hed_repo * repo)
{
	hed_assert_api(repo);
	hed_assert_api(repo->env);
	hed_assert_api(!repo->txn);

	unsigned int flags = repo->flags & O_RDONLY ? MDB_RDONLY : 0;

#if defined(CONFIG_HED_REPO_3PC)
	int fd;
	int ret;

	if (!flags) {
		fd = ufile_new(stroll_lvstr_cstr(&repo->backup),
			O_RDWR | O_TRUNC | O_CLOEXEC, repo->mode);
		if (fd < 0)
			return fd;

		ret = mdb_env_copyfd(repo->env, fd);
		ufile_close(fd);
		if (ret)
			return ret;
	}
#endif

	return mdb_txn_begin(repo->env, NULL, flags, &repo->txn);
}

int __hed_nonull(1) __warn_result
hed_repo_commit(struct hed_repo * repo)
{
	hed_assert_api(repo);
	hed_assert_api(repo->env);
	hed_assert_api(repo->txn);

	int ret;

	ret = mdb_txn_commit(repo->txn);
	repo->txn = NULL;
	return ret;
}

void __hed_nonull(1)
hed_repo_abort(struct hed_repo * repo)
{
	hed_assert_api(repo);
	hed_assert_api(repo->env);
	hed_assert_api(repo->txn);

	mdb_txn_abort(repo->txn);
	repo->txn = NULL;
}

#if defined(CONFIG_HED_REPO_3PC)
int __hed_nonull(1) __warn_result
hed_repo_rollback(struct hed_repo * repo)
{
	hed_assert_api(repo);
	hed_assert_api(repo->env);
	hed_assert_api(!repo->txn);

	repo_close(repo);
	rename(stroll_lvstr_cstr(&repo->backup), stroll_lvstr_cstr(&repo->path));
	return repo_open(repo, repo->flags);
}
#endif

int __hed_nonull(1, 2, 3, 5, 6) __warn_result
hed_repo_get(struct hed_repo * repo,
             const char * table,
             const uint8_t * key,
             size_t klen,
             uint8_t * * const value,
             size_t * vlen)
{
	hed_assert_api(repo);
	hed_assert_api(repo->env);
	hed_assert_api(repo->txn);
	hed_assert_api(table);
	hed_assert_api(key);
	hed_assert_api(klen > 0);
	hed_assert_api(value);
	hed_assert_api(vlen);

	int ret;
	MDB_dbi dbi;
	MDB_val content;
STROLL_IGNORE_WARN("-Wcast-qual")
	MDB_val idx = {
		.mv_data = (uint8_t *)key,
		.mv_size = klen
	};
STROLL_RESTORE_WARN

	ret = mdb_dbi_open(repo->txn, table, 0, &dbi);
	if (ret)
		return ret;

	ret = mdb_get(repo->txn, dbi, &idx, &content);
	if (ret)
		return ret;

	*value = content.mv_data;
	*vlen = content.mv_size;
	return 0;
}

int __hed_nonull(1, 2, 3, 5) __warn_result
hed_repo_update(struct hed_repo * repo,
                const char * table,
                const uint8_t * key,
                size_t klen,
                const uint8_t * value,
                size_t vlen)
{
	hed_assert_api(repo);
	hed_assert_api(repo->env);
	hed_assert_api(repo->txn);
	hed_assert_api(table);
	hed_assert_api(key);
	hed_assert_api(klen > 0);
	hed_assert_api(value);
	hed_assert_api(vlen > 0);

	int ret;
	MDB_dbi dbi;
STROLL_IGNORE_WARN("-Wcast-qual")
	MDB_val idx = {
		.mv_data = (uint8_t *)key,
		.mv_size = klen
	};
	MDB_val content = {
		.mv_data = (uint8_t *)value,
		.mv_size = vlen
	};
STROLL_RESTORE_WARN

	ret = mdb_dbi_open(repo->txn, table, 0, &dbi);
	if (ret)
		return ret;

	return mdb_put(repo->txn, dbi, &idx, &content, 0);
}


int __hed_nonull(1, 2, 3) __warn_result
hed_repo_del(struct hed_repo * repo,
                const char * table,
                const uint8_t * key,
                size_t klen)

{
	hed_assert_api(repo);
	hed_assert_api(repo->env);
	hed_assert_api(repo->txn);
	hed_assert_api(table);
	hed_assert_api(key);
	hed_assert_api(klen > 0);

	int ret;
	MDB_dbi dbi;
STROLL_IGNORE_WARN("-Wcast-qual")
	MDB_val idx = {
		.mv_data = (uint8_t *)key,
		.mv_size = klen
	};
STROLL_RESTORE_WARN

	ret = mdb_dbi_open(repo->txn, table, 0, &dbi);
	if (ret)
		return ret;

	return mdb_del(repo->txn, dbi, &idx, NULL);
}

ssize_t __hed_nonull(1, 2) __warn_result
hed_repo_count(struct hed_repo * repo,
               const char * table)
{
	hed_assert_api(repo);
	hed_assert_api(repo->env);
	hed_assert_api(repo->txn);
	hed_assert_api(table);

	MDB_dbi dbi;
	MDB_stat stat;
	ssize_t ret;

	ret = mdb_dbi_open(repo->txn, table, 0, &dbi);
	if (ret)
		return -ret;

	ret = mdb_stat(repo->txn, dbi, &stat);
	if (ret)
		return -ret;

	return (ssize_t)stat.ms_entries;
}

uint32_t __hed_nonull(1, 2) __warn_result
hed_repo_next_seq(struct hed_repo * repo,
                  const char * table)
{
	hed_assert_api(repo);
	hed_assert_api(repo->env);
	hed_assert_api(repo->txn);
	hed_assert_api(table);

	int ret;
	size_t len;
	uint32_t *old;
	uint32_t seq;

	ret = hed_repo_get(repo, ".hed", (const uint8_t *)table, strlen(table),
	                   (uint8_t **)&old, &len);
	if (ret)
		return 0;

	hed_assert_api(len == sizeof(seq));
	seq = *old + 1;

	ret = hed_repo_update(repo , ".hed",
	                      (const uint8_t *)table, strlen(table),
	                      (uint8_t *)&seq, sizeof(seq));
	if (ret)
		return 0;

	return seq;
}

struct hed_repo_iter * __hed_nonull(1, 2) __warn_result
hed_repo_create_iter(struct hed_repo * repo,
                     const char * table)
{
	hed_assert_api(repo);
	hed_assert_api(repo->env);
	hed_assert_api(repo->txn);
	hed_assert_api(table);

	struct hed_repo_iter *iter;
	MDB_dbi dbi;

	if (mdb_dbi_open(repo->txn, table, 0, &dbi))
		return NULL;

	iter = malloc(sizeof(*iter));
	if (!iter)
		return NULL;

	iter->repo = repo;
	if (mdb_cursor_open(repo->txn, dbi, &iter->cursor))
		goto error;

	if (mdb_cursor_get(iter->cursor, NULL, NULL, MDB_FIRST))
		goto error;

	return iter;
error:
	free(iter);
	return NULL;
}

void __hed_nonull(1)
hed_repo_destroy_iter(struct hed_repo_iter *iter)
{
	hed_assert_api(iter);
	hed_assert_api(iter->cursor);

	mdb_cursor_close(iter->cursor);
	free(iter);
}

int __hed_nonull(1) __warn_result
hed_repo_step(struct hed_repo_iter * iter,
              uint8_t * * const key,
              size_t  * klen,
              uint8_t * * const value,
              size_t * vlen)
{
	hed_assert_api(iter);
	hed_assert_api(iter->cursor);
	hed_assert_api(!!key | !!value);
	hed_assert_api(!(!!key ^ !!klen));
	hed_assert_api(!(!!value ^ !!vlen));

	MDB_val idx;
	MDB_val content;
	int ret;

	ret = mdb_cursor_get(iter->cursor, &idx, &content, MDB_GET_CURRENT);
	if (ret)
		return -EINVAL;

	if (key) {
		*key  = idx.mv_data;
		*klen = idx.mv_size;
	}

	if (value) {
		*value = content.mv_data;
		*vlen  = content.mv_size;
	}

	ret = mdb_cursor_get(iter->cursor, NULL, NULL, MDB_NEXT);
	if (ret == MDB_NOTFOUND)
		return 0;

	if (ret)
		return -EINVAL;

	return EAGAIN;
}
