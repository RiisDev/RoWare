#include "bad_business.hpp"

#include "../games.hpp"

void roblox::games::bad_business_t::toggle_features() const
{
	enabled_map["enable_esp_name"] = false;
	enabled_map["allow_memory_aimbot"] = false;
	enabled_map["allow_esp_tools"] = false;
	enabled_map["allow_triggerbot"] = false;
}

roblox::structs::player_t* lp = nullptr;

auto start = std::chrono::steady_clock::now();

std::optional<roblox::structs::instance_t*> roblox::games::bad_business_t::get_local_player_character() const
{
	if (lp && (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start) < std::chrono::seconds(1)))
		return lp;

	float closest_distance = 9999.f;

	const auto cam_pos = roblox::globals::data_model->find_first_class<roblox::structs::workspace_t>("Workspace")->current_camera->cframe.position;

	for (const auto players = get_players(); const auto player : players)
	{
		const auto parts = get_part(player);

		if (!parts.has_value())
			continue;

		const auto [head, body] = parts.value();

		const auto head_dist = head->primitive->cframe.position.distance(cam_pos);

		if (head_dist < closest_distance)
		{
			lp = player;

			closest_distance = head_dist;
		}
	}

	start = std::chrono::steady_clock::now();

	return lp;
}

std::vector<roblox::structs::player_t*> roblox::games::bad_business_t::get_players() const
{
	std::vector<roblox::structs::player_t*> bb_children;

	const auto bb_players_folder = roblox::globals::data_model->workspace->find_first_child("Characters");

	if (!bb_players_folder || !roblox::functions::is_valid_ptr(bb_players_folder))
		return bb_children;

	for (const auto& plr : bb_players_folder->get_children<roblox::structs::player_t>())
	{
		if (!plr || !roblox::functions::is_valid_ptr(plr))
			continue;

		bb_children.push_back(plr);
	}

	return bb_children;
}

std::optional<roblox::games::part_data_t> roblox::games::bad_business_t::get_part(const roblox::structs::player_t* player) const
{
	const auto body = player->find_first_child<roblox::structs::part_instance_t>("Body");

	if (!body || !roblox::functions::is_valid_ptr(body))
		return std::nullopt;

	if (const auto head_part = body->find_first_child<roblox::structs::part_instance_t>("Head"); roblox::functions::is_valid_ptr(head_part))
		if (const auto torso_part = body->find_first_child<roblox::structs::part_instance_t>("Chest"); roblox::functions::is_valid_ptr(torso_part))
			return roblox::games::part_data_t{ head_part, torso_part };

	return std::nullopt;
}

std::optional<float> roblox::games::bad_business_t::get_health(const roblox::structs::player_t* player) const
{
	const auto stats = player->find_first_child< roblox::structs::number_value>("Health");

	if (!stats || !roblox::functions::is_valid_ptr(stats))
		return std::nullopt;

	return stats->read_value(*reinterpret_cast<std::uint32_t*>(roblox::offsets::number_xor::xor_offset));
}

std::optional<float> roblox::games::bad_business_t::get_max_health(const roblox::structs::player_t* player) const
{
	const auto stats = player->find_first_child<roblox::structs::number_value>("Health");

	if (!stats || !roblox::functions::is_valid_ptr(stats))
		return std::nullopt;

	const auto health = stats->find_first_child<roblox::structs::number_value>("MaxHealth");

	if (!health || !roblox::functions::is_valid_ptr(health))
		return std::nullopt;

	return health->read_value(*reinterpret_cast<std::uint32_t*>(roblox::offsets::number_xor::xor_offset));
}

bool roblox::games::bad_business_t::is_behind_wall(const roblox::structs::player_t* player, roblox::games::part_data_t parts) const
{
	const auto player_character = player;

	if (!player_character || !roblox::functions::is_valid_ptr(player_character))
		return true;

	const auto local_player_character = get_local_player_character();

	if (!local_player_character.has_value())
		return true;

	const auto body = local_player_character.value()->find_first_child("Body");

	if (!body || !roblox::functions::is_valid_ptr(body))
		return true;

	const auto head_prim = body->find_first_child<roblox::structs::part_instance_t>("Head");

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

	auto start = math::types::vector_t{ head_pv->translation.x, head_pv->translation.y + 1, head_pv->translation.z };
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

std::vector<roblox::structs::player_t*> teammates;

auto teammates_start = std::chrono::steady_clock::now();

bool roblox::games::bad_business_t::is_same_team(const roblox::structs::player_t* player) const
{
	if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - teammates_start) < std::chrono::seconds(1))
	{
		for (const auto teammate : teammates)
			if (player == teammate)
				return true;

		return false;
	}

	std::vector<roblox::structs::player_t*> temp_teammates;

	const auto lp = roblox::games::game->get_local_player();

	if (lp)
	{
		const auto guis = lp->find_first_child("PlayerGui");

		if (!guis)
			return false;

		const auto childs = guis->get_children();

		for (const auto child : childs)
		{
			if (child->name == "NameGui")
			{
				const auto billboard = static_cast<roblox::structs::billboard_t*>(child);

				const auto head = billboard->adornee;

				const auto character = head->parent;

				if (character)
					temp_teammates.push_back(static_cast<roblox::structs::player_t*>(character->parent));
			}
		}

		teammates = temp_teammates;
	}

	teammates_start = std::chrono::steady_clock::now();

	return false;
}