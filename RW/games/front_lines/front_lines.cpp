#include "front_lines.hpp"

#include "../games.hpp"

std::vector<roblox::structs::player_t*> roblox::games::front_lines_t::get_players() const
{
	std::vector<roblox::structs::player_t*> fl_children;

	for (const auto& plr : roblox::globals::data_model->workspace->get_children<roblox::structs::player_t>())
	{
		if (!plr || *reinterpret_cast<const std::uint32_t*>(plr->name.c_str()) != '_51r')
			continue;

		fl_children.push_back(plr);
	}

	return fl_children;
}

std::optional<roblox::games::part_data_t> roblox::games::front_lines_t::get_part(const roblox::structs::player_t* player) const
{
	if (const auto head_part = player->find_first_child<roblox::structs::part_instance_t>("Head"); roblox::functions::is_valid_ptr(head_part))
		if (const auto torso_part = player->find_first_child<roblox::structs::part_instance_t>("HumanoidRootPart"); roblox::functions::is_valid_ptr(torso_part))
			return roblox::games::part_data_t{ head_part, torso_part };

	return std::nullopt;
}

std::optional<float> roblox::games::front_lines_t::get_health(const roblox::structs::player_t* player) const
{
	return std::nullopt;
}

std::optional<float> roblox::games::front_lines_t::get_max_health(const roblox::structs::player_t* player) const
{
	return std::nullopt;
}

//bool roblox::games::front_lines_t::is_mouse_over_player() const
//{
//	return false;
//}

bool roblox::games::front_lines_t::is_same_team(const roblox::structs::player_t* player) const
{
	return player->find_first_child("friendly_marker") != 0;
}

bool roblox::games::front_lines_t::is_behind_wall(const roblox::structs::player_t* player, roblox::games::part_data_t parts) const
{
	if (!player || !roblox::functions::is_valid_ptr(player))
		return true;

	const auto player_character = player;

	if (!player_character || !roblox::functions::is_valid_ptr(player_character))
		return true;

	const auto local_player_character = get_local_player_character();

	if (!local_player_character.has_value())
		return true;

	const auto head_prim = local_player_character.value()->find_first_child<roblox::structs::part_instance_t>("Head");

	if (!head_prim || !roblox::functions::is_valid_ptr(head_prim))
		return true;

	const auto head_pv = roblox::functions::pv::get_pv_from_part_instance(head_prim);

	if (!head_pv || !roblox::functions::is_valid_ptr(head_pv))
		return true;

	const auto& [head_part, torso] = parts;

	if (!head_part || !roblox::functions::is_valid_ptr(head_part) || !torso || !roblox::functions::is_valid_ptr(torso))
		return true;

	const auto enemy_head_pv = roblox::functions::pv::get_pv_from_part_instance(head_part);

	if (!enemy_head_pv || !roblox::functions::is_valid_ptr(enemy_head_pv))
		return true;

	auto start = head_pv->translation;
	auto dir = enemy_head_pv->translation - start;

	auto hit_instance = roblox::functions::raycast::fire_raycast_helper(roblox::globals::data_model->find_first_class("Workspace"), start, dir, local_player_character.value());

	if (!hit_instance)
		return true;

	const auto is_hit = [&]() -> bool
	{
		while (hit_instance && hit_instance->parent)
		{
			if (hit_instance->self == player_character->self)
				return true;

			hit_instance = hit_instance->parent;
		}

		return false;
	};

	return !is_hit();
}

std::optional<roblox::structs::instance_t*> roblox::games::front_lines_t::get_local_player_character() const
{
	return get_local_player();
}

roblox::structs::player_t* roblox::games::front_lines_t::get_local_player() const
{
	for (const auto players = get_players(); const auto player : players)
		if (player->find_first_child("Humanoid"))
			return player;

	return nullptr;
}

void roblox::games::front_lines_t::toggle_features() const
{
	enabled_map["enable_esp_name"] = false;
	enabled_map["allow_memory_aimbot"] = false;
	enabled_map["allow_esp_health"] = false;
	enabled_map["allow_esp_tools"] = false;
	enabled_map["allow_aimbot_health"] = false;
	enabled_map["allow_triggerbot"] = false;
	enabled_map["allow_wallcheck"] = false;
}