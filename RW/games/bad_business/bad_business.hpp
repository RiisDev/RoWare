#pragma once
#include <Windows.h>
#include <cstdint>

#include "../base_game/base_game.hpp"

namespace roblox::games
{
	struct bad_business_t : public base_game_t
	{
		[[nodiscard]] bool is_base() const override { return false; };

		[[nodiscard]] std::vector<roblox::structs::player_t*> get_players() const override;

		[[nodiscard]] std::optional<part_data_t> get_part(const roblox::structs::player_t* player) const override;

		[[nodiscard]] bool is_behind_wall(const roblox::structs::player_t* player, roblox::games::part_data_t parts) const override;

		[[nodiscard]] bool is_mouse_over_player() const override { return false; };

		[[nodiscard]] std::optional<roblox::structs::instance_t*> get_local_player_character() const override;

		[[nodiscard]] std::optional<float> get_health(const roblox::structs::player_t* player) const override;

		[[nodiscard]] std::optional<float> get_max_health(const roblox::structs::player_t* player) const override;

		[[nodiscard]] bool is_same_team(const roblox::structs::player_t* player) const override;

		void toggle_features() const override;
	};
}