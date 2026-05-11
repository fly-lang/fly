/*===-- std/os/fly_os_path.h - fly.os path manipulation API -------------===
 *
 * Pure path string manipulation — zero syscalls except fly_path_absolute,
 * fly_path_isFile, fly_path_isDir, fly_path_isSym, and fly_path_glob.
 *===----------------------------------------------------------------------===*/

#ifndef FLY_OS_PATH_H
#define FLY_OS_PATH_H

#include "fly_os_io.h"

void fly_path_join          (const fly_string *base, const fly_string *comp, fly_string *out);
void fly_path_joinN         (const fly_string *parts, size_t n, fly_string *out);
void fly_path_absolute      (const fly_string *path, fly_string *out);
void fly_path_basename      (const fly_string *path, fly_string *out);
void fly_path_dirname       (const fly_string *path, fly_string *out);
void fly_path_ext           (const fly_string *path, fly_string *out);
void fly_path_stem          (const fly_string *path, fly_string *out);
void fly_path_split         (const fly_string *path, fly_string *out_dir, fly_string *out_base);
void fly_path_splitExt      (const fly_string *path, fly_string *out_stem, fly_string *out_ext);
void fly_path_isAbsolute    (const fly_string *path, int *out);
void fly_path_isRelative    (const fly_string *path, int *out);
void fly_path_normalize     (const fly_string *path, fly_string *out);
void fly_path_normalizeInPlace(fly_string *path);
void fly_path_rel           (const fly_string *base, const fly_string *target, fly_string *out);
void fly_path_isFile        (const fly_string *path, int *out);
void fly_path_isDir         (const fly_string *path, int *out);
void fly_path_isSym         (const fly_string *path, int *out);
void fly_path_glob          (const fly_string *pattern, fly_string_array *out);
void fly_path_match         (const fly_string *pattern, const fly_string *name, int *out);
void fly_path_comp          (const fly_string *path, fly_string_array *out);
void fly_path_sep           (uint8_t *out);

#endif /* FLY_OS_PATH_H */
