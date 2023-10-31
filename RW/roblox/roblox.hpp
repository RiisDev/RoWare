#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "../math/math.hpp"

const auto base = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));

namespace roblox
{
	namespace games { enum class game_t : std::uint64_t; }

	bool init(std::uintptr_t job);
}

namespace roblox::functions
{
	bool is_valid_ptr(const void* const address);
}

namespace roblox::structs
{
	using virtual_function_table_t = void*;
	struct instance_t;
	struct part_instance_t;

	enum class rig_type_t : std::uint16_t
	{
		R6,
		R15
	};
	
	struct class_descriptor_t
	{
		virtual_function_table_t virtual_function_table;

		std::string& name;

		std::uint8_t pad_1[40];

		std::uint32_t id;
	};

	struct children_array_t
	{
		std::uintptr_t instances;

		std::uintptr_t last_element_in_array;
	};

	struct reference_t
	{
		virtual_function_table_t virtual_function_table;

		std::uint8_t pad_0[8];

		instance_t* instance;
	};

	struct instance_t
	{
		virtual_function_table_t vtable;

		std::uintptr_t self;

		reference_t* reference;

		class_descriptor_t* class_descriptor;

		std::uint8_t pad_1[20];

		std::string& name;

		children_array_t* children;

		std::uint8_t pad_2[4];

		instance_t* parent;

		template<class type_t = instance_t>
		type_t* find_first_child(const std::string& child_name) const
		{
			if (!this)
				return nullptr;

			const auto children_array = this->children;

			if (!children_array)
				return nullptr;

			const auto last_element = children_array->last_element_in_array;

			if (!last_element)
				return nullptr;

			for (auto parent = children_array->instances; parent != last_element && parent; parent += sizeof(roblox::structs::children_array_t))
			{
				const auto current_instance = *reinterpret_cast<roblox::structs::instance_t**>(parent);

				if (!current_instance)
					continue;

				if (current_instance->name == child_name)
					return static_cast<type_t*>(current_instance);
			}

			return nullptr;
		}

		template<class type_t = instance_t>
		type_t* find_first_class(const std::string& class_name) const
		{
			if (!this)
				return nullptr;

			const auto children_array = this->children;

			if (!children_array)
				return nullptr;

			const auto last_element = children_array->last_element_in_array;

			if (!last_element)
				return nullptr;

			for (auto parent = children_array->instances; parent != last_element && parent; parent += sizeof(roblox::structs::children_array_t))
			{
				const auto current_instance = *reinterpret_cast<roblox::structs::instance_t**>(parent);

				if (!current_instance)
					continue;

				if (current_instance->class_descriptor->name == class_name)
					return static_cast<type_t*>(current_instance);
			}

			return nullptr;
		}

		template<class type_t = instance_t>
		std::vector<type_t*> get_children() const
		{
			std::vector<type_t*> children;

			if (!this)
				return children;

			const auto children_array = this->children;

			if (!children_array)
				return children;

			const auto last_element = children_array->last_element_in_array;

			for (auto parent = children_array->instances; parent != last_element && parent; parent += sizeof(roblox::structs::children_array_t))
			{
				if (!parent)
					return children;

				if (const auto current_instance = *reinterpret_cast<type_t**>(parent))
					children.push_back(current_instance);
			}

			return children;
		}
	};

	struct primitive_t
	{
		std::uint8_t pad_0[216];

		math::types::cframe_t cframe;

		std::uint8_t pad_1[216];

		math::types::vector_t size; //0x1e8
	};

	struct part_instance_t : instance_t
	{
		std::uint8_t pad_0[200 - sizeof(instance_t)];

		primitive_t* primitive;

		bool is_instance_part_instance();
	};

	struct player_t : instance_t //dont forget that those inherit instance_t while finding offsets
	{
		std::uint8_t pad_0[124 - sizeof(instance_t)];

		instance_t* character;

