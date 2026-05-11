/*===-- std/os/fly_os_fs.h - fly.os filesystem API ----------------------===*/

#ifndef FLY_OS_FS_H
#define FLY_OS_FS_H

#include "fly_os_io.h"

/* ── Open / close ────────────────────────────────────────────────────────── */

void _F6fly_os6fsOpen_Ss_Cfly_file    (void *err_ctx, const fly_string *path, fly_file *out);
void _F6fly_os8fsCreate_Ss_Cfly_file  (void *err_ctx, const fly_string *path, fly_file *out);
void _F6fly_os9fsOpenOpts_Ss_y_ui_Cfly_file (void *err_ctx, const fly_string *path, uint8_t flags, uint32_t perm, fly_file *out);
void _F6fly_os7fsClose_Cfly_file     (void *err_ctx, fly_file *f);
void _F6fly_os8fsReader_Cfly_file_Cfly_reader (void *err_ctx, fly_file *f, fly_reader *out);
void _F6fly_os8fsWriter_Cfly_file_Cfly_writer (void *err_ctx, fly_file *f, fly_writer *out);

/* ── Convenience read/write ──────────────────────────────────────────────── */

void _F6fly_os6fsRead_Ss_Cfly_buf     (void *err_ctx, const fly_string *path, fly_buf *out);
void _F6fly_os7fsWrite_Ss_Cfly_buf_ui  (void *err_ctx, const fly_string *path, const fly_buf *data, uint32_t perm);
void _F6fly_os8fsAppend_Ss_Cfly_buf_ui (void *err_ctx, const fly_string *path, const fly_buf *data, uint32_t perm);

/* ── Seek ────────────────────────────────────────────────────────────────── */

void _F6fly_os9fsSeekTo_Cfly_file_l_i_l (void *err_ctx, fly_file *f, int64_t offset, int whence, int64_t *out_pos);
void _F6fly_os9fsSeekPos_Cfly_file_l    (void *err_ctx, fly_file *f, int64_t *out_pos);

/* ── Metadata ────────────────────────────────────────────────────────────── */

void _F6fly_os6fsStat_Ss_Cfly_stat    (void *err_ctx, const fly_string *path, fly_stat *out);
void _F6fly_os7fsLstat_Ss_Cfly_stat   (void *err_ctx, const fly_string *path, fly_stat *out);
void _F6fly_os6fsSize_Ss_ul            (void *err_ctx, const fly_string *path, uint64_t *out);
void _F6fly_os8fsExists_Ss_b          (void *err_ctx, const fly_string *path, int *out);
void _F6fly_os6fsSync_Cfly_file      (void *err_ctx, fly_file *f);
void _F6fly_os10fsTruncate_Ss_ul       (void *err_ctx, const fly_string *path, uint64_t size);
void _F6fly_os7fsChmod_Ss_ui           (void *err_ctx, const fly_string *path, uint32_t mode);

/* ── File operations ─────────────────────────────────────────────────────── */

void _F6fly_os8fsDelete_Ss            (void *err_ctx, const fly_string *path);
void _F6fly_os6fsCopy_Ss_Ss            (void *err_ctx, const fly_string *src, const fly_string *dst);
void _F6fly_os6fsMove_Ss_Ss            (void *err_ctx, const fly_string *src, const fly_string *dst);
void _F6fly_os8fsRename_Ss_Ss          (void *err_ctx, const fly_string *src, const fly_string *dst);

/* ── Directory operations ────────────────────────────────────────────────── */

void _F6fly_os11fsDirCreate_Ss_ui      (void *err_ctx, const fly_string *path, uint32_t perm);
void _F6fly_os14fsDirCreateAll_Ss_ui   (void *err_ctx, const fly_string *path, uint32_t perm);
void _F6fly_os11fsDirDelete_Ss        (void *err_ctx, const fly_string *path);
void _F6fly_os14fsDirDeleteAll_Ss     (void *err_ctx, const fly_string *path);
void _F6fly_os9fsDirRead_Ss_Cfly_dir_entries (void *err_ctx, const fly_string *path, fly_dir_entries *out);
void _F6fly_os9fsDirWalk_Ss_Cfly_walk_fn     (void *err_ctx, const fly_string *path,
                                              void (*cb)(const fly_string *path,
                                                         const fly_stat   *stat,
                                                         void             *userdata),
                                              void *userdata);

/* ── Symlinks ────────────────────────────────────────────────────────────── */

void _F6fly_os15fsSymlinkCreate_Ss_Ss  (void *err_ctx, const fly_string *target, const fly_string *link);
void _F6fly_os13fsSymlinkRead_Ss_Ss    (void *err_ctx, const fly_string *path, fly_string *out);

/* ── Temporary ───────────────────────────────────────────────────────────── */

void _F6fly_os10fsTempFile_Ss_Ss_Ss_Cfly_file (void *err_ctx, const fly_string *dir, const fly_string *pattern,
                                             fly_string *out_path, fly_file *out_file);
void _F6fly_os9fsTempDir_Ss_Ss_Ss       (void *err_ctx, const fly_string *dir, const fly_string *pattern,
                                       fly_string *out);

#endif /* FLY_OS_FS_H */
