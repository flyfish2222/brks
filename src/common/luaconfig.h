#ifndef BRKS_COMMON_LUANCONFIG_H_
#define BRKS_COMMON_LUANCONFIG_H_

#include <string>

#include "configdef.h"

class LuaConfig
{
public:
	LuaConfig();
	virtual ~LuaConfig();
	bool DoFile(const std::string& path);
	const st_env_config& GetConfig();

	explicit operator bool() const;

private:
	st_env_config m_config;
	bool m_bSucess;
};

#endif

