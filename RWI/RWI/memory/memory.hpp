#pragma once
#include <cstddef>
#include <string>
#include <memory>
#include <type_traits>
#include <comdef.h>
#include <tlhelp32.h>

struct section_data
{
	std::uintptr_t start;
	std::uint32_t size;
};

enum cc : std::uint32_t
{
	cc_cdecl,
	cc_stdcall,
	cc_fastcall
};

struct func_data
{
	std::uintptr_t tuple_address;
	std::uintptr_t func_address;
};

template<cc convention, typename ret, typename... args>
ret __stdcall caller(func_data* data)
{
	const auto tuple = *reinterpret_cast<std::tuple<args...>*>(data->tuple_address);

	if constexpr (convention == cc_cdecl)
	{
		typedef ret(__cdecl* def)(args...);
		def func = reinterpret_cast<def>(data->func_address);

		return std::apply(func, tuple);
	}

	else if constexpr (convention == cc_stdcall)
	{
		typedef ret(__stdcall* def)(args...);
		def func = reinterpret_cast<def>(data->func_address);

		return std::apply(func, tuple);
	}

	else if constexpr (convention == cc_fastcall)
	{
		typedef ret(__fastcall* def)(args...);
		def func = reinterpret_cast<def>(data->func_address);

		return std::apply(func, tuple);
	}
}

class erw
{
private:
	template<auto t>
	using constant_t = std::integral_constant<decltype(t), t>;

	using smart_handle = std::unique_ptr<std::remove_pointer_t<HANDLE>, constant_t<CloseHandle>>;

	smart_handle process_handle;

	DWORD pid;

	std::size_t get_function_size(void* src);

public:
	erw(const char* const process_name);
	erw(DWORD process_id) : pid{ process_id } { process_handle = smart_handle{ OpenProcess(PROCESS_ALL_ACCESS, false, process_id) }; };
	erw(HANDLE handle);
	erw(const erw&) = delete;

	std::uintptr_t get_process_module(const char* const module_name) const;

	section_data get_section_by_name(const char* const module_name, const char* const section_name);

	std::uintptr_t get_export(const char* const module_name, const char* const function_name);

	bool load_dll(const char* const path);

	const char* allocate_string(const char* const str, std::uint32_t size = 0);

	std::uintptr_t pattern_scan(std::string_view module_name, std::string_view section_name, std::string_view pattern, std::string_view mask);

	void* map_function(void* src);

	DWORD set_protection(std::uintptr_t address, std::uint16_t protection);

	bool is_handle_open() {  return process_handle.get() != INVALID_HANDLE_VALUE && process_handle.get() != 0; };

	HANDLE get_handle() { return process_handle.get(); }

	template<typename type>
	type read_memory(std::uintptr_t address) const
	{
		type buffer;

		bool result = ReadProcessMemory(process_handle.get(), reinterpret_cast<void*>(address), &buffer, sizeof(buffer), nullptr);

		if (result)
			return buffer;

		return {};
	}

	template<typename type, class t, std::size_t n>
	void read_buffer(std::uintptr_t address, t(&buffer)[n]) const { ReadProcessMemory(process_handle.get(), reinterpret_cast<void*>(address), &buffer, n, nullptr); }

	template<typename type>
	bool write_memory(std::uintptr_t address, type buffer) const
	{
		bool result = WriteProcessMemory(process_handle.get(), reinterpret_cast<void*>(address), &buffer, sizeof(buffer), nullptr);

		return result;
	}

	template<typename type>
	type* allocate_object(type src)
	{
		type* obj = static_cast<type*>(VirtualAllocEx(process_handle.get(), 0, sizeof(src), MEM_COMMIT, PAGE_READWRITE));

		if (!obj)
			return nullptr;

		WriteProcessMemory(process_handle.get(), obj, &src, sizeof(src), nullptr);

		return obj;
	}

	template<cc convention, typename ret, typename... args>
	ret call_function(std::uintptr_t address, std::tuple<args...> tup, bool wait_for_exit = true)
	{
		const auto func = map_function(caller<convention, ret, args...>);
		const auto tuple = allocate_object(tup);

		func_data data;
		data.func_address = address;
		data.tuple_address = reinterpret_cast<std::uintptr_t>(tuple);

		const auto func_data = this->allocate_object(data);

		const auto handle = CreateRemoteThread(process_handle.get(), nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(func), func_data, 0, nullptr);

		if (handle)
		{
			WaitForSingleObject(handle, wait_for_exit ? 5000 : 100);

			DWORD return_val{};

			GetExitCodeThread(handle, &return_val);

			if (wait_for_exit)
			{
				VirtualFreeEx(process_handle.get(), func, 0, MEM_RELEASE);
				VirtualFreeEx(process_handle.get(), func_data, 0, MEM_RELEASE);
				VirtualFreeEx(process_handle.get(), tuple, 0, MEM_RELEASE);
			}

			return return_val;
		}

		return {};
	}
};