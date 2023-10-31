#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <optional>
#include <unordered_map>

#include "../../renderer/menu/ui.hpp"
#include "../../roblox/roblox.hpp"

namespace features::misc
{
	void callback();

	void reset_delay_timer(roblox::structs::player_t* local_player);

	void trigger_bot();
}