#include "rush_point.hpp"

#include "../../renderer/menu/config/config.hpp"
#include "../games.hpp"

std::vector<roblox::structs::player_t*> roblox::games::rush_point_t::get_players() const
{
	std::vector<roblox::structs::player_t*> rp_children;

	const auto map = roblox::globals::data_model->workspace->find_first_child("MapFolder");

	if (!map)
		return rp_children;

	const auto rp_players_folder = map->find_first_child("Players");

	if (!rp_players_folder || !roblox::functions::is_valid_ptr(rp_players_folder))
		return rp_children;

	for (const auto& plr : rp_players_folder->get_children<roblox::structs::player_t>())
	{
		if (!plr || !roblox::functions::is_valid_ptr(plr) || !plr->find_first_class("Humanoid"))
			continue;

		rp_children.push_back(plr);
	}

	return rp_children;
}

std::optional<roblox::games::part_data_t> roblox::games::rush_point_t::get_part(const roblox::structs::player_t* player) const
{
	if (const auto head_part = player->find_first_child<roblox::structs::part_instance_t>("Head"); roblox::functions::is_valid_ptr(head_part))
		if (const auto torso_part = player->find_first_child<roblox::structs::part_instance_t>("HumanoidRootPart"); roblox::functions::is_valid_ptr(torso_part))
			return roblox::games::part_data_t{ head_part, torso_part };

	return std::nullopt;
}

std::optional<float> roblox::games::rush_point_t::get_health(const roblox::structs::player_t* player) const
{
	if (const auto humanoid = player->find_first_child<roblox::structs::humanoid_t>("Humanoid"))
		return humanoid->get_health();

	return std::nullopt;
}

std::optional<float> roblox::games::rush_point_t::get_max_health(const roblox::structs::player_t* player) const
{
	if (const auto humanoid = player->find_first_child<roblox::structs::humanoid_t>("Humanoid"))
		return humanoid->get_max_health();

	return std::nullopt;
}

void roblox::games::rush_point_t::toggle_features() const
{
	enabled_map["allow_esp_tools"] = false;
}

bool roblox::games::rush_point_t::is_mouse_over_player() const
{
	const auto is_hit_player = [](const roblox::structs::instance_t* model) -> bool
	{
		if (model->parent && std::strstr(model->parent->name.c_str(), "Players") != 0)
		{
			const auto player = static_cast<const roblox::structs::player_t*>(model);

			if (std::get<bool>(menu::config::config_map["enable_team_check"]))
				if (roblox::games::game->is_same_team(player))
					return false;

			if (std::get<bool>(menu::config::config_map["enable_aim_health_check"]))
			{
				const auto health_optional = roblox::games::game->get_health(player);

				if (health_optional.has_value())
				{
					const auto health = health_optional.value();

					if (health <= 0)
						return false;
				}
			}

			return true;
		}

		return false;
	};

	if (const auto mouse = get_local_player()->mouse; roblox::functions::is_valid_ptr(mouse))
	{
		auto hit = roblox::functions::mouse::get_hit(mouse);

		if (hit && roblox::functions::is_valid_ptr(hit))
		{
			while (true)
			{
				if (hit->class_descriptor->name == "Model")
					if (is_hit_player(hit))
						return true;

				if (!hit->parent)
					break;

				hit = hit->parent;
			}
		}
	}

	return false;
}

bool roblox::games::rush_point_t::is_same_team(const roblox::structs::player_t* player) const
{
	const auto lp = get_local_player();

	if (!lp)
		return false;

	const auto lp_char = lp->character;

	if (!lp_char || !player->character)
		return false;

	const auto lp_team = lp_char->find_first_child("Team");
	const auto plr_team = player->find_first_child("Team");

	if (!lp_team || !plr_team)
		return false;

	const auto lp_team_name = *reinterpret_cast<std::uint32_t*>(lp_team->self + 0x6C);
	const auto plr_team_name = *reinterpret_cast<std::uint32_t*>(plr_team->self + 0x6C);

	return lp_team_name == plr_team_name;
}

bool roblox::games::rush_point_t::is_behind_wall(const roblox::structs::player_t* player, roblox::games::part_data_t parts) const
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