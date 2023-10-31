#include "task_scheduler.hpp"
#include "../features/aimbot/aimbot.hpp"

#include <mutex>
#include <functional>
#include "../features/misc/misc.hpp"

using job_callback_t = std::uint32_t(__thiscall*)(std::uintptr_t job, std::uintptr_t stats);
job_callback_t waiting_script_job_original;

using job_callback_destroy_t = std::uint32_t(__thiscall*)(std::uintptr_t job, std::uintptr_t arg);
job_callback_destroy_t waiting_script_job_destroy_original;

std::uint32_t state = 2;

int __fastcall stealth_thread(const std::uintptr_t job, std::uintptr_t, const std::uintptr_t arg)
{
	if (state == 1 && task_scheduler::init())
		state = 2;

	return 1;
}

std::uint32_t __fastcall waiting_script_job_destroy_hook(const std::uintptr_t job, std::uintptr_t, const std::uintptr_t arg)
{
	std::printf("[TH] Teleport\n");

	roblox::globals::players = 0;

	roblox::globals::mouse_service = 0;

	const auto ret = waiting_script_job_destroy_original(job, arg);

	state = 1;

	return ret;
}

void append_job()
{
	std::uintptr_t* space = new std::uintptr_t[0x160];

	roblox::functions::task_scheduler::data_model_job(space, "Ro-Ware Teleport Handler", 1, 0, 5, 0, 0);

	auto virtual_function_table = new void* [6];

	std::memcpy(virtual_function_table, reinterpret_cast<void*>(space[0]), 6 * sizeof(void*));

	space[0] = reinterpret_cast<std::uintptr_t>(virtual_function_table);

	virtual_function_table[2] = stealth_thread;

	roblox::functions::task_scheduler::add_job(space, 0);
}

int to_lock = -1;

std::uint32_t __fastcall waiting_script_job_hook(const std::uintptr_t job, std::uintptr_t, const std::uintptr_t stats)
{
	static std::once_flag rbx_init;

	std::call_once(rbx_init, append_job);

	if (state == 2 && roblox::init(job))
		state = 0;

	modules::aimbot::callback();
	features::misc::callback();

	//if (ui::Static::render_interface && roblox::globals::mouse_service)
	//{
	//	if (to_lock == -1)
	//		to_lock = *reinterpret_cast<std::uint32_t*>(roblox::globals::mouse_service->self + 0xCC);

	//	*reinterpret_cast<std::uint32_t*>(roblox::globals::mouse_service->self + 0xCC) = 0;
	//}

	//if (to_lock != -1 && roblox::globals::mouse_service && !ui::Static::render_interface)
	//{
	//	*reinterpret_cast<std::uint32_t*>(roblox::globals::mouse_service->self + 0xCC) = to_lock;

	//	to_lock = -1;
	//}

	return waiting_script_job_original(job, stats);
}

std::uint32_t find_cap_offset(std::uintptr_t task_scheduler)
{
	for (auto i = 0u; i < 0x1000; i += 0x4)
	{
		const auto offset = *reinterpret_cast<double*>(task_scheduler + i);

		if (1.0 / offset == 60.0)
			return i;
	}

	return 0;
}

bool task_scheduler::init()
{
	std::uintptr_t waiting_script_job = roblox::functions::task_scheduler::get_job("WaitingHybridScriptsJob", false); // roblox::offsets::fflags::is_in_beta()

	std::printf("[TS] Job: %X\n", waiting_script_job);

	if (!waiting_script_job)
		return false;

	constexpr std::uint8_t virtual_function_table_size = 7, waiting_script_job_hook_index = 5, waiting_script_job_destroy_hook_index = 0;

	auto virtual_function_table = new void* [virtual_function_table_size];

	std::memcpy(virtual_function_table, *reinterpret_cast<std::uintptr_t**>(waiting_script_job), virtual_function_table_size * sizeof(void*));

	waiting_script_job_original = reinterpret_cast<job_callback_t>(virtual_function_table[waiting_script_job_hook_index]);
	waiting_script_job_destroy_original = reinterpret_cast<job_callback_destroy_t>(virtual_function_table[waiting_script_job_destroy_hook_index]);

	virtual_function_table[waiting_script_job_hook_index] = waiting_script_job_hook;
	virtual_function_table[waiting_script_job_destroy_hook_index] = waiting_script_job_destroy_hook;

	*reinterpret_cast<std::uintptr_t*>(waiting_script_job) = reinterpret_cast<std::uintptr_t>(virtual_function_table);

	std::printf("[TS] Hooked\n");

	return true;
}

std::uint32_t cap_offset = 0;

void task_scheduler::set_fps_cap(double fps)
{
	const auto task_scheduler = roblox::functions::task_scheduler::get_task_scheduler();

	if (!cap_offset)
		cap_offset = find_cap_offset(task_scheduler);

	*reinterpret_cast<double*>(task_scheduler + cap_offset) = 1.0 / fps;
}