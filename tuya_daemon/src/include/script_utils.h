#ifndef SCRIPT_UTILS_H
#define SCRIPT_UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <syslog.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int get_data_from_lua_file(char *file_path, char **payload);
int get_lua_files(const char *dir_path, char ***files, int *files_count);

#endif