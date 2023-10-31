#include "memory.hpp"

#include <filesystem>

erw::erw(const char* const process_name)
{
	const auto hwnd = FindWindowA(nullptr, process_name);

	GetWindowThreadProcessId(hwnd, &pid);

	process_handle = smart_handle{ OpenProcess(PROCESS_ALL_ACCESS, false, pid) };
}

erw::erw(HANDLE handle)
{
	pid = GetProcessId(handle);

	process_handle = smart_handle{ handle };
}

std::uintptr_t erw::get_process_module(const char* const module_name) const
{
    MODULEENTRY32W me32;

	auto snapshot = smart_handle{ CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid) };

    me32.dwSize = sizeof(MODULEENTRY32W);

    auto mod_res = Module32FirstW(snapshot.get(), &me32);
 
    while (mod_res)
    {
        if (std::equal(module_name, module_name + strlen(module_name), std::wstring(me32.szModule).begin()))
            return reinterpret_cast<std::uintptr_t>(me32.modBaseAddr);

        mod_res = Module32NextW(snapshot.get(), &me32);
    }

    return {};
}

section_data erw::get_section_by_name(const char* const module_name, const char* const section_name)
{
	const auto mod_base = get_process_module(module_name);

	const auto dos_header = read_memory<IMAGE_DOS_HEADER>(mod_base);
	const auto nt_headers = read_memory<IMAGE_NT_HEADERS>(mod_base + dos_header.e_lfanew);

	auto section = reinterpret_cast<IMAGE_SECTION_HEADER*>((mod_base + dos_header.e_lfanew) + sizeof(IMAGE_NT_HEADERS));

	for (auto iteration = 0u; iteration < nt_headers.FileHeader.NumberOfSections; ++iteration, ++section)
	{
		const auto segment_name = section->Name;

		auto buffer = reinterpret_cast<std::uintptr_t>(segment_name);

		std::string string_buffer;

		do
			string_buffer.push_back(read_memory<std::uint8_t>(buffer++));
		while (read_memory<std::uint8_t>(buffer) != '\0');

		if (std::strcmp(string_buffer.c_str(), section_name) == 0)
		{
			const auto section_start = mod_base + read_memory<std::uint32_t>(reinterpret_cast<std::uintptr_t>(section) + 0xC);
			const auto section_size = read_memory<std::uint32_t>(reinterpret_cast<std::uintptr_t>(section) + 0x8);

			return { section_start, section_size };
		}
	}

	return {};
}

