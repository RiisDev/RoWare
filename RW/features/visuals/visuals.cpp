#include "visuals.hpp"

#include "../../roblox/roblox.hpp"
#include "../../games/games.hpp"

float rainbow_count = 0;
float offset = 0;

void features::visuals::callback()
{
	rainbow_count += 0.001;

	if (rainbow_count >= 1) rainbow_count = 0;

	if (!roblox::globals::players || !roblox::globals::data_model || !roblox::globals::data_model->is_loaded)
		return;

	const auto children = roblox::globals::players->get_children<roblox::structs::player_t>();

	for (const auto& child : roblox::games::game->get_players())
	{
		const auto local_player = roblox::games::game->get_local_player();

		if (!child || child == local_player || child == roblox::games::game->get_local_player_character() || !roblox::functions::is_valid_ptr(child) || !local_player)
			continue;

		if (child->class_descriptor->id != 0x4A && child->class_descriptor->id != 0x23) //checks if class is either player or model instance
			continue;

		if (std::get<bool>(menu::config::config_map["enemy_only_esp"]) && roblox::games::game->is_same_team(child))
			continue;
		
		const auto parts = roblox::games::game->get_part(child);

		if (parts.has_value())
		{
			const auto& [head_part_opt, torso_opt] = parts.value();

			if (!head_part_opt || !torso_opt)
				continue;

			const auto humanoid_root_part_pv = roblox::functions::pv::get_pv_from_part_instance(torso_opt); const auto head_part_pv = roblox::functions::pv::get_pv_from_part_instance(head_part_opt);

			if (!humanoid_root_part_pv || !head_part_pv)
				continue;
			
			const auto lp_pos = roblox::games::game->get_local_player_position();

			std::uint32_t distance = -1;

			if (lp_pos.has_value())
			{
				distance = static_cast<std::uint32_t>(humanoid_root_part_pv->translation.distance(lp_pos.value()));

				if (distance > std::get<int>(menu::config::config_map["esp_distance"]))
					continue;
			}

			const auto draw_list = ImGui::GetBackgroundDrawList();

			offset = -4.0f;

			if (std::get<bool>(menu::config::config_map["enable_3d_square"]))
				draw_3D_square(draw_list, torso_opt);

			if (std::get<bool>(menu::config::config_map["enable_look_direction"]))
				draw_look_line(draw_list, head_part_opt);

			if (std::get<bool>(menu::config::config_map["enable_3d_cube"]))
				draw_3D_cube(draw_list, torso_opt);

			if (std::get<bool>(menu::config::config_map["enable_2d_but_good"]))
				draw_2D_box(draw_list, torso_opt);

			if (std::get<bool>(menu::config::config_map["enable_head_hitbox"])) 
				draw_head_marker(draw_list, head_part_opt);

			if (std::get<bool>(menu::config::config_map["enable_tracers"]))
				draw_tracers(draw_list, torso_opt);

			if (distance != -1 && std::get<bool>(menu::config::config_map["enable_esp_distance"]))
				draw_distance(draw_list, head_part_opt, distance);

			if (std::get<bool>(menu::config::config_map["enable_esp_name"]))
				draw_name(draw_list, child, torso_opt);

			if (std::get<bool>(menu::config::config_map["enable_tool_esp"]))
				draw_tool_name(draw_list, child, torso_opt);

			if (std::get<bool>(menu::config::config_map["enable_esp_health"]))
				draw_health(draw_list, child, torso_opt);

			draw_custom_box(draw_list, torso_opt);
		}
	}
}

void features::visuals::draw_3D_square(ImDrawList* draw_list, roblox::structs::part_instance_t* humanoid_root_part)
{
	if (humanoid_root_part->primitive)
	{
		const auto& part_cframe = humanoid_root_part->primitive->cframe;

		auto finished = part_cframe * math::types::cframe_t({ -1.5, 2.75, 0 });
		const auto screen_pos_1 = math::functions::world_to_screen(finished.position);

		finished = part_cframe * math::types::cframe_t({ 1.5, 2.75, 0 });
		const auto screen_pos_2 = math::functions::world_to_screen(finished.position);

		finished = part_cframe * math::types::cframe_t({ -1.5, -3.75, 0 });
		const auto screen_pos_3 = math::functions::world_to_screen(finished.position);

		finished = part_cframe * math::types::cframe_t({ 1.5, -3.75, 0 });
		const auto screen_pos_4 = math::functions::world_to_screen(finished.position);

		if (!screen_pos_1.has_value() || !screen_pos_2.has_value() || !screen_pos_3.has_value() || !screen_pos_4.has_value())
			return;

		ImColor col = (std::get<bool>(menu::config::config_map["enable_rainbow"])) ? ImColor::HSV(rainbow_count, 1, 1) : ImColor{ std::get< ImU32 >(menu::config::config_map["3d_square_color"]) };

		draw_list->AddQuad
		(
			ImVec2(screen_pos_1.value().x, screen_pos_1.value().y),
			ImVec2(screen_pos_2.value().x, screen_pos_2.value().y),
			ImVec2(screen_pos_4.value().x, screen_pos_4.value().y),
			ImVec2(screen_pos_3.value().x, screen_pos_3.value().y),
			col,
			2
		);
	}
}