		std::uint8_t pad_1[48];

		std::uintptr_t team;

		std::uint8_t pad_2[1228];

		double idle_time;

		std::uint8_t pad_3[68];

		instance_t* mouse;

		bool aim_at(bool is_memory, float smooth_amount, bool head) const;
	};

	struct players_t : instance_t
	{
		std::uint8_t pad_0[304 - sizeof(instance_t)];

		std::uint32_t max_players;

		std::uint8_t pad_1[12];

		player_t* local_player;
	};

	struct camera_t
	{
		std::uint8_t pad_0[0xB8];

		math::types::cframe_t cframe;
	};

	struct humanoid_t : instance_t
	{
		std::uint8_t pad_0[524];

		rig_type_t rig_type;

		float get_health();

		float get_max_health();
	};

	struct workspace_t : instance_t
	{
		std::uint8_t pad_0[824 - sizeof(instance_t)];

		camera_t* current_camera;
	};

	struct data_model_t : instance_t
	{
		std::uint8_t pad_0[420 - sizeof(instance_t)];

		workspace_t* workspace;

		std::uint8_t pad_1[136];

		std::uint8_t is_loaded;
	};

	struct pv_t
	{
		math::types::matrix3_t rotation;

		math::types::vector_t translation;
	};

	struct visual_engine_t
	{
		std::uint8_t pad_0[368];

		math::types::matrix4_t view_matrix;

		std::uint8_t pad_1[1356];

		std::pair<float, float> screen_size;
	};

	struct property_t
	{
		virtual_function_table_t vtable;

		std::uint8_t pad_0[4];

		void* get_function;

		std::uint8_t pad_1[12];

		void* set_function;
	};

	struct number_value : instance_t
	{
		std::uint8_t pad_0[0x88 - sizeof(instance_t)];

		double xored_value;

		double read_value(std::uintptr_t xor_offset) { return xored_value; }
	};

	struct billboard_t : instance_t
	{
		std::uint8_t pad_0[0x354 - sizeof(instance_t)];

		part_instance_t* adornee;
	};

	struct job_t
	{
		virtual_function_table_t functions;

		std::uint8_t pad_0[12];

		std::string name;

		std::uint8_t pad_1[16];

		double time;

		std::uint8_t pad_2[16];

		double time_spend;

		std::uint8_t pad_3[8];

		std::uintptr_t state;
	};
}

namespace roblox::functions::task_scheduler
{
	inline std::uintptr_t get_task_scheduler() { return *reinterpret_cast<std::uintptr_t*>(base + 0x38250E4); } //function: 0x11E90B0

	using data_model_job_t = void(__thiscall*)(std::uintptr_t* job, const char* name, std::uint32_t a0, std::uint32_t a1, std::uint32_t a2, std::uintptr_t a3, std::uintptr_t a4);
	const auto data_model_job = reinterpret_cast<data_model_job_t>(base + 0xF50B20); //LuaGc 

	using add_job_t = void(*)(std::uintptr_t* job, std::uint32_t unk);
	const auto add_job = reinterpret_cast<add_job_t>(base + 0x9D1C90); //Write Marshalled

	inline std::uintptr_t get_job(const char* job_name, bool second = false)
	{
		int job_c = 0;

		const auto task_scheduler = get_task_scheduler();
		
		if (!task_scheduler)
			return 0;

		const auto job_start = *reinterpret_cast<std::uintptr_t*>(task_scheduler + 0x134);

		const auto last_element = *reinterpret_cast<std::uintptr_t*>(task_scheduler + 0x138);

		for (auto job = job_start; job != last_element; job += 0x8)
		{
			const auto current_instance = *reinterpret_cast<roblox::structs::job_t**>(job);

			if (!current_instance)
				continue;

			if (current_instance->name == job_name)
			{
				if (++job_c != 2 && second)
					continue;

				return reinterpret_cast<std::uintptr_t>(current_instance);
			}
		}

		return 0;
	}
}

