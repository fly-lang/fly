/*===-- std/os/fly_os_fs.h - fly.os filesystem API ----------------------===*/

#ifndef FLY_OS_FS_H
#define FLY_OS_FS_H

#include "fly_os_io.h"

/* ── Open / close ────────────────────────────────────────────────────────── */

void fly_fs_open    (const fly_string *path, fly_file *out);
void fly_fs_create  (const fly_string *path, fly_file *out);
void fly_fs_openOpts(const fly_string *path, uint8_t flags, uint32_t perm, fly_file *out);
void fly_fs_close   (fly_file *f);
void fly_fs_reader  (fly_file *f, fly_reader *out);
void fly_fs_writer  (fly_file *f, fly_writer *out);

/* ── Convenience read/write ──────────────────────────────────────────────── */

void fly_fs_read  (const fly_string *path, fly_buf *out);
void fly_fs_write (const fly_string *path, const fly_buf *data, uint32_t perm);
void fly_fs_append(const fly_string *path, const fly_buf *data, uint32_t perm);

/* ── Seek ────────────────────────────────────────────────────────────────── */

void fly_fs_seekTo (fly_file *f, int64_t offset, int whence, int64_t *out_pos);
void fly_fs_seekPos(fly_file *f, int64_t *out_pos);

/* ── Metadata ────────────────────────────────────────────────────────────── */

void fly_fs_stat    (const fly_string *path, fly_stat *out);
void fly_fs_lstat   (const fly_string *path, fly_stat *out);
void fly_fs_size    (const fly_string *path, uint64_t *out);
void fly_fs_exists  (const fly_string *path, int *out);
void fly_fs_sync    (fly_file *f);
void fly_fs_truncate(const fly_string *path, uint64_t size);
void fly_fs_chmod   (const fly_string *path, uint32_t mode);

/* ── File operations ─────────────────────────────────────────────────────── */

void fly_fs_delete(const fly_string *path);
void fly_fs_copy  (const fly_string *src, const fly_string *dst);
void fly_fs_move  (const fly_string *src, const fly_string *dst);
void fly_fs_rename(const fly_string *src, const fly_string *dst);

/* ── Directory operations ────────────────────────────────────────────────── */

void fly_fs_dirCreate   (const fly_string *path, uint32_t perm);
void fly_fs_dirCreateAll(const fly_string *path, uint32_t perm);
void fly_fs_dirDelete   (const fly_string *path);
void fly_fs_dirDeleteAll(const fly_string *path);
void fly_fs_dirRead     (const fly_string *path, fly_dir_entries *out);
void fly_fs_dirWalk     (const fly_string *path,
                         void (*cb)(const fly_string *path,
                                    const fly_stat   *stat,
                                    void             *userdata),
                         void *userdata);

/* ── Symlinks ────────────────────────────────────────────────────────────── */

void fly_fs_symlinkCreate(const fly_string *target, const fly_string *link);
void fly_fs_symlinkRead  (const fly_string *path, fly_string *out);

/* ── Temporary ───────────────────────────────────────────────────────────── */

void fly_fs_tempFile(const fly_string *dir, const fly_string *pattern,
                     fly_string *out_path, fly_file *out_file);
void fly_fs_tempDir (const fly_string *dir, const fly_string *pattern,
                     fly_string *out);

#endif /* FLY_OS_FS_H */