void features::visuals::draw_look_line(ImDrawList* draw_list, roblox::structs::part_instance_t* head_part)
{
	if (head_part->primitive)
	{
		const auto& spot = head_part->primitive->cframe;
		const auto screen_spot = math::functions::world_to_screen(spot.position);

		auto new_spot = spot * math::types::cframe_t({ 0, 0, -2 });
		const auto screen_new_spot = math::functions::world_to_screen(new_spot.position);

		if (!screen_spot.has_value() || !screen_new_spot.has_value())
			return;

		ImColor col = (std::get<bool>(menu::config::config_map["enable_rainbow"])) ? ImColor::HSV(rainbow_count, 1, 1) : ImColor{ std::get< ImU32 >(menu::config::config_map["look_direction_color"]) };

		draw_list->AddLine(ImVec2(screen_spot.value().x, screen_spot.value().y), ImVec2(screen_new_spot.value().x, screen_new_spot.value().y), col, 2);
	}
}

void features::visuals::draw_3D_cube(ImDrawList* draw_list, roblox::structs::part_instance_t* humanoid_root_part)
{
	if (humanoid_root_part->primitive)
	{
		const auto& part_cframe = humanoid_root_part->primitive->cframe;

		auto finished = part_cframe * math::types::cframe_t({ -1.5, 2.75, -1.5 }); // RTOP RIGHT
		const auto screen_pos_1 = math::functions::world_to_screen(finished.position);

		finished = part_cframe * math::types::cframe_t({ -1.5, 2.75, 1.5 }); // RTOP LEFT
		const auto screen_pos_2 = math::functions::world_to_screen(finished.position);

		finished = part_cframe * math::types::cframe_t({ 1.5, 2.75, 1.5 });	// LTOP RIGHT
		const auto screen_pos_3 = math::functions::world_to_screen(finished.position);

		finished = part_cframe * math::types::cframe_t({ 1.5, 2.75, -1.5 }); // LTOP LEFT
		const auto screen_pos_4 = math::functions::world_to_screen(finished.position);

		finished = part_cframe * math::types::cframe_t({ -1.5, -3.75, -1.5 }); // RBOTTOM RIGHT
		const auto screen_pos_5 = math::functions::world_to_screen(finished.position);

		finished = part_cframe * math::types::cframe_t({ -1.5, -3.75, 1.5 }); // RBOTTOM LEFT
		const auto screen_pos_6 = math::functions::world_to_screen(finished.position);

		finished = part_cframe * math::types::cframe_t({ 1.5, -3.75, 1.5 }); // LBOTTOM RIGHT
		const auto screen_pos_7 = math::functions::world_to_screen(finished.position);

		finished = part_cframe * math::types::cframe_t({ 1.5, -3.75, -1.5 }); // LBOTTOM LEFT
		const auto screen_pos_8 = math::functions::world_to_screen(finished.position);

		if (!screen_pos_1.has_value() || !screen_pos_2.has_value() || !screen_pos_3.has_value() || !screen_pos_4.has_value() ||
			!screen_pos_5.has_value() || !screen_pos_6.has_value() || !screen_pos_7.has_value() || !screen_pos_8.has_value()
			)
			return;

		ImColor col = (std::get<bool>(menu::config::config_map["enable_rainbow"])) ? ImColor::HSV(rainbow_count, 1, 1) : ImColor{ std::get< ImU32 >(menu::config::config_map["3d_cube_color"]) };
		float thickness = 2;

		draw_list->AddLine(ImVec2(screen_pos_1.value().x, screen_pos_1.value().y), ImVec2(screen_pos_2.value().x, screen_pos_2.value().y), col, thickness);
		draw_list->AddLine(ImVec2(screen_pos_2.value().x, screen_pos_2.value().y), ImVec2(screen_pos_3.value().x, screen_pos_3.value().y), col, thickness);
		draw_list->AddLine(ImVec2(screen_pos_3.value().x, screen_pos_3.value().y), ImVec2(screen_pos_4.value().x, screen_pos_4.value().y), col, thickness);
		draw_list->AddLine(ImVec2(screen_pos_4.value().x, screen_pos_4.value().y), ImVec2(screen_pos_1.value().x, screen_pos_1.value().y), col, thickness);

		draw_list->AddLine(ImVec2(screen_pos_5.value().x, screen_pos_5.value().y), ImVec2(screen_pos_6.value().x, screen_pos_6.value().y), col, thickness); 
		draw_list->AddLine(ImVec2(screen_pos_6.value().x, screen_pos_6.value().y), ImVec2(screen_pos_7.value().x, screen_pos_7.value().y), col, thickness);
		draw_list->AddLine(ImVec2(screen_pos_7.value().x, screen_pos_7.value().y), ImVec2(screen_pos_8.value().x, screen_pos_8.value().y), col, thickness);
		draw_list->AddLine(ImVec2(screen_pos_8.value().x, screen_pos_8.value().y), ImVec2(screen_pos_5.value().x, screen_pos_5.value().y), col, thickness);

		draw_list->AddLine(ImVec2(screen_pos_1.value().x, screen_pos_1.value().y), ImVec2(screen_pos_2.value().x, screen_pos_2.value().y), col, thickness); 
		draw_list->AddLine(ImVec2(screen_pos_2.value().x, screen_pos_2.value().y), ImVec2(screen_pos_6.value().x, screen_pos_6.value().y), col, thickness);
		draw_list->AddLine(ImVec2(screen_pos_6.value().x, screen_pos_6.value().y), ImVec2(screen_pos_5.value().x, screen_pos_5.value().y), col, thickness);
		draw_list->AddLine(ImVec2(screen_pos_5.value().x, screen_pos_5.value().y), ImVec2(screen_pos_1.value().x, screen_pos_1.value().y), col, thickness);

		draw_list->AddLine(ImVec2(screen_pos_3.value().x, screen_pos_3.value().y), ImVec2(screen_pos_4.value().x, screen_pos_4.value().y), col, thickness); 
		draw_list->AddLine(ImVec2(screen_pos_4.value().x, screen_pos_4.value().y), ImVec2(screen_pos_8.value().x, screen_pos_8.value().y), col, thickness);
		draw_list->AddLine(ImVec2(screen_pos_8.value().x, screen_pos_8.value().y), ImVec2(screen_pos_7.value().x, screen_pos_7.value().y), col, thickness);
		draw_list->AddLine(ImVec2(screen_pos_7.value().x, screen_pos_7.value().y), ImVec2(screen_pos_3.value().x, screen_pos_3.value().y), col, thickness);
	}
}