namespace roblox::functions::cframe
{
	using look_at_t = void(__thiscall*)(math::types::cframe_t* cframe, math::types::vector_t* vec, float v1, float v2, float v3);
	const auto look_at = reinterpret_cast<look_at_t>(base + 0xC320D0); //seat weld 3rd xref right above RootRigAttachment last xref of called func

	using lerp_t = void(__thiscall*)(math::types::cframe_t* cframe, math::types::cframe_t* ret, math::types::cframe_t* vec, float smooth);
	const auto lerp = reinterpret_cast<lerp_t>(base + 0xC31F30); //55 8B EC 83 E4 F0 F3 0F 10 45 10

	inline void look_at_helper(math::types::cframe_t* cframe, math::types::vector_t vec)
	{
		look_at(cframe, &vec, 0, 1, 0);
	}

	inline void lerp_helper(math::types::cframe_t* cframe, math::types::cframe_t* ret, math::types::cframe_t& vec, float smooth)
	{
		lerp(cframe, ret, &vec, smooth);
	}
}

namespace roblox::functions::mouse
{
	using get_hit_t = roblox::structs::instance_t* (__thiscall*)(roblox::structs::instance_t* mouse);
	const auto get_hit = reinterpret_cast<get_hit_t>(base + 0x100A720); //55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 83 EC 20 53 8B D9 56 57 89 5D F0 8B 03
}

namespace roblox::functions::raycast
{
	using fire_raycast_t = void(__thiscall*)(std::uintptr_t workspace, std::uintptr_t* ret, math::types::vector_t* start, math::types::vector_t* direction, std::uintptr_t filter);
	const auto fire_raycast = reinterpret_cast<fire_raycast_t>(base + 0xFCC310); //RaycastParam's CollisionGroup '%s' does not exist second xref RBX::Reflection::BoundFuncDesc<RBX::WorldRoot,RBX::LuaRaycastResult __cdecl (RBX::Vector3 const &,RBX::Vector3 const &,RBX::LuaRaycastParams const &),3>::`vftable';
	//55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 64 89 25 ? ? ? ? 81 EC ? ? ? ? 8B 55 14 
	inline roblox::structs::instance_t* fire_raycast_helper(roblox::structs::instance_t* workspace, math::types::vector_t origin, math::types::vector_t direction, roblox::structs::instance_t* ignore_instance)
	{
		//FilterDescendantsInstances is not safe to modify in parallel func after if
		const auto filter_descendant_list = base + 0x2350DB0; //const RBX::FilterDescendantsList::`vftable'
		const auto rbx_filter_ref = base + 0x2252220; //std::_Ref_count_obj2<class RBX::FilterDescendantsList>

		std::uintptr_t raycast_params[9]{};

		std::uintptr_t blacklist_space[10]{};
		std::uintptr_t instance_ptr[3]{};

		instance_ptr[0] = ignore_instance->self;

		blacklist_space[0] = rbx_filter_ref;
		blacklist_space[1] = 2;
		blacklist_space[2] = 1;
		blacklist_space[3] = filter_descendant_list;
		blacklist_space[4] = reinterpret_cast<std::uintptr_t>(instance_ptr);
		blacklist_space[5] = reinterpret_cast<std::uintptr_t>(instance_ptr) + 0x8;
		blacklist_space[6] = reinterpret_cast<std::uintptr_t>(instance_ptr) + 0x8;

		raycast_params[0] = reinterpret_cast<std::uintptr_t>(blacklist_space) + 0xC;
		raycast_params[1] = reinterpret_cast<std::uintptr_t>(blacklist_space);
		raycast_params[2] = 0x61666544;
		raycast_params[3] = 0x746C75;
		raycast_params[4] = 0x14;
		raycast_params[5] = 0x3F7DBC0A;
		raycast_params[6] = 0x7;
		raycast_params[7] = 0xF;
		raycast_params[8] = 0x0;

		std::uintptr_t ret[0x30];

		roblox::functions::raycast::fire_raycast(workspace->self, ret, &origin, &direction, reinterpret_cast<std::uintptr_t>(raycast_params));

		if (!ret)
			return nullptr;

		return reinterpret_cast<roblox::structs::instance_t*>(ret[0xA]);
	}
}

