#ifndef __INTERNAL_H
#define __INTERNAL_H

#include "native.h"

#ifdef __cplusplus
extern "C" {
#endif

const char *c_str(NUMTYPE cell);
NUMTYPE c_isdig(char c);
NUMTYPE c_cell_new(NUMTYPE tag, const char *value);
NUMTYPE c_cell_copy(NUMTYPE _cell);
NUMTYPE c_cell_env_get(NUMTYPE _cell);
NUMTYPE c_cell_env_set(NUMTYPE _env, NUMTYPE _cell);
NUMTYPE c_cell_list(NUMTYPE _cell);
NUMTYPE c_cell_strcmp(const char *s, NUMTYPE _cell);
NUMTYPE c_cell_type(NUMTYPE _cell);
NUMTYPE c_cell_value(NUMTYPE _cell); // const char *
NUMTYPE c_list(NUMTYPE _content);
NUMTYPE c_list_empty(NUMTYPE _list);
NUMTYPE c_list_size(NUMTYPE _list);
NUMTYPE c_list_index(NUMTYPE index, NUMTYPE _list);
NUMTYPE c_list_push_back(NUMTYPE _cell, NUMTYPE _list);
NUMTYPE c_atom_false();
NUMTYPE c_atom_true();
NUMTYPE c_atom_nil();

NUMTYPE c_free_cell(NUMTYPE _cell);
NUMTYPE c_free_env(NUMTYPE _env);

NUMTYPE c_environment(NUMTYPE _outer);
NUMTYPE c_env_get(const char *_name, NUMTYPE _env);
NUMTYPE c_env_has(const char *_name, NUMTYPE _env);
NUMTYPE c_env_set(const char *_name, NUMTYPE _value, NUMTYPE _env);

NUMTYPE c_add_globals(NUMTYPE _env);
NUMTYPE c_call_proc(NUMTYPE _cell, NUMTYPE _args);

NUMTYPE c_parse(const char *code, NUMTYPE _dest);
NUMTYPE lispmain(NUMTYPE, char **);

NUMTYPE internal_syscall1(NUMTYPE);
NUMTYPE internal_syscall2(NUMTYPE, NUMTYPE);
NUMTYPE internal_syscall3(NUMTYPE, NUMTYPE, NUMTYPE);
NUMTYPE internal_syscall4(NUMTYPE, NUMTYPE, NUMTYPE, NUMTYPE);

#ifdef __cplusplus
}
#endif
#endif