void features::visuals::draw_2D_box(ImDrawList* draw_list, roblox::structs::part_instance_t* humanoid_root_part)
{
	roblox::structs::camera_t* current_camera = roblox::globals::data_model->workspace->current_camera;

	if (current_camera && humanoid_root_part->primitive)
	{
		auto fixed_cframe = math::types::cframe_t{ humanoid_root_part->primitive->cframe.position };
		fixed_cframe.look_at_locked(current_camera->cframe.position);

		auto finished = fixed_cframe * math::types::cframe_t({ -1.5, 2.75, 0 });
		const auto screen_pos_1 = math::functions::world_to_screen(finished.position);

		finished = fixed_cframe * math::types::cframe_t({ 1.5, 2.75, 0 });
		const auto screen_pos_2 = math::functions::world_to_screen(finished.position);

		finished = fixed_cframe * math::types::cframe_t({ -1.5, -3.75, 0 });
		const auto screen_pos_3 = math::functions::world_to_screen(finished.position);

		finished = fixed_cframe * math::types::cframe_t({ 1.5, -3.75, 0 });
		const auto screen_pos_4 = math::functions::world_to_screen(finished.position);

		if (!screen_pos_1.has_value() || !screen_pos_2.has_value() || !screen_pos_3.has_value() || !screen_pos_4.has_value())
			return;

		ImColor col = (std::get<bool>(menu::config::config_map["enable_rainbow"])) ? ImColor::HSV(rainbow_count, 1, 1) : ImColor{ std::get< ImU32 >(menu::config::config_map["2d_color"]) };

		draw_list->AddQuad
		(
			ImVec2(screen_pos_1.value().x, screen_pos_1.value().y),
			ImVec2(screen_pos_2.value().x, screen_pos_2.value().y),
			ImVec2(screen_pos_4.value().x, screen_pos_4.value().y),
			ImVec2(screen_pos_3.value().x, screen_pos_3.value().y),
			col,
			2
		);
	}
}

