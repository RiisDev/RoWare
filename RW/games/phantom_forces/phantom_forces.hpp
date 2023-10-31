#pragma once
#include <Windows.h>
#include <cstdint>

#include "../base_game/base_game.hpp"

namespace roblox::games
{
	struct phantom_forces_t : public base_game_t
	{
		[[nodiscard]] bool is_base() const override { return false; };

		[[nodiscard]] std::vector<roblox::structs::player_t*> get_players() const override;

		[[nodiscard]] std::optional<part_data_t> get_part(const roblox::structs::player_t* player) const override;

		[[nodiscard]] std::optional<float> get_health(const roblox::structs::player_t* player) const override { return std::nullopt; }

		[[nodiscard]] std::optional<float> get_max_health(const roblox::structs::player_t* player) const override { return std::nullopt; }

		[[nodiscard]] std::optional<const char*> get_player_name(const roblox::structs::player_t* player) const override { return std::nullopt; }

		[[nodiscard]] std::optional<const char*> get_player_tool_name(const roblox::structs::player_t* player) const override { return std::nullopt; }

		[[nodiscard]] bool is_behind_wall(const roblox::structs::player_t* player, roblox::games::part_data_t parts) const override;

		[[nodiscard]] bool is_mouse_over_player() const override;

		[[nodiscard]] int get_stride() const override;

		[[nodiscard]] std::optional<int> get_index() const override;

		[[nodiscard]] std::optional<int> get_indesc() const override;

		[[nodiscard]] bool is_same_team(const roblox::structs::player_t* player) const override;

		void toggle_features() const override;
	};
}