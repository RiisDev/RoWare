#pragma once
#include <Windows.h>
#include <cstdint>

#include "../base_game/base_game.hpp"

namespace roblox::games
{
	struct arsenal_t : public base_game_t
	{
		[[nodiscard]] bool is_base() const override { return false; };

		[[nodiscard]] std::optional<float> get_health(const roblox::structs::player_t* player) const override;

		[[nodiscard]] std::optional<float> get_max_health(const roblox::structs::player_t* player) const override;
	};
}