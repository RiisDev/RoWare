#include "math.hpp"

#include "../roblox/roblox.hpp"

std::optional<math::types::vector2_t> math::functions::world_to_screen(const math::types::vector_t& in)
{
	const auto visual_engine = *reinterpret_cast<roblox::structs::visual_engine_t**>(roblox::offsets::visual_engine::visual_engine);

	const auto matrix = visual_engine->view_matrix;

	const auto& [width, height] = visual_engine->screen_size;

	math::types::vector4_t comp = matrix * in;

	if ( comp.w < 0.001f)
		return std::nullopt;

	comp.x /= comp.w;
	comp.y /= comp.w;

	comp.x = ( width / 2.0f ) + (comp.x * width ) / 2.0f;
	comp.y = ( height / 2.0f ) - (comp.y * height ) / 2.0f;

	math::types::vector2_t ret;
	ret.x = comp.x;
	ret.y = comp.y;

	return std::optional<math::types::vector2_t>(ret);
}