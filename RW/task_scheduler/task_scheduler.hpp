#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

#include "../roblox/roblox.hpp"

namespace task_scheduler
{
	bool init();

	void set_fps_cap(double fps);
}