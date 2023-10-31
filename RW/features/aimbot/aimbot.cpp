#include "aimbot.hpp"

#include "../../renderer/menu/config/config.hpp"
#include "../../renderer/renderer.hpp"
#include "../../games/games.hpp"

#include <random>

roblox::structs::player_t* modules::aimbot::get_closest_alive_player()
{
	float closest_distance = (std::numeric_limits<float>::max)();
	roblox::structs::player_t* player = nullptr;

	if (!roblox::globals::players || !roblox::functions::is_valid_ptr(roblox::globals::players))
		return nullptr;

	for (const auto& child : roblox::games::game->get_players())
	{
		const auto local_player = roblox::games::game->get_local_player();

		if (!child || child == local_player || !roblox::functions::is_valid_ptr(child) || !local_player || child == roblox::games::game->get_local_player_character())
			continue;

		if (child->class_descriptor->id != 0x4A && child->class_descriptor->id != 0x23) //checks if class is either player or model instance
			continue;

		if (std::get<bool>(menu::config::config_map["enable_team_check"]))
			if (roblox::games::game->is_same_team(child))
				continue;

		const auto parts = roblox::games::game->get_part(child);

		if (parts.has_value())
		{
			const auto& [torso_opt, head_part_opt] = parts.value();

			if (!head_part_opt || !torso_opt)
				continue;

			const auto head_part = head_part_opt;
			const auto torso = torso_opt;

			const auto head_part_primitive = head_part->primitive;

			if (!head_part_primitive)
				continue;

			const auto head_part_pv = roblox::functions::pv::get_pv_from_part_instance(head_part);

			if (!head_part_pv)
				continue;
			const auto head_position = math::functions::world_to_screen(head_part_pv->translation);

			if (!head_position.has_value() || !roblox::functions::is_valid_ptr(torso))
				continue;

			if (std::get<bool>(menu::config::config_map["enable_wall_check"]))
				if (roblox::games::game->is_behind_wall(child, parts.value()))
					continue;

			if (std::get<bool>(menu::config::config_map["max_distance_aim_enabled"]))
				if (roblox::games::game->check_distance(parts.value()))
					continue;

			const auto& head_screen_position = head_position.value();

			POINT p;

			if (!GetCursorPos(&p))
				continue;

			if (!ScreenToClient(renderer::values::roblox_window, &p))
				continue;

			// Fix for dpi scaling
			p.x /= renderer::values::dpi_scale;
			p.y /= renderer::values::dpi_scale;

			const math::types::vector2_t mouse_loc{ static_cast<float>(p.x), static_cast<float>(p.y) };

			const auto distance = mouse_loc.distance(head_screen_position);

			if (std::get< bool >(menu::config::config_map["enable_fov_check"]))
				if (std::get<int>(menu::config::config_map["fov_amount"]) * 10.f < distance)
					continue;

			if (distance < closest_distance)
			{
				closest_distance = distance;

				player = child;
			}

		}
	}

	return player;
}

void modules::aimbot::callback()
{
	if (!roblox::globals::players || !roblox::globals::data_model || !roblox::globals::data_model->is_loaded)
		return;

	const auto smoothing_max = std::get< int >(menu::config::config_map["smoothing_amount"]);

	static bool locked = false;

	static bool head = false;

	static roblox::structs::player_t* player = nullptr;

	if (!GetAsyncKeyState(std::get< int >(menu::config::config_map["aim_hotkey"])) || !std::get< bool >(menu::config::config_map["enable_aimbot"]))
	{
		locked = false;

		player = nullptr;

		return;
	}

	if (!locked || !player || !roblox::functions::is_valid_ptr(player))
	{
		player = get_closest_alive_player();

		locked = true;

		std::random_device rd;
		std::mt19937 mt(rd());
		std::bernoulli_distribution dist(0.5);

		head = dist(mt);
	}

	if(player->aim_at(!std::get< int >(menu::config::config_map["aimbot_type"]), smoothing_max, std::get<int>(menu::config::config_map["target_bone"]) == 2 ? head : !std::get<int>(menu::config::config_map["target_bone"])))
	{
		locked = false;

		player = nullptr;

		return;
	}
}

void modules::aimbot::draw_fov()
{
	if (!std::get<bool>(menu::config::config_map["draw_fov"]))
		return;

	POINT p;

	if (GetCursorPos(&p))
	{
		if (ScreenToClient(renderer::values::roblox_window, &p))
		{
			const auto radius = std::get<int>(menu::config::config_map["fov_amount"]) * 10;

			const auto draw_list = ImGui::GetBackgroundDrawList();

			// Fix for dpi scaling
			p.x /= renderer::values::dpi_scale;
			p.y /= renderer::values::dpi_scale;

			draw_list->AddCircleFilled({ static_cast<float>(p.x), static_cast<float>(p.y) }, radius, ImColor{ 0.0f, 0.0f, 0.0f, 0.2f });
		}
	}
}