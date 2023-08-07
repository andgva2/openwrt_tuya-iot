#include <script_utils.h>

/// search for files in scripts directory with extention .lua, ignore others
/// \param[in] dir_path path to directory
/// \param[out] files array of files
/// \param[out] files_count count of files
/// \return 0 if success, 1 if error
int get_lua_files(const char *dir_path, char ***files, int *files_count)
{
	DIR *dir;
	struct dirent *ent;
	int count	  = 0;
	char **files_list = NULL;
	char *file_path	  = NULL;
	int file_path_len = 0;

	if ((dir = opendir(dir_path)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_type == DT_REG) {
				if (count == 15) {
					break;
				}
				if (strstr(ent->d_name, ".lua") != NULL) {
					count++;
					files_list    = realloc(files_list, count * sizeof(char *));
					file_path_len = strlen(dir_path) + strlen(ent->d_name) + 2;
					file_path     = malloc(file_path_len);
					snprintf(file_path, file_path_len, "%s/%s", dir_path, ent->d_name);
					files_list[count - 1] = file_path;
				}
			}
		}
		closedir(dir);
		syslog(LOG_INFO, "INFO: Found %d lua files in %s", count, dir_path);
	} else {
		syslog(LOG_ERR, "ERROR: Can't open directory %s", dir_path);
		return 1;
	}

	*files	     = files_list;
	*files_count = count;

	return 0;
}

void lua_err(lua_State *L, char *msg)
{
	syslog(LOG_ERR, "ERROR: %s: %s", msg, lua_tostring(L, -1));
}

///

int get_data_from_lua_file(const char *file_path, char **json_data)
{
	int ret = 0;
	lua_State *L;
	L = luaL_newstate();
	luaL_openlibs(L);
	if (luaL_loadfile(L, file_path)) {
		lua_err(L, "luaL_loadfile() failed");
	}
	if (lua_pcall(L, 0, 0, 0)) {
		lua_err(L, "lua_pcall() failed");
	}

	lua_getglobal(L, "get_data");
	if (lua_isfunction(L, 0) == 0) {
		lua_err(L, "lua_isfunction() failed");
		ret = 1;
		goto cleanup;
	}

	lua_pop(L, 1);

	lua_getglobal(L, "init");
	if (lua_isfunction(L, 0) == 0) {
		lua_pop(L, 1);
	} else {
		if (lua_pcall(L, 0, 0, 0)) {
			lua_err(L, "lua_pcall() failed");
		}
	}

	lua_getglobal(L, "get_data");
	if (lua_pcall(L, 0, 1, 0)) {
		lua_err(L, "lua_pcall() failed");
	}

	lua_getglobal(L, "deinit");
	if (lua_isfunction(L, 0) == 0) {
		lua_pop(L, 1);
	} else {
		if (lua_pcall(L, 0, 0, 0)) {
			lua_err(L, "lua_pcall() failed");
		}
	}
	*json_data = strdup(lua_tostring(L, -1));

cleanup:;
	lua_close(L);
	return ret;
}
