/*===-- std/os/fly_os_env.h - fly.os environment API --------------------===*/

#ifndef FLY_OS_ENV_H
#define FLY_OS_ENV_H

#include "fly_os_io.h"

/* Called from _start to register argc/argv before any fly_env_* call. */
void fly_env_init  (int argc, char **argv);

void _F6fly_os6envGet_Ss_Ss             (void *err_ctx, const fly_string *key, fly_string *out);
void _F6fly_os6envSet_Ss_Ss             (void *err_ctx, const fly_string *key, const fly_string *value);
void _F6fly_os9envDelete_Ss            (void *err_ctx, const fly_string *key);
void _F6fly_os6envAll_Cfly_string_array (void *err_ctx, fly_string_array *out);
void _F6fly_os9envExpand_Ss_Ss          (void *err_ctx, const fly_string *s, fly_string *out);
void _F6fly_os9envCwdGet_Ss            (void *err_ctx, fly_string *out);
void _F6fly_os9envCwdSet_Ss            (void *err_ctx, const fly_string *path);
void _F6fly_os11envArgsGet_Cfly_string_array (void *err_ctx, fly_string_array *out);
void _F6fly_os13envArgsCount_i        (void *err_ctx, int *out);
void _F6fly_os11envHostname_Ss         (void *err_ctx, fly_string *out);
void _F6fly_os9envOsname_Ss            (void *err_ctx, fly_string *out);
void _F6fly_os7envExit_i              (void *err_ctx, int code);

#endif /* FLY_OS_ENV_H */
