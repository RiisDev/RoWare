#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <optional>
#include <unordered_map>

#include "../../renderer/menu/ui.hpp"
#include "../../roblox/roblox.hpp"

namespace features::visuals
{
	void callback();

	void draw_3D_square(ImDrawList* draw_list, roblox::structs::part_instance_t* humanoid_root_part);

	void draw_look_line(ImDrawList* draw_list, roblox::structs::part_instance_t* head_part);

	void draw_3D_cube(ImDrawList* draw_list, roblox::structs::part_instance_t* humanoid_root_part);

	void draw_2D_box(ImDrawList* draw_list, roblox::structs::part_instance_t* humanoid_root_part);

	void draw_head_marker(ImDrawList* draw_list, roblox::structs::part_instance_t* head_part);

	void draw_tracers(ImDrawList* draw_list, roblox::structs::part_instance_t* humanoid_root_part);

	void draw_distance(ImDrawList* draw_list, roblox::structs::part_instance_t* head_part, std::uint32_t distance);

	void draw_name(ImDrawList* draw_list, roblox::structs::player_t* player, roblox::structs::part_instance_t* humanoid_root_part);

	void draw_custom_box(ImDrawList* draw_list, roblox::structs::part_instance_t* humanoid_root_part);

	void draw_tool_name(ImDrawList* draw_list, roblox::structs::player_t* player, roblox::structs::part_instance_t* humanoid_root_part);

	void draw_health(ImDrawList* draw_list, roblox::structs::player_t* player, roblox::structs::part_instance_t* humanoid_root_part);
}