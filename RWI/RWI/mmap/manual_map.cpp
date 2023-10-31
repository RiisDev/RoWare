#include <Windows.h>
#include <TlHelp32.h>
#include <comdef.h>

#include <iostream>

#include "manual_map.hpp"

struct dll_data
{
	std::uintptr_t base;

	HMODULE(__stdcall* load_library)(const char*);

	FARPROC(__stdcall* get_proc_address)(HMODULE, const char*);
};

void __stdcall mapper(const dll_data* data)
{
	const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(data->base)->e_lfanew;
	const auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(data->base + dos_header);

	auto reallocs = reinterpret_cast<PIMAGE_BASE_RELOCATION>(data->base + nt_headers->OptionalHeader.DataDirectory[5].VirtualAddress);
	auto imports = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(data->base + nt_headers->OptionalHeader.DataDirectory[1].VirtualAddress);

	while (reallocs->VirtualAddress)
	{
		for (std::uint32_t i = 0u, iteration = (reallocs->SizeOfBlock - 8) / 2; i < iteration; ++i)
			if (reinterpret_cast<std::uint16_t*>(reallocs + 1)[i] >> 12 == 3)
				*reinterpret_cast<std::uintptr_t*>(data->base + (reallocs->VirtualAddress + (reinterpret_cast<std::uint16_t*>(reallocs + 1)[i] & 0xFFF))) += data->base - nt_headers->OptionalHeader.ImageBase;

		reallocs = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reinterpret_cast<std::uint8_t*>(reallocs) + reallocs->SizeOfBlock);
	}

	while (imports->Characteristics)
	{
		PIMAGE_THUNK_DATA original_first_thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(data->base + imports->OriginalFirstThunk);
		PIMAGE_THUNK_DATA first_thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(data->base + imports->FirstThunk);

		HMODULE module = data->load_library(reinterpret_cast<const char*>(data->base) + imports->Name);

		while (original_first_thunk->u1.AddressOfData)
		{
			std::uintptr_t function;

			if (original_first_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
				function = reinterpret_cast<std::uintptr_t>(data->get_proc_address(module, reinterpret_cast<const char*>(original_first_thunk->u1.Ordinal & 0xFFF)));
			else
				function = reinterpret_cast<std::uintptr_t>(data->get_proc_address(module, (reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(reinterpret_cast<std::uint8_t*>(data->base) + original_first_thunk->u1.AddressOfData))->Name));

			first_thunk->u1.Function = function;
			original_first_thunk++;
			first_thunk++;
		}

		imports++;
	}

	if (nt_headers->OptionalHeader.AddressOfEntryPoint)
	{
		const auto entry_point = data->base + nt_headers->OptionalHeader.AddressOfEntryPoint;

		reinterpret_cast<int(__stdcall*)(HMODULE, int, int)>(entry_point)(reinterpret_cast<HMODULE>(data->base), DLL_PROCESS_ATTACH, 0);
	}
}

bool manual_map::map_dll(erw& mem, const char* const dll_name, const char* process_name, std::uintptr_t base_of_stub, std::uintptr_t& base, std::size_t& size)
{
	const auto handle = mem.get_handle();

	FILE* file;

	fopen_s(&file, dll_name, "rb");

	if (!file)
		return false;

	std::uint32_t start = ftell(file);

	fseek(file, 0, 2);

	std::uint32_t end = ftell(file);

	fseek(file, start, 0);

	auto binary = std::make_unique<std::uint8_t[]>(end);

	fread(binary.get(), end, 1, file);

	fclose(file);

	DWORD pid{};

	const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(binary.get())->e_lfanew;
	const auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(binary.get() + dos_header);

	const auto section_headers = reinterpret_cast<PIMAGE_SECTION_HEADER>(nt_headers + 1);

	const auto raw_dll = VirtualAllocEx(handle, 0, nt_headers->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	const auto inj_stub = VirtualAllocEx(handle, 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	size = nt_headers->OptionalHeader.SizeOfImage;

	const auto dll_data_allocation = VirtualAllocEx(handle, 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!raw_dll || !dll_data_allocation)
		return false;

	WriteProcessMemory(handle, raw_dll, binary.get(), nt_headers->OptionalHeader.SizeOfHeaders, 0);

	for (auto i = 0u; i < nt_headers->FileHeader.NumberOfSections; ++i)
		WriteProcessMemory(handle, reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(raw_dll) + section_headers[i].VirtualAddress), binary.get() + section_headers[i].PointerToRawData, section_headers[i].SizeOfRawData, 0);

	dll_data data_struct;

	data_struct.base = reinterpret_cast<std::uintptr_t>(raw_dll);
	data_struct.load_library = reinterpret_cast<decltype(&LoadLibraryA)>(mem.get_export("KERNEL32.DLL", "LoadLibraryA"));
	data_struct.get_proc_address = reinterpret_cast<decltype(&GetProcAddress)>(mem.get_export("KERNEL32.DLL", "GetProcAddress"));

	base = reinterpret_cast<std::uintptr_t>(raw_dll);

	WriteProcessMemory(handle, dll_data_allocation, &data_struct, sizeof(data_struct), 0);
	WriteProcessMemory(handle, reinterpret_cast<void*>(inj_stub), mapper, 0x200, 0);

	HANDLE thread_handle = CreateRemoteThread(handle, 0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(inj_stub), dll_data_allocation, 0, 0);

	if (!thread_handle || thread_handle == INVALID_HANDLE_VALUE)
		return false;

	WaitForSingleObject(thread_handle, 4000);

	DWORD return_val{};

	GetExitCodeThread(thread_handle, &return_val);

	return return_val;
}