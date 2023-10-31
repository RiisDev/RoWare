#pragma once
#include <Windows.h>
#include <cstdint>

#include "../../roblox/roblox.hpp"

namespace roblox::games
{
	struct part_data_t
	{
		roblox::structs::part_instance_t* head;
		roblox::structs::part_instance_t* torso;
	};

	struct base_game_t
	{
		[[nodiscard]] virtual bool is_base() const { return true; };

		[[nodiscard]] virtual std::vector<roblox::structs::player_t*> get_players() const;

		[[nodiscard]] virtual std::optional<part_data_t> get_part(const roblox::structs::player_t* player) const;

		[[nodiscard]] virtual std::optional<float> get_health(const roblox::structs::player_t* player) const;

		[[nodiscard]] virtual std::optional<float> get_max_health(const roblox::structs::player_t* player) const;

		[[nodiscard]] virtual bool is_behind_wall(const roblox::structs::player_t* player, roblox::games::part_data_t parts) const;

		[[nodiscard]] virtual bool is_same_team(const roblox::structs::player_t* player) const;

		[[nodiscard]] virtual std::optional<const char*> get_player_name(const roblox::structs::player_t* player) const;

		[[nodiscard]] virtual std::optional<const char*> get_player_tool_name(const roblox::structs::player_t* player) const;

		[[nodiscard]] virtual bool is_mouse_over_player() const;

		[[nodiscard]] virtual int get_stride() const;

		[[nodiscard]] virtual std::optional<int> get_index() const;

		[[nodiscard]] virtual std::optional<int> get_indesc() const;

		[[nodiscard]] virtual bool check_distance(roblox::games::part_data_t parts) const;

		[[nodiscard]] virtual std::optional<roblox::structs::instance_t*> get_local_player_character() const;

		[[nodiscard]] virtual std::optional<math::types::vector_t> get_local_player_position() const;

		[[nodiscard]] virtual structs::player_t* get_local_player() const;

		virtual void toggle_features() const {};
	};
}