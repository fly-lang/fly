/*===-- std/os/fly_os_path.h - fly.os path manipulation API -------------===
 *
 * Pure path string manipulation — zero syscalls except fly_path_absolute,
 * fly_path_isFile, fly_path_isDir, fly_path_isSym, and fly_path_glob.
 *===----------------------------------------------------------------------===*/

#ifndef FLY_OS_PATH_H
#define FLY_OS_PATH_H

#include "fly_os_io.h"

void _F6fly_os8pathJoin_Ss_Ss_Ss          (void *err_ctx, const fly_string *base, const fly_string *comp, fly_string *out);
void _F6fly_os9pathJoinN_Ss_ul_Ss         (void *err_ctx, const fly_string *parts, size_t n, fly_string *out);
void _F6fly_os12pathAbsolute_Ss_Ss       (void *err_ctx, const fly_string *path, fly_string *out);
void _F6fly_os12pathBasename_Ss_Ss       (void *err_ctx, const fly_string *path, fly_string *out);
void _F6fly_os11pathDirname_Ss_Ss        (void *err_ctx, const fly_string *path, fly_string *out);
void _F6fly_os7pathExt_Ss_Ss             (void *err_ctx, const fly_string *path, fly_string *out);
void _F6fly_os8pathStem_Ss_Ss            (void *err_ctx, const fly_string *path, fly_string *out);
void _F6fly_os9pathSplit_Ss_Ss_Ss         (void *err_ctx, const fly_string *path, fly_string *out_dir, fly_string *out_base);
void _F6fly_os12pathSplitExt_Ss_Ss_Ss     (void *err_ctx, const fly_string *path, fly_string *out_stem, fly_string *out_ext);
void _F6fly_os13pathIsAbsolute_Ss_b     (void *err_ctx, const fly_string *path, int *out);
void _F6fly_os13pathIsRelative_Ss_b     (void *err_ctx, const fly_string *path, int *out);
void _F6fly_os13pathNormalize_Ss_Ss      (void *err_ctx, const fly_string *path, fly_string *out);
void _F6fly_os20pathNormalizeInPlace_Ss (void *err_ctx, fly_string *path);
void _F6fly_os7pathRel_Ss_Ss_Ss           (void *err_ctx, const fly_string *base, const fly_string *target, fly_string *out);
void _F6fly_os10pathIsFile_Ss_b         (void *err_ctx, const fly_string *path, int *out);
void _F6fly_os9pathIsDir_Ss_b           (void *err_ctx, const fly_string *path, int *out);
void _F6fly_os9pathIsSym_Ss_b           (void *err_ctx, const fly_string *path, int *out);
void _F6fly_os8pathGlob_Ss_Cfly_string_array (void *err_ctx, const fly_string *pattern, fly_string_array *out);
void _F6fly_os9pathMatch_Ss_Ss_b         (void *err_ctx, const fly_string *pattern, const fly_string *name, int *out);
void _F6fly_os8pathComp_Ss_Cfly_string_array (void *err_ctx, const fly_string *path, fly_string_array *out);
void _F6fly_os7pathSep_y               (void *err_ctx, uint8_t *out);

#endif /* FLY_OS_PATH_H */