std::uintptr_t erw::get_export(const char* const module_name, const char* const function_name)
{
	const auto base_address = get_process_module(module_name);

	const auto dos_header = read_memory<IMAGE_DOS_HEADER>(base_address);

	const auto nt_headers = read_memory<IMAGE_NT_HEADERS>(base_address + dos_header.e_lfanew);

	const auto export_directory = read_memory<IMAGE_EXPORT_DIRECTORY>(base_address + nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	const auto functions = base_address + export_directory.AddressOfFunctions;
	const auto names = base_address + export_directory.AddressOfNames;
	const auto ordinals = base_address + export_directory.AddressOfNameOrdinals;

	for (auto i = 0u; i < export_directory.NumberOfNames; ++i)
	{
		auto buffer = reinterpret_cast<std::uintptr_t>(reinterpret_cast<char*>(base_address) + read_memory<std::uint32_t>(names + i * sizeof(std::uint32_t)));

		std::string string_buffer;

		do
			string_buffer.push_back(read_memory<std::uint8_t>(buffer++));
		while (read_memory<std::uint8_t>(buffer) != '\0');

		if (std::strcmp(string_buffer.c_str(), function_name) == 0)
			return base_address + read_memory<std::uint32_t>(functions + read_memory<std::uint16_t>(ordinals + i * sizeof(std::uint16_t)) * sizeof(std::uint32_t));
	}

	return {};
}

bool erw::load_dll(const char* const path)
{
	auto current_directory = std::filesystem::current_path().string().append("/");

	for (auto i = 0u; i < current_directory.length(); ++i)
		if (current_directory[i] == '/')
			current_directory[i] = '\\';

	const auto str_addr = reinterpret_cast<std::uintptr_t>(allocate_string(current_directory.append(path).c_str()));
	const auto payload_alloc = reinterpret_cast<std::uintptr_t>(VirtualAllocEx(process_handle.get(), 0, 0x100, MEM_COMMIT, PAGE_EXECUTE_READWRITE));
	const auto loadlib_addr = get_export("KERNELBASE.dll", "LoadLibraryA");

	if (!payload_alloc)
		return false;

#ifdef _M_X64
	std::uint8_t payload[] =
	{
		0x48, 0x83, 0xEC, 0x28, 0x48,
		0xB9, 0x50, 0x23, 0xC4, 0x29,
		0xF6, 0x7F, 0x00, 0x00, 0xFF,
		0x15, 0x02, 0x00, 0x00, 0x00,
		0xEB, 0x08, 0x30, 0x8C, 0xDF,
		0x24, 0xF8, 0x7F, 0x00, 0x00,
		0x33, 0xC0, 0x48, 0x83, 0xC4,
		0x28, 0xC3
	};

	*reinterpret_cast<std::uintptr_t*>(payload + 6) = str_addr;
	*reinterpret_cast<std::uintptr_t*>(payload + 22) = loadlib_addr;
#else
	std::uint8_t payload[] =
	{
		0x55, 0x89, 0xE5, 0x68, 0x00,
		0x00, 0x00, 0x00, 0xE8, 0x00,
		0x00, 0x00, 0x00, 0x5D, 0xC2,
		0x00, 0x00
	};

	*reinterpret_cast<std::uintptr_t*>(payload + 4) = str_addr;
	*reinterpret_cast<std::uintptr_t*>(payload + 9) = loadlib_addr - (payload_alloc + 8) - 5;
#endif

	WriteProcessMemory(process_handle.get(), reinterpret_cast<void*>(payload_alloc), payload, sizeof(payload), 0);

	const auto handle = CreateRemoteThread(process_handle.get(), nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(payload_alloc), nullptr, 0, nullptr);

	if (handle != nullptr)
	{
		DWORD res = WaitForSingleObject(handle, 5000);

		VirtualFreeEx(process_handle.get(), reinterpret_cast<void*>(payload_alloc), 0, MEM_RELEASE);
		VirtualFreeEx(process_handle.get(), reinterpret_cast<void*>(str_addr), 0, MEM_RELEASE);

		return res != 0;
	}

	return false;
}

const char* erw::allocate_string(const char* const str, std::uint32_t size)
{
	std::uint32_t str_size;

	if (size == 0)
		str_size = strlen(str) + 1;
	else
		str_size = size;

	void* memory = VirtualAllocEx(process_handle.get(), 0, str_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!memory)
		return nullptr;

	WriteProcessMemory(process_handle.get(), memory, str, str_size, nullptr);

	return static_cast<const char*>(memory);
}

std::uintptr_t erw::pattern_scan(std::string_view module_name, std::string_view section_name, std::string_view pattern, std::string_view mask)
{
	const auto data = get_section_by_name(module_name.data(), section_name.data());

	auto buffer = std::make_unique<std::uint8_t[]>(0x1000);

	for (auto at = data.start; at < data.start + data.size; at += sizeof(buffer))
	{
		ReadProcessMemory(process_handle.get(), reinterpret_cast<void*>(at), buffer.get(), 0x1000, nullptr);

		for (auto b = 0u; b < sizeof(buffer); ++b)
		{
			const auto is_same = [&]() -> bool
			{
				for (auto i = 0u; i < mask.length(); ++i)
					if (buffer[b + i] != static_cast<std::uint8_t>(pattern[i]) && mask[i] == 'x')
						return false;

				return true;
			};

			if (is_same())
				return at + b;
		}
	}

	return {};
}

void* erw::map_function(void* src)
{
	const auto size = get_function_size(src);

	auto memory = VirtualAllocEx(process_handle.get(), nullptr, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!memory)
		return {};

	auto buffer = std::make_unique<std::uint8_t[]>(size);

	for (auto i = 0u; i < size; ++i)
		buffer[i] = static_cast<std::uint8_t*>(src)[i];

	WriteProcessMemory(process_handle.get(), memory, buffer.get(), size, nullptr);

	return memory;
}

DWORD erw::set_protection(std::uintptr_t address, std::uint16_t protection)
{
	DWORD old;

	VirtualProtectEx(process_handle.get(), reinterpret_cast<void*>(address), sizeof(address), protection, &old);

	return old;
}

std::size_t erw::get_function_size(void* src)
{
	std::uint8_t* bytes = static_cast<std::uint8_t*>(src);

	do
		++bytes;
	while (!(bytes[0] == 0xCC && bytes[1] == 0xCC));

	return bytes - src;
}