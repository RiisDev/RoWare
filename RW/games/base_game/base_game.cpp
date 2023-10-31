#include "base_game.hpp"

#include "../../renderer/menu/config/config.hpp"
#include "../../roblox/roblox.hpp"
#include "../games.hpp"

#include <vector>

std::vector<roblox::structs::player_t*> roblox::games::base_game_t::get_players() const
{
	const auto players = roblox::globals::players->get_children<roblox::structs::player_t>();

	if (!players.empty())
		return players;

	return {};
}

std::optional<roblox::games::part_data_t> roblox::games::base_game_t::get_part(const roblox::structs::player_t* player) const
{
	if (!player || !roblox::functions::is_valid_ptr(player))
		return std::nullopt;

	if (const auto character = player->character; roblox::functions::is_valid_ptr(character))
		if (const auto head_part = character->find_first_child<roblox::structs::part_instance_t>("Head"))
			if (const auto torso_part = character->find_first_child<roblox::structs::part_instance_t>("HumanoidRootPart"))
				if (torso_part->is_instance_part_instance() && head_part->is_instance_part_instance())
					return roblox::games::part_data_t{ head_part, torso_part };

	return std::nullopt;
}

std::optional<float> roblox::games::base_game_t::get_health(const roblox::structs::player_t* player) const
{
	if (const auto character = player->character; roblox::functions::is_valid_ptr(character))
		if (const auto humanoid = character->find_first_child<roblox::structs::humanoid_t>("Humanoid"))
			return humanoid->get_health();

	return std::nullopt;
}

std::optional<float> roblox::games::base_game_t::get_max_health(const roblox::structs::player_t* player) const
{
	if (const auto character = player->character; roblox::functions::is_valid_ptr(character))
		if (const auto humanoid = character->find_first_child<roblox::structs::humanoid_t>("Humanoid"))
			return humanoid->get_max_health();

	return std::nullopt;
}

std::optional<const char*> roblox::games::base_game_t::get_player_name(const roblox::structs::player_t* player) const
{
	return player->name.c_str();
}

bool roblox::games::base_game_t::check_distance(roblox::games::part_data_t parts) const
{
	const auto& [head_part, torso] = parts;

	const auto lp_pos = roblox::games::game->get_local_player_position();

	if (lp_pos.has_value())
		if (head_part->primitive->cframe.position.distance(lp_pos.value()) >= std::get<int>(menu::config::config_map["max_distance_aim"]))
			return true;

	return false;
}

bool roblox::games::base_game_t::is_mouse_over_player() const
{
	const auto is_hit_player = [](const roblox::structs::instance_t* model) -> bool
	{
		const auto players = roblox::games::game->get_players();

		for (const auto player : players)
			if (const auto character = player->character; roblox::functions::is_valid_ptr(character))
				if (character == model)
				{
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

roblox::structs::player_t* roblox::games::base_game_t::get_local_player() const
{
	if (globals::data_model)
		if (const auto players = globals::data_model->find_first_class<structs::players_t>("Players"))
			if (players && players->local_player && roblox::functions::is_valid_ptr(players->local_player))
				return players->local_player;

	return nullptr;
}

std::optional<roblox::structs::instance_t*> roblox::games::base_game_t::get_local_player_character() const
{
	const auto lp = get_local_player();

	if (!lp)
		return std::nullopt;

	if (const auto lp_character = get_local_player()->character)
		return lp_character;

	return std::nullopt;
}

std::optional<math::types::vector_t> roblox::games::base_game_t::get_local_player_position() const
{
	const auto character = roblox::games::game->get_local_player_character();

	if (character.has_value())
	{
		const auto head_prim = character.value()->find_first_child<roblox::structs::part_instance_t>("Head");
		
		if (!head_prim->is_instance_part_instance())
			return std::nullopt;

		if (!head_prim || !roblox::functions::is_valid_ptr(head_prim))
		{
			const auto workspace = roblox::globals::data_model->find_first_class<roblox::structs::workspace_t>("Workspace");

			const auto camera = workspace->current_camera;

			return camera->cframe.position;
		}

		return head_prim->primitive->cframe.position;
	}

	return std::nullopt;
}

int roblox::games::base_game_t::get_stride() const
{
	return 44;
}

std::optional<int> roblox::games::base_game_t::get_index() const
{
	return std::nullopt;
}

std::optional<int> roblox::games::base_game_t::get_indesc() const
{
	return std::nullopt;
}

std::optional<const char*> roblox::games::base_game_t::get_player_tool_name(const roblox::structs::player_t* player) const
{
	if (player && roblox::functions::is_valid_ptr(player))
		if (const auto character = player->character; roblox::functions::is_valid_ptr(character))
			if (const auto tool = character->find_first_class<roblox::structs::instance_t>("Tool"); roblox::functions::is_valid_ptr(tool))
				return tool->name.c_str();

	return std::nullopt;
}

bool roblox::games::base_game_t::is_behind_wall(const roblox::structs::player_t* player, roblox::games::part_data_t parts) const
{
	if (!player || !roblox::functions::is_valid_ptr(player))
		return true;

	const auto player_character = player->character;

	if (!player_character || !roblox::functions::is_valid_ptr(player_character))
		return true;

	const auto local_player_character = get_local_player_character();

	if (!local_player_character.has_value())
		return true;

	const auto head_prim = local_player_character.value()->find_first_child<roblox::structs::part_instance_t>("Head");

	if (!head_prim->is_instance_part_instance())
		return true;

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

bool roblox::games::base_game_t::is_same_team(const roblox::structs::player_t* player) const
{
	const auto lp = get_local_player();

	if (!lp)
		return false;

	return player->team == lp->team;
}