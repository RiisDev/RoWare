#include "config.hpp"

#include <filesystem>
#include <Windows.h>
#include <fstream>

#include "../../../dependencies/json/json.hpp"

#include "../ui.hpp"
#include "../../../task_scheduler/task_scheduler.hpp"

std::vector<std::string>  menu::config::configs = { "default" };

std::unordered_map< const char*, std::variant< bool, int, float, ImU32 > > menu::config::config_map
{
	{"enable_aimbot", false},
	{"aimbot_type", 0},
	{"enable_smoothing", false},
	{"smoothing_amount", 1},
	{"enable_fov_check", false},
	{"fov_amount", 0},
	{"target_bone", 0},
	{"aim_hotkey", VK_RBUTTON},
	{"open_menu", VK_INSERT},

	{"enable_rcs", false},
	{"rcs_pitch", 0},
	{"rcs_yaw", 0},

	{"enable_trigger_bot", false},
	{"enable_team_check", false},

	{"enable_head_hitbox", false},
	{"enable_esp_distance", false},
	{"enable_tracers", false},

	{"esp_distance", 750},

	{"modified_speed", 16},
	{"enable_forced_speed", false},

	{"enable_3d_square", false},
	{"enable_3d_cube", false},
	{"enable_2d_but_good", false},
	{"enable_look_direction", false},
	{"enable_custom_esp", false},

	{"enable_rainbow", false},
	{"enable_wall_check", false},
	{"enable_esp_name", false},
	{"enable_trigger_bot", false},

	{"enable_chams", false},
	{"chams_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},

	{"custom_line_count", 0},
	{"waypoint_count", 0},


	{"max_distance_aim", 0},
	{"max_distance_aim_enabled", false},
	{"enable_tool_esp", false},
	{"enemy_only_esp", false},
	{"anti_idle_kick", false},
	{"enable_aim_health_check", false},
	{"enable_esp_health", false},
	{"enable_info", false},

	{"3d_square_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},
	{"3d_cube_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},
	{"2d_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f} },
	{"look_direction_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},
	{"custom_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},
	{"tracers_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},

	{"obs_hide", false},
	{"fps_limit", 60}
};

std::unordered_map< const char*, std::variant< bool, int, float, ImU32 > > menu::config::default_config = {
	{"enable_aimbot", false},
	{"aimbot_type", 0},
	{"enable_smoothing", false},
	{"smoothing_amount", 1},
	{"enable_fov_check", false},
	{"draw_fov", false},
	{"fov_amount", 0},
	{"target_bone", 0},
	{"aim_hotkey", VK_RBUTTON},
	{"open_menu", VK_INSERT},

	{"enable_rcs", false},
	{"rcs_pitch", 0},
	{"rcs_yaw", 0},

	{"enable_trigger_bot", false},
	{"enable_team_check", false},

	{"enable_head_hitbox", false},
	{"enable_esp_distance", false},
	{"enable_tracers", false},

	{"esp_distance", 750},

	{"modified_speed", 16},
	{"enable_forced_speed", false},

	{"enable_3d_square", false},
	{"enable_3d_cube", false},
	{"enable_2d_but_good", false},
	{"enable_look_direction", false},
	{"enable_custom_esp", false},

	{"enable_rainbow", false},
	{"enable_wall_check", false},
	{"enable_esp_name", false},
	{"enable_trigger_bot", false},

	{"enable_chams", false},
	{"chams_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},

	{"custom_line_count", 0},
	{"waypoint_count", 0},

	{"max_distance_aim", 0},
	{"max_distance_aim_enabled", false},
	{"enable_tool_esp", false},
	{"enemy_only_esp", false},
	{"anti_idle_kick", false},
	{"enable_aim_health_check", false},
	{"enable_esp_health", false},
	{"enable_info", false},

	{"3d_square_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},
	{"3d_cube_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},
	{"2d_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f} },
	{"look_direction_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},
	{"custom_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},
	{"tracers_color", ImColor{1.0f, 1.0f, 1.0f, 1.0f}},

	{"obs_hide", false},
	{"fps_limit", 60}
};

void menu::config::dump_to_cfg(const char* const name) //todo: switch to chilkat cuz this lib fucking sucks and throws 9999999 exceptions
{

}

void menu::config::delete_cfg(const char* const name)
{

}

void menu::config::load_cfg(const char* const name)
{

}

void menu::config::refresh_configs()
{

}