void features::visuals::draw_head_marker(ImDrawList* draw_list, roblox::structs::part_instance_t* head_part)
{
	if (head_part->primitive)
	{
		const auto& spot = head_part->primitive->cframe;
		const auto screen_spot = math::functions::world_to_screen(spot.position);

		if (!screen_spot.has_value())
			return;

		ImColor col = (std::get<bool>(menu::config::config_map["enable_rainbow"])) ? ImColor::HSV(rainbow_count, 1, 1) : ImColor{ 255, 255, 255 };

		draw_list->AddLine(ImVec2(screen_spot.value().x + 5.f, screen_spot.value().y), ImVec2(screen_spot.value().x - 5.f, screen_spot.value().y), col);
		draw_list->AddLine(ImVec2(screen_spot.value().x, screen_spot.value().y + 5.f), ImVec2(screen_spot.value().x, screen_spot.value().y - 5.f), col);
	}
}

void features::visuals::draw_tracers(ImDrawList* draw_list, roblox::structs::part_instance_t* humanoid_root_part)
{

	const auto& part_cframe = humanoid_root_part->primitive->cframe;

	auto finished = part_cframe * math::types::cframe_t({ 0, -3.75, 0 });
	const auto screen_pos_1 = math::functions::world_to_screen(finished.position);

	if (!screen_pos_1.has_value())
		return;

	ImColor col = (std::get<bool>(menu::config::config_map["enable_rainbow"])) ? ImColor::HSV(rainbow_count, 1, 1) : ImColor{ std::get< ImU32 >(menu::config::config_map["tracers_color"]) };

	const auto visual_engine = *reinterpret_cast<roblox::structs::visual_engine_t**>(roblox::offsets::visual_engine::visual_engine);

	const auto& [width, height] = visual_engine->screen_size;

	draw_list->AddLine({ width / 2.f, static_cast<float>(height) }, { screen_pos_1.value().x, screen_pos_1.value().y }, col);
}

void features::visuals::draw_distance(ImDrawList* draw_list, roblox::structs::part_instance_t* head_part, uint32_t distance)
{
	roblox::structs::camera_t* current_camera = roblox::globals::data_model->workspace->current_camera;
	if (current_camera && head_part->primitive)
	{
		const auto& part_cframe = head_part->primitive->cframe;

		const auto screen_pos_1 = math::functions::world_to_screen(part_cframe.position);

		std::string to_draw; to_draw.append(std::to_string(distance)); to_draw.append("M");

		if (screen_pos_1.has_value())
		{
			ImColor col = (std::get<bool>(menu::config::config_map["enable_rainbow"])) ? ImColor::HSV(rainbow_count, 1, 1) : ImColor{ 255, 255, 255 };

			const auto head_screen_position = screen_pos_1.value();
			{
				draw_list->AddLine({ head_screen_position.x, head_screen_position.y }, { head_screen_position.x - 5.f, head_screen_position.y - 10.f }, col);
			}

			{
				draw_list->AddLine({ head_screen_position.x - 5.f, head_screen_position.y - 10.f }, { head_screen_position.x - 20.f, head_screen_position.y - 10.f }, col);
			}

			draw_list->AddText(ImGui::GetDefaultFont(), 12.5f, { head_screen_position.x - 20.f, head_screen_position.y - 20.f }, col, to_draw.c_str());
		}
	}
}

void features::visuals::draw_name(ImDrawList* draw_list, roblox::structs::player_t* player, roblox::structs::part_instance_t* humanoid_root_part)
{
	const auto name = roblox::games::game->get_player_name(player);

	if (name.has_value() && humanoid_root_part->primitive)
	{
		const auto& part_cframe = humanoid_root_part->primitive->cframe;

		const auto finished = part_cframe * math::types::cframe_t({ 0, -3.75, 0 });
		const auto screen_pos_1 = math::functions::world_to_screen(finished.position);

		if (!screen_pos_1.has_value())
			return;

		draw_list->AddText(ImGui::GetDefaultFont(), 14.f, ImVec2(screen_pos_1.value().x - ui::calc_text_size(name.value(), 14.f).x / 2, screen_pos_1.value().y), ImColor{ 255, 255, 255 }, name.value());
	}
}

