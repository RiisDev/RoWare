#pragma once
#include <Windows.h>
#include "../memory/memory.hpp"

namespace manual_map
{
	bool map_dll(erw& mem, const char* const dll_name, const char* process_name, std::uintptr_t base_of_stub, std::uintptr_t& base, std::size_t& size);
}