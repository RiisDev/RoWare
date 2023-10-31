#pragma once
#include <Windows.h>
#include <cstdint>

#include "../base_game/base_game.hpp"

namespace roblox::games
{
	struct front_lines_t : public base_game_t
	{
		[[nodiscard]] virtual std::vector<roblox::structs::player_t*> get_players() const;

		[[nodiscard]] std::optional<part_data_t> get_part(const roblox::structs::player_t* player) const override;

		[[nodiscard]] std::optional<float> get_health(const roblox::structs::player_t* player) const override;

		[[nodiscard]] std::optional<float> get_max_health(const roblox::structs::player_t* player) const override;

		[[nodiscard]] bool is_same_team(const roblox::structs::player_t* player) const override;

		[[nodiscard]] bool is_behind_wall(const roblox::structs::player_t* player, roblox::games::part_data_t parts) const override;

		[[nodiscard]] virtual std::optional<roblox::structs::instance_t*> get_local_player_character() const;

		[[nodiscard]] virtual structs::player_t* get_local_player() const;

		void toggle_features() const override;
	};
}