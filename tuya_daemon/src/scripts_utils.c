#include <script_utils.h>

void lua_err(lua_State *L, char *msg)
{
	syslog(LOG_USER | LOG_ERR, "ERROR: %s: %s", msg, lua_tostring(L, -1));
}

int lua_state_init(char *file_path, lua_State **L)
{
	int ret = 0;
	*L	= luaL_newstate();
	luaL_openlibs(*L);

	if (luaL_loadfile(*L, file_path)) {
		lua_err(*L, "luaL_loadfile() failed");
		ret = 1;
		goto cleanup;
	}
	if (lua_pcall(*L, 0, 0, 0)) {
		lua_err(*L, "lua_pcall() failed");
		ret = 1;
		goto cleanup;
	}

	return ret;

cleanup:;
	lua_close(*L);
	return ret;
}

int does_lua_fun_exist(char *file_path, char *fun_name)
{
	int ret = 0;
	lua_State *L;
	if (lua_state_init(file_path, &L)) {
		ret = -1;
		return ret;
	}

	lua_getglobal(L, fun_name);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		ret = 0;
		goto cleanup;
	} else {
		lua_pop(L, 1);
		ret = 1;
		goto cleanup;
	}

cleanup:;
	lua_close(L);
	return ret;
}

void try_call_lua_fun_no_args_no_ret(lua_State *L, char *file_path, char *fun_name)
{
	lua_getglobal(L, fun_name);
	if (does_lua_fun_exist(file_path, fun_name) == 1) {
		if (lua_pcall(L, 0, 0, 0)) {
			lua_err(L, "lua_pcall() failed");
		}
	} else {
		lua_pop(L, 1);
	}
}

int get_data_from_lua_file(char *file_path, char **payload)
{
	int ret = 0;
	lua_State *L;
	if (lua_state_init(file_path, &L)) {
		ret = -1;
		return ret;
	}

	syslog(LOG_USER | LOG_INFO, "INFO: Lua file %s loaded", file_path);

	try_call_lua_fun_no_args_no_ret(L, file_path, "init");

	syslog(LOG_USER | LOG_INFO, "INFO: Tried to call lua function init()");

	lua_getglobal(L, "get_data");
	if (lua_pcall(L, 0, 1, 0)) {
		lua_err(L, "lua_pcall() failed");
		ret = 1;
		goto cleanup;
	}

	syslog(LOG_USER | LOG_INFO, "INFO: Lua function get_data() called");

	*payload = strdup(lua_tostring(L, -1));

	try_call_lua_fun_no_args_no_ret(L, file_path, "deinit");

	syslog(LOG_USER | LOG_INFO, "INFO: Tried to call lua function deinit()");

cleanup:;
	lua_close(L);
	return ret;
}

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
					// +2 for '/' and '\0'
					file_path_len = strlen(dir_path) + strlen(ent->d_name) + 2;
					file_path     = malloc(file_path_len);
					if (file_path == NULL) {
						syslog(LOG_USER | LOG_ERR,
						       "ERROR: Can't allocate memory for file_path");
						continue;
					}
					snprintf(file_path, file_path_len, "%s/%s", dir_path, ent->d_name);
					if (does_lua_fun_exist(file_path, "get_data") == 1) {
						count++;
						files_list = realloc(files_list, count * sizeof(char *));
						files_list[count - 1] = file_path;
					} else {
						free(file_path);
					}
				}
			}
		}
		closedir(dir);
		syslog(LOG_USER | LOG_INFO, "INFO: Found %d lua files in %s", count, dir_path);
	} else {
		syslog(LOG_USER | LOG_ERR, "ERROR: Can't open directory %s", dir_path);
		return 1;
	}

	*files	     = files_list;
	*files_count = count;

	return 0;
}