float scale_between(float unscaledNum, float minAllowed, float maxAllowed, float min, float max)
{
	return (maxAllowed - minAllowed) * (unscaledNum - min) / (max - min) + minAllowed;
}

void features::visuals::draw_custom_box(ImDrawList* draw_list, roblox::structs::part_instance_t* humanoid_root_part)
{
	roblox::structs::camera_t* current_camera = roblox::globals::data_model->workspace->current_camera;
	if (current_camera && humanoid_root_part->primitive)
	{
		auto fixed_cframe = math::types::cframe_t{ humanoid_root_part->primitive->cframe.position };
		fixed_cframe.look_at_locked(current_camera->cframe.position);

		for(const auto& [pos1,pos2] : ui::lines)
		{
			const auto scaledx1 = scale_between(pos1.x, -2.5, 2.5, -165.f, 165.f);
			const auto scaledx2 = scale_between(pos2.x, -2.5, 2.5, -165.f, 165.f);

			const auto scaledy1 = scale_between(pos1.y, -3.75, 4.75, -159.f, 159.f);
			const auto scaledy2 = scale_between(pos2.y, -3.75, 4.75, -159.f, 159.f);

			auto finished = fixed_cframe * math::types::cframe_t({ -scaledx1, -scaledy1, 0 });
			const auto screen_pos_1 = math::functions::world_to_screen(finished.position);

			if (!screen_pos_1.has_value())
				return;

			finished = fixed_cframe * math::types::cframe_t({ -scaledx2, -scaledy2, 0 });
			const auto screen_pos_2 = math::functions::world_to_screen(finished.position);

			if (!screen_pos_2.has_value())
				return;

			ImColor col = (std::get<bool>(menu::config::config_map["enable_rainbow"])) ? ImColor::HSV(rainbow_count, 1, 1) : ImColor{ std::get< ImU32 >(menu::config::config_map["custom_color"]) };

			draw_list->AddLine(ImVec2{screen_pos_1.value().x, screen_pos_1.value().y}, ImVec2{ screen_pos_2.value().x, screen_pos_2.value().y }, col, 2.f);
		}
	}
}

void features::visuals::draw_tool_name(ImDrawList* draw_list, roblox::structs::player_t* player, roblox::structs::part_instance_t* humanoid_root_part)
{
	if (humanoid_root_part->primitive)
	{
		const auto& part_cframe = humanoid_root_part->primitive->cframe;

		const auto name = roblox::games::game->get_player_tool_name(player);

		if (!name.has_value())
			return;

		const auto finished = part_cframe * math::types::cframe_t({ 0, -3.75, 0 });
		const auto screen_pos_1 = math::functions::world_to_screen(finished.position);

		if (!screen_pos_1.has_value())
			return;

		int count = 0;

		if (std::get<bool>(menu::config::config_map["enable_esp_name"]))
			count++;

		if (std::get<bool>(menu::config::config_map["enable_esp_health"]))
			count++;

		draw_list->AddText(ImGui::GetDefaultFont(), 14.f, ImVec2(screen_pos_1.value().x - ui::calc_text_size(name.value(), 14.f).x / 2, screen_pos_1.value().y + count * 10.f), ImColor{ 255, 255, 255 }, name.value());
	}
}

void features::visuals::draw_health(ImDrawList* draw_list, roblox::structs::player_t* player, roblox::structs::part_instance_t* humanoid_root_part)
{
	if (humanoid_root_part->primitive)
	{
		const auto& part_cframe = humanoid_root_part->primitive->cframe;

		const auto finished = part_cframe * math::types::cframe_t({ 0, -3.75, 0 });
		const auto screen_pos_1 = math::functions::world_to_screen(finished.position);

		if (!screen_pos_1.has_value())
			return;

		const auto health = roblox::games::game->get_health(player);

		if (!health.has_value())
			return;

		const auto maxhealth = roblox::games::game->get_max_health(player);

		if (!maxhealth.has_value())
			return;

		const auto health_str = std::to_string(static_cast<int>(std::floor(health.value()))) + " / " + std::to_string(static_cast<int>(std::floor(maxhealth.value())));

		draw_list->AddText(ImGui::GetDefaultFont(), 14.f, ImVec2(screen_pos_1.value().x - ui::calc_text_size(health_str.c_str(), 14.f).x / 2, std::get<bool>(menu::config::config_map["enable_esp_name"]) ? screen_pos_1.value().y + 10.f : screen_pos_1.value().y), ImColor{ 255, 255, 255 }, health_str.c_str());
	}
}