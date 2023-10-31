#include "games.hpp"

#include "../games/bad_business/bad_business.hpp"
#include "../games/rush_point/rush_point.hpp"
#include "../games/arsenal/arsenal.hpp"
#include "../games/front_lines/front_lines.hpp"
#include "../games/phantom_forces/phantom_forces.hpp"

std::shared_ptr<roblox::games::base_game_t> roblox::games::game = nullptr;

std::unordered_map<roblox::games::game_t, std::shared_ptr<roblox::games::base_game_t>> game_map =
{
	{roblox::games::game_t::phantom_forces, std::make_shared<roblox::games::phantom_forces_t>()},
	{roblox::games::game_t::arsenal, std::make_shared<roblox::games::arsenal_t>()},
	{roblox::games::game_t::bad_business, std::make_shared<roblox::games::bad_business_t>()},
	{roblox::games::game_t::rush_point, std::make_shared<roblox::games::rush_point_t>()},
	{roblox::games::game_t::front_lines, std::make_shared<roblox::games::front_lines_t>()}
};

std::unordered_map<const char*, bool> roblox::games::enabled_map
{
	{"enable_esp_name", true},
	{"allow_memory_aimbot", true},
	{"allow_esp_health", true},
	{"allow_aimbot_health", true},
	{"allow_esp_tools", true},
	{"allow_triggerbot", true},
	{"allow_wallcheck", true}
};

void roblox::games::init_game()
{
	if (game_map.contains(roblox::globals::place_id))
		roblox::games::game = game_map[roblox::globals::place_id];
	else
		roblox::games::game = std::make_shared<roblox::games::base_game_t>();
}