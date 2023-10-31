#pragma once

#include <unordered_map>

#include <variant>

#include "../../../roblox/roblox.hpp"

#include "../../dependencies/imgui/imgui.h"

namespace menu::config
{
	extern std::vector<std::string> configs;

	extern std::unordered_map< const char*, std::variant< bool, int, float, ImU32 > > config_map;

	extern std::unordered_map< const char*, std::variant< bool, int, float, ImU32 > > default_config;

	void dump_to_cfg(const char* const name);

	void delete_cfg(const char* const name);

	void load_cfg(const char* const name);

	void refresh_configs();
}

