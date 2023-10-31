#include <Windows.h>
#include <thread>

#define BYPASS_DEBUG 0

void open_console()
{
	const auto lib = LoadLibraryA("KERNEL32.dll");

	if (!lib)
		return;

	const auto free_console = reinterpret_cast<std::uintptr_t>(GetProcAddress(lib, "FreeConsole"));

	if (free_console)
	{
		static std::uintptr_t jmp = free_console + 0x6;

		DWORD old{};

		VirtualProtect(reinterpret_cast<void*>(free_console), 0x6, PAGE_EXECUTE_READWRITE, &old);

		*reinterpret_cast<std::uintptr_t**>(free_console + 0x2) = &jmp;
		*reinterpret_cast<std::uint8_t*>(free_console + 0x6) = 0xC3;

		VirtualProtect(reinterpret_cast<void*>(free_console), 0x6, old, &old);
	}

	AllocConsole();

	FILE* file_stream;

	freopen_s(&file_stream, "CONIN$", "r", stdin);
	freopen_s(&file_stream, "CONOUT$", "w", stdout);
	freopen_s(&file_stream, "CONOUT$", "w", stderr);

	fclose(file_stream);

	SetConsoleTitleA("Ro-Ware-Hook");
}

std::vector<std::uintptr_t> get_wow64_clones()
{
	std::vector<std::uintptr_t> allocations;

	std::uintptr_t addr = 0;

	MEMORY_BASIC_INFORMATION mbi;

	while (VirtualQuery(reinterpret_cast<std::uintptr_t*>(addr), &mbi, sizeof(mbi)))
	{
		if (mbi.State == MEM_COMMIT && mbi.Protect == PAGE_EXECUTE_READ && mbi.RegionSize == 0x1000)
		{
			const auto data = reinterpret_cast<std::uint8_t*>(mbi.BaseAddress);

			if (data[0] == 0xFF && data[1] == 0x25 && data[10] == 0xEA)
				for (auto i = 0; i < 0x50; ++i)
					if ((data + i)[0] == 0x00 && (data + i)[3] == 0x23)
						allocations.push_back(reinterpret_cast<std::uintptr_t>(mbi.BaseAddress) + i + 2);
		}

		addr += mbi.RegionSize;
	}

	return allocations;
}

std::uintptr_t tramp_hook(std::uintptr_t func, std::uintptr_t new_func, std::size_t inst_size)
{
	constexpr auto extra_size = 5;

	auto clone = reinterpret_cast<std::uintptr_t>(VirtualAlloc(nullptr, inst_size + extra_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

	if (!clone)
		return 0;

	std::memmove(reinterpret_cast<void*>(clone), reinterpret_cast<void*>(func), inst_size);

	*reinterpret_cast<std::uint8_t*>(clone + inst_size) = 0xE9;
	*reinterpret_cast<std::uintptr_t*>(clone + inst_size + 1) = (func - clone - extra_size);

	DWORD old_protect{ 0 };

	VirtualProtect(reinterpret_cast<void*>(func), inst_size, PAGE_EXECUTE_READWRITE, &old_protect);

	std::memset(reinterpret_cast<void*>(func), 0x90, inst_size);

	*reinterpret_cast<std::uint8_t*>(func) = 0xE9;
	*reinterpret_cast<std::uintptr_t*>(func + 1) = (new_func - func - extra_size);

	VirtualProtect(reinterpret_cast<void*>(func), inst_size, old_protect, &old_protect);

	return clone;
}

const auto query_vir_mem_address = reinterpret_cast<std::uintptr_t>(GetProcAddress(LoadLibraryA("ntdll.dll"), "NtQueryVirtualMemory"));

using nt_query_virtual_memory_t = NTSTATUS(__stdcall*)(HANDLE process_handle, std::uintptr_t base_address, std::uint32_t mem_info_class, MEMORY_BASIC_INFORMATION* mem_info, std::size_t memory_info_length, std::size_t* return_len);
auto nt_query_virtual_memory = reinterpret_cast<nt_query_virtual_memory_t>(query_vir_mem_address);

std::uintptr_t base_of_dll = 0;
std::size_t size_of_dll = 0;

struct rbx_data
{
	std::uintptr_t base;
	std::size_t size;
};

_declspec(dllexport) void __stdcall gamer(std::uintptr_t data)
{
	const auto infos = reinterpret_cast<rbx_data*>(data);

	base_of_dll = infos->base;
	size_of_dll = infos->size;
}

NTSTATUS __stdcall nt_query_virtual_memory_hook(HANDLE process_handle, std::uintptr_t base_address, std::uint32_t mem_info_class, MEMORY_BASIC_INFORMATION* mem_info, std::size_t memory_info_length, std::size_t* return_len)
{
	const auto ret = nt_query_virtual_memory(process_handle, base_address, mem_info_class, mem_info, memory_info_length, return_len);

	if (!base_of_dll || !mem_info || !base_address)
		return ret;

	if (reinterpret_cast<std::uintptr_t>(mem_info->BaseAddress) == base_of_dll)
	{
#if BYPASS_DEBUG
		std::printf("Allocation Skipped\n");
#endif

		mem_info->AllocationProtect = 0;
		mem_info->Protect = PAGE_NOACCESS;
		mem_info->State = MEM_FREE;
		mem_info->Type = 0;
		mem_info->RegionSize = size_of_dll;
	}

	return ret;
}

void d_main()
{
#if BYPASS_DEBUG
	open_console();
#endif

	auto allocs = get_wow64_clones();

	while (true)
	{
		allocs = get_wow64_clones();

		if (allocs.size() != 2)
			std::this_thread::sleep_for(std::chrono::seconds(1));
		else
			break;
	}

	for (const auto alloc : allocs)
	{
		DWORD old_protect{ 0 };

		VirtualProtect(reinterpret_cast<void*>(alloc), 5, PAGE_EXECUTE_READWRITE, &old_protect);

		*reinterpret_cast<std::uint8_t*>(alloc) = 0xE9;
		*reinterpret_cast<std::uintptr_t*>(alloc + 1) = (query_vir_mem_address - alloc - 5);

		VirtualProtect(reinterpret_cast<void*>(alloc), 5, old_protect, &old_protect);

#if BYPASS_DEBUG
		std::printf("Patched Clone: %x\n", alloc);
#endif
	}

	nt_query_virtual_memory = reinterpret_cast<nt_query_virtual_memory_t>(tramp_hook(query_vir_mem_address, reinterpret_cast<std::uintptr_t>(nt_query_virtual_memory_hook), 5));
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		std::thread{ d_main }.detach();

	return TRUE;
}