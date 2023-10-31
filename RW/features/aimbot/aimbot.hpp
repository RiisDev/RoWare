#pragma once

#include "../../roblox/roblox.hpp"

namespace modules::aimbot
{
	roblox::structs::player_t* get_closest_alive_player();

	void callback();

	void draw_fov();
}