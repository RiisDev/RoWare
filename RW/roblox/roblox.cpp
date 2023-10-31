#include "roblox.hpp"

#include "../renderer/renderer.hpp"
#include "../features/visuals/visuals.hpp"
#include "../renderer/menu/config/config.hpp"

#include "../games/games.hpp"

roblox::structs::data_model_t* roblox::globals::data_model;

roblox::structs::players_t* roblox::globals::players;

roblox::structs::instance_t* roblox::globals::mouse_service;

roblox::games::game_t roblox::globals::place_id;

math::types::matrix4_t roblox::globals::view_matrix;

bool roblox::init(std::uintptr_t job)
{
	roblox::globals::place_id = *reinterpret_cast<games::game_t*>(roblox::offsets::data_model::place_id);
	const auto script_context_reference = *reinterpret_cast<roblox::structs::reference_t**>(job + roblox::offsets::waiting_script_job::task_scheduler::script_context);

	roblox::globals::data_model = static_cast<roblox::structs::data_model_t*>(script_context_reference->instance->parent);

	std::printf("[Init] DataModel: %p\n", roblox::globals::data_model);

	std::printf("[Init] WorkSpace: %p\n", roblox::globals::data_model->workspace);

	while (!roblox::globals::data_model->is_loaded)
		return false;

	std::printf("[Init] data_model->is_loaded = true\n");

	roblox::games::init_game();

	roblox::games::game->toggle_features();

	roblox::globals::players = roblox::globals::data_model->find_first_class<roblox::structs::players_t>("Players");

	roblox::globals::mouse_service = roblox::globals::data_model->find_first_class("MouseService");

	if (!roblox::globals::players || !roblox::functions::is_valid_ptr(roblox::globals::players))
		return false;

	std::printf("[Init] Initialized\n\n");

	static std::once_flag rbx_init;

	std::call_once(rbx_init, renderer::initialize);

	return true;
}

bool roblox::functions::is_valid_ptr(const void* const address)
{
	MEMORY_BASIC_INFORMATION mbi{};

	if (VirtualQuery(address, &mbi, sizeof(MEMORY_BASIC_INFORMATION)))
		return !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) && (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY));

	return false;
}

float roblox::structs::humanoid_t::get_health()
{
	using health_get_function = float(__thiscall*)(roblox::structs::humanoid_t*);

	const auto health_property = *reinterpret_cast<roblox::structs::property_t**>(roblox::offsets::humanoid::properties::health);

	return static_cast<health_get_function>(health_property->get_function)(this);
}

float roblox::structs::humanoid_t::get_max_health()
{
	using maxhealth_get_function = float(__thiscall*)(roblox::structs::humanoid_t*);

	const auto health_property = *reinterpret_cast<roblox::structs::property_t**>(roblox::offsets::humanoid::properties::max_health);

	return static_cast<maxhealth_get_function>(health_property->get_function)(this);
}

bool roblox::structs::player_t::aim_at(bool is_mouse, float smooth_amount, bool head) const //ADD BACK TEAMCHEKC
{
	const auto parts = roblox::games::game->get_part(this);

	if (parts.has_value()) 
	{
		const auto& [head_part, torso] = parts.value();

		const auto head_root_part_pv = roblox::functions::pv::get_pv_from_part_instance(head ? head_part : torso);

		if (!head_root_part_pv || !roblox::functions::is_valid_ptr(head_root_part_pv))
			return true;

		const auto head_position = math::functions::world_to_screen(head_root_part_pv->translation);

		if (!head_position.has_value())
			return true;

		const auto& head_screen_position = head_position.value();

		if (!torso)
			return true;

		if (std::get<bool>(menu::config::config_map["enable_wall_check"]))
			if (roblox::games::game->is_behind_wall(this, parts.value()))
				return true;

		if (std::get<bool>(menu::config::config_map["enable_team_check"]))
			if (roblox::games::game->is_same_team(this))
				return true;

		if (std::get<bool>(menu::config::config_map["max_distance_aim_enabled"]))
			if (roblox::games::game->check_distance(parts.value()))
				return true;

		if (std::get<bool>(menu::config::config_map["enable_aim_health_check"]))
		{
			const auto health_optional = roblox::games::game->get_health(this);

			if (health_optional.has_value())
			{
				const auto health = health_optional.value();

				if (health <= 0)
					return true;
			}
		}

		POINT p;

		if (!is_mouse)
		{
			const auto workspace = roblox::globals::data_model->find_first_class<roblox::structs::workspace_t>("Workspace");

			const auto camera = workspace->current_camera;

			auto current = camera->cframe;
			auto pos = camera->cframe;

			roblox::functions::cframe::look_at_helper(&pos, head_root_part_pv->translation);

			math::types::cframe_t ret;

			const auto smoothing = std::get< bool >(menu::config::config_map["enable_smoothing"]) ? 1.0f - (static_cast<float>(std::get< int >(menu::config::config_map["smoothing_amount"])) / 100.0f) : 1.0f;

			roblox::functions::cframe::lerp_helper(&current, &camera->cframe, pos, smoothing);

			return false;
		}
		else
		{
			if (renderer::is_roblox_on_top() && GetCursorPos(&p) && !ui::Static::render_interface)
			{
				if (ScreenToClient(renderer::values::roblox_window, &p))
				{
					// Fix for dpi scaling
					p.x /= renderer::values::dpi_scale;
					p.y /= renderer::values::dpi_scale;

					const math::types::vector2_t mouse_loc{ static_cast<float>(p.x), static_cast<float>(p.y) };

					const auto distance = mouse_loc.distance(head_screen_position);

					if (std::get< bool >(menu::config::config_map["enable_fov_check"]))
						if (std::get<int>(menu::config::config_map["fov_amount"]) * 50.f < distance)
							return true;

					INPUT i{};
					i.type = INPUT_MOUSE;
					i.mi.dwFlags = MOUSEEVENTF_MOVE;

					i.mi.dx = (head_screen_position.x - p.x);
					i.mi.dy = (head_screen_position.y - p.y);

					const auto smooth = [&](float amt)
					{
						amt = amt >= 3.f ? amt : 3.f;

						const auto old = amt;

						auto x = i.mi.dx; if (x < 0) { x = ~x; }

						auto y = i.mi.dy; if (y < 0) { y = ~y; }

						while (x <= amt && x != 0 && amt > 10)
							amt /= 2;

						i.mi.dx /= amt;

						amt = old;

						while (y <= amt && y != 0 && amt > 10)
							amt /= 2;

						i.mi.dy /= amt;
					};

					smooth(std::get< bool >(menu::config::config_map["enable_smoothing"]) ? smooth_amount : 5.f);
					
					SendInput(1, &i, sizeof(INPUT));

					return false;
				}
			}
		}
	}

	return true;

}

double roblox::functions::time::get_current_idle_time()
{
	if (!roblox::games::game->get_local_player())
		return 0.0;

	return get_current_time() - roblox::games::game->get_local_player()->idle_time;
}

double roblox::functions::time::get_biggest_idle_time()
{
	if (!roblox::games::game->get_local_player())
		return max_idle_time;

	const auto current_time = get_current_time();
	const auto idle_time = roblox::games::game->get_local_player()->idle_time;

	if (current_time - idle_time > max_idle_time)
		max_idle_time = current_time - idle_time;

	return max_idle_time;
}

bool roblox::structs::part_instance_t::is_instance_part_instance()
{
	if (!this || this->class_descriptor->id != 0x75 && this->class_descriptor->id != 0x6A)
		return false;

	return true;
}