#include "arsenal.hpp"

#include "../../roblox/roblox.hpp"

std::optional<float> roblox::games::arsenal_t::get_health(const roblox::structs::player_t* player) const
{
	const auto stats = player->find_first_child("NRPBS");

	if (!stats || !roblox::functions::is_valid_ptr(stats))
		return std::nullopt;

	const auto health = stats->find_first_child<roblox::structs::number_value>("Health");

	if (!health || !roblox::functions::is_valid_ptr(health))
		return std::nullopt;

	return health->read_value(*reinterpret_cast<std::uint32_t*>(roblox::offsets::number_xor::xor_offset));
}

std::optional<float> roblox::games::arsenal_t::get_max_health(const roblox::structs::player_t* player) const
{
	const auto stats = player->find_first_child("NRPBS");

	if (!stats || !roblox::functions::is_valid_ptr(stats))
		return std::nullopt;

	const auto health = stats->find_first_child<roblox::structs::number_value>("MaxHealth");

	if (!health || !roblox::functions::is_valid_ptr(health))
		return std::nullopt;

	return health->read_value(*reinterpret_cast<std::uint32_t*>(roblox::offsets::number_xor::xor_offset));
}