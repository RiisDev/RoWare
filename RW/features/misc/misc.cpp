#include "misc.hpp"

#include "../../roblox/roblox.hpp"
#include "../../renderer/renderer.hpp"
#include "../../games/games.hpp"

bool shoot_flag = false;

void features::misc::callback()
{
	if (!roblox::globals::data_model || !roblox::globals::data_model->is_loaded)
		return;

	features::misc::trigger_bot();

	features::misc::reset_delay_timer(roblox::games::game->get_local_player());
}

void features::misc::reset_delay_timer(roblox::structs::player_t* local_player)
{
	if (local_player && std::get<bool>(menu::config::config_map["anti_idle_kick"]))
		local_player->idle_time = *reinterpret_cast<double*>(roblox::offsets::time::current_time);
}

void features::misc::trigger_bot()
{
	INPUT input{};

	if (shoot_flag)
	{
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_LEFTUP;

		SendInput(1, &input, sizeof(INPUT));

		shoot_flag = false;
	}

	if (renderer::is_roblox_on_top() && !ui::Static::render_interface && std::get<bool>(menu::config::config_map["enable_trigger_bot"]))
	{
		const auto triggered = roblox::games::game->is_mouse_over_player();

		if (triggered)
		{
			input.type = INPUT_MOUSE;
			input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

			SendInput(1, &input, sizeof(INPUT));

			shoot_flag = true;
		}
	}
}