namespace roblox::functions::colors
{
	using get_colors_instance_t = std::uintptr_t(__thiscall*)();
	const auto get_colors_instance = reinterpret_cast<get_colors_instance_t>(base + 0x76D8C0); //BrickColor 4th xref

	inline math::types::vector_t get_color_by_id(std::uint32_t id)
	{
		const auto colors = *reinterpret_cast<std::uintptr_t*>(get_colors_instance() + 0x24);

		const auto color_instance = colors + 0x30 * id;

		const auto color = *reinterpret_cast<math::types::vector_t*>(color_instance + 0x8);

		return { color.x * 255, color.y * 255, color.z * 255 };
	}
}

namespace roblox::functions::pv
{
	using get_pv_pos_t = std::uintptr_t(__thiscall*)(roblox::structs::part_instance_t* part_instance);
	const auto get_pv_pos = reinterpret_cast<get_pv_pos_t>(base + 0xF6AF40); //RotVelocity   56 8B B1 ? ? ? ? 8D 46 30 F7 DE 1B F6 23 F0 8B CE E8 ? ? ? ? 8D 86 ? ? ? ? 5E C3 CC 55 8B EC 8B 89 ? ? ? ? 

	inline roblox::structs::pv_t* get_pv_from_part_instance(roblox::structs::part_instance_t* part_instance)
	{
		const auto pos = get_pv_pos(part_instance);

		if (!pos)
			return nullptr;

		return reinterpret_cast<roblox::structs::pv_t*>(pos - 0x3C);
	}
}

namespace roblox::offsets::waiting_script_job::task_scheduler
{
	constexpr std::uint16_t script_context = 0x134;
}

namespace roblox::offsets::number_xor
{
	const std::uintptr_t xor_offset = base + 0x3720800 + 0xC; //the metatable is locked   // 66 0F 6E 4D ? 0F 28 15 ? ? ? ?
}

namespace roblox::offsets::data_model
{
	const std::uintptr_t place_id = base + 0x383B120; //PlaceId=%llu\n
}

namespace roblox::offsets::visual_engine
{
	const std::uintptr_t visual_engine = base + 0x39C04E4; //var PlatformInfo = '%s';\n
}

namespace roblox::offsets::time
{
	const std::uintptr_t current_time = base + 0x3ADD108; //Failed timeGetDevCaps
}

namespace roblox::offsets::fflags
{
	const std::uintptr_t is_beta = base + 0x35E30E0;

	inline bool is_in_beta() { return *reinterpret_cast<bool*>(is_beta); }
}

namespace roblox::offsets::humanoid
{
	constexpr std::uint16_t max_health = 0x21C;

	constexpr std::uint16_t health = 0x218;

	constexpr std::uint16_t rig_type = 0x244;

	namespace properties
	{
		const std::uintptr_t walkspeed = base + 0x3A389D4 - 0x400000;

		const std::uintptr_t health = base + 0x380E69C;

		const std::uintptr_t max_health = base + 0x380E704;
	};
}

namespace roblox::globals
{
	extern games::game_t place_id;

	extern roblox::structs::data_model_t* data_model;

	extern roblox::structs::players_t* players;

	extern math::types::matrix4_t view_matrix;

	extern roblox::structs::instance_t* mouse_service;
}

namespace roblox::functions::time
{
	static double max_idle_time = 0.0;

	inline double get_current_time() { return *reinterpret_cast<double*>(roblox::offsets::time::current_time); }

	double get_current_idle_time();

	double get_biggest_idle_time();
}