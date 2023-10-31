#include "phantom_forces.hpp"

#include "../../renderer/menu/config/config.hpp"
#include "../games.hpp"

std::vector<roblox::structs::player_t*> roblox::games::phantom_forces_t::get_players() const
{
	std::vector<roblox::structs::player_t*> pf_children;

	const auto pf_players_folder = roblox::globals::data_model->workspace->find_first_child("Players");

	if (!pf_players_folder || !roblox::functions::is_valid_ptr(pf_players_folder))
		return pf_children;

	for (const auto& team : pf_players_folder->get_children())
	{
		if (!team || !roblox::functions::is_valid_ptr(team))
			continue;

		for (const auto& child : team->get_children<roblox::structs::player_t>())
		{
			if (!child || !roblox::functions::is_valid_ptr(child))
				continue;

			pf_children.push_back(child);
		}
	}

	return pf_children;
}

std::optional<roblox::games::part_data_t> roblox::games::phantom_forces_t::get_part(const roblox::structs::player_t* player) const
{
	if (const auto head_part = player->find_first_child<roblox::structs::part_instance_t>("Head"); roblox::functions::is_valid_ptr(head_part))
		if (const auto torso_part = player->find_first_child<roblox::structs::part_instance_t>("Torso"); roblox::functions::is_valid_ptr(torso_part))
			return roblox::games::part_data_t{ head_part, torso_part };

	return std::nullopt;
}

void roblox::games::phantom_forces_t::toggle_features() const
{
	enabled_map["enable_esp_name"] = false;
	enabled_map["allow_memory_aimbot"] = false;
	enabled_map["allow_esp_health"] = false;
	enabled_map["allow_esp_tools"] = false;
}

bool roblox::games::phantom_forces_t::is_behind_wall(const roblox::structs::player_t* player, roblox::games::part_data_t parts) const
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

	auto start = head_pv->translation + math::types::vector_t{ 0, 1, 0 };
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

bool roblox::games::phantom_forces_t::is_mouse_over_player() const
{
	const auto local_player_character = get_local_player_character();

	if (!local_player_character.has_value())
		return false;

	const auto head_prim = local_player_character.value()->find_first_child<roblox::structs::part_instance_t>("Head");

	if (!head_prim || !roblox::functions::is_valid_ptr(head_prim))
		return false;

	const auto workspace = roblox::globals::data_model->find_first_class<roblox::structs::workspace_t>("Workspace");

	const auto camera = workspace->current_camera;

	if (!camera)
		return false;

	auto start = camera->cframe.position;
	auto dir = math::types::vector_t{ -camera->cframe.right_vector.z, -camera->cframe.up_vector.z, -camera->cframe.back_vector.z } *1000.f;

	auto hit_instance = roblox::functions::raycast::fire_raycast_helper(roblox::globals::data_model->find_first_class("Workspace"), start, dir, local_player_character.value());

	if (!hit_instance)
		return false;

	const auto is_hit = [&]() -> bool
	{
		if (hit_instance && hit_instance->parent)
		{
			if (std::strstr(hit_instance->parent->name.c_str(), "Player") != 0)
			{
				if (std::get<bool>(menu::config::config_map["enable_team_check"]))
					if (roblox::games::game->is_same_team(static_cast<roblox::structs::player_t*>(hit_instance->parent)))
						return false;

				return true;
			}

			hit_instance = hit_instance->parent;
		}

		return false;
	};

	return is_hit();
}

bool roblox::games::phantom_forces_t::is_same_team(const roblox::structs::player_t* player) const
{
	const auto lp = get_local_player();

	if (!lp || !player || !player->parent)
		return false;

	const auto team = reinterpret_cast<roblox::structs::instance_t*>(lp->team);

	const auto enemy_team_color = player->parent->name;

	auto enemy_team_name = "";

	if (enemy_team_color == "Bright blue")
		enemy_team_name = "Phantoms";
	else
		enemy_team_name = "Ghosts";

	return std::strcmp(enemy_team_name, team->name.c_str()) == 0;
}

int roblox::games::phantom_forces_t::get_stride() const
{
	return 32;
}

std::optional<int> roblox::games::phantom_forces_t::get_index() const
{
	return 132;
}

std::optional<int> roblox::games::phantom_forces_t::get_indesc() const
{
	return 264;
}