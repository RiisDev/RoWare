#pragma once
#include <Windows.h>
#include <cstdint>
#include <unordered_map>
#include <memory>

#include "base_game/base_game.hpp"

namespace roblox::games
{
	enum class game_t : std::uint64_t
	{
		phantom_forces = 292439477,
		arsenal = 286090429,
		bad_business = 3233893879,
		rush_point = 7113341058,
		front_lines = 5938036553
	};

	extern std::unordered_map<const char*, bool> enabled_map;

	extern std::shared_ptr<roblox::games::base_game_t> game;

	void init_game();
}