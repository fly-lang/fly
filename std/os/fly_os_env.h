/*===-- std/os/fly_os_env.h - fly.os environment API --------------------===*/

#ifndef FLY_OS_ENV_H
#define FLY_OS_ENV_H

#include "fly_os_io.h"

/* Called from _start to register argc/argv before any fly_env_* call. */
void fly_env_init  (int argc, char **argv);

void fly_env_get      (const fly_string *key, fly_string *out);
void fly_env_set      (const fly_string *key, const fly_string *value);
void fly_env_delete   (const fly_string *key);
void fly_env_all      (fly_string_array *out);
void fly_env_expand   (const fly_string *s, fly_string *out);
void fly_env_cwdGet   (fly_string *out);
void fly_env_cwdSet   (const fly_string *path);
void fly_env_argsGet  (fly_string_array *out);
void fly_env_argsCount(int *out);
void fly_env_hostname (fly_string *out);
void fly_env_osname   (fly_string *out);
void fly_env_exit     (int code);

#endif /* FLY_OS_ENV_H */
