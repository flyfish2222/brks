#include "luaconfig.h"
#include <iostream>

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}



LuaConfig::LuaConfig() : m_bSucess(false)
{

}

LuaConfig::~LuaConfig()
{

}

bool LuaConfig::DoFile(const std::string& path)
{
	lua_State* luaState = luaL_newstate();
	luaL_openlibs(luaState);
	bool err = luaL_loadfile(luaState, path.c_str())
		|| lua_pcall(luaState, 0, 0, 0);

	if (err)
	{
		printf("%s\n", lua_tostring(luaState, -1));
		lua_close(luaState);
		return false;
	}

	lua_getglobal(luaState, "json");
	lua_pushstring(luaState, "db");
	lua_gettable(luaState, -2);

	lua_pushstring(luaState, "ip");
	lua_gettable(luaState, -2);
	this->m_config.db_ip = lua_tostring(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, "port");
	lua_gettable(luaState, -2);
	this->m_config.db_port = lua_tointeger(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, "user");
	lua_gettable(luaState, -2);
	this->m_config.db_user = lua_tostring(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, "pwd");
	lua_gettable(luaState, -2);
	this->m_config.db_pwd = lua_tostring(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, "name");
	lua_gettable(luaState, -2);
	this->m_config.db_name = lua_tostring(luaState, -1);
	lua_pop(luaState, 1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, "brk_port");
	lua_gettable(luaState, -2);
	this->m_config.svr_port = lua_tointeger(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, "log_conf");
	lua_gettable(luaState, -2);
	this->m_config.log_conf = lua_tostring(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, "process_count");
	lua_gettable(luaState, -2);
	this->m_config.process_count = lua_tointeger(luaState, -1);
	lua_pop(luaState, 1);

	if (this->m_config.process_count <= 0)
		this->m_config.process_count = 4;

	lua_pop(luaState, 1);
	lua_close(luaState);

	this->m_bSucess = true;
	return true;
}

LuaConfig::operator bool() const
{
	return this->m_bSucess;
}

const st_env_config& LuaConfig::GetConfig()
{
	return this->m_config;
}