#include <iostream>

#include "memory/memory.hpp"
#include "mmap/manual_map.hpp"

#include <filesystem>
#include <thread>
#include <curl/curl.h>

HANDLE get_process_handle(const wchar_t* process_name, bool check_text = false)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	HANDLE process = nullptr;

	PROCESSENTRY32W process_entry;

	process_entry.dwSize = sizeof(process_entry);

	if (Process32FirstW(snapshot, &process_entry))
	{
		do
		{
			if (std::wcscmp(process_entry.szExeFile, process_name) == 0)
			{
				HANDLE new_handle = OpenProcess(PROCESS_ALL_ACCESS, false, process_entry.th32ProcessID);

				auto mem = erw(new_handle);

				const auto text = mem.get_section_by_name("RobloxPlayerBeta.exe", ".text");

				if ((check_text && text.size != 0) || (!check_text && text.size == 0))
				{
					process = OpenProcess(PROCESS_ALL_ACCESS, false, process_entry.th32ProcessID);

					break;
				}
			}

		} while (Process32NextW(snapshot, &process_entry));
	}

	CloseHandle(snapshot);

	return process;
}

struct rbx_data
{
	std::uintptr_t base;
	std::size_t size;
};

struct MemoryStruct
{
	char* memory;
	size_t size;
};

static size_t write_memory_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct* mem = (struct MemoryStruct*)userp;

	char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if (ptr == NULL)
	{
		printf("not enough memory (realloc returned NULL)\n");

		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);

	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

const char* send_get_request(const char* url)
{
	static bool init = false;

	if (!init)
	{
		curl_global_init(CURL_GLOBAL_ALL);

		init = true;
	}

	CURL* curl_handle;
	CURLcode res;

	struct MemoryStruct chunk;

	chunk.memory = (char*)malloc(1);
	chunk.size = 0;

	curl_handle = curl_easy_init();

	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&chunk));
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	res = curl_easy_perform(curl_handle);

	curl_easy_cleanup(curl_handle);

	return chunk.memory;
}

extern "C"
{
	__declspec(dllexport) int __stdcall GetPid()
	{
		auto handle = get_process_handle(L"RobloxPlayerBeta.exe", true);

		if (!handle || handle == INVALID_HANDLE_VALUE)
			return 0;

		const auto pid = GetProcessId(handle);

		CloseHandle(handle);

		return pid;
	}

	__declspec(dllexport) int __stdcall InjectRoWare()
	{
		std::uintptr_t base{};
		std::size_t size{};

		if (!std::filesystem::exists(".\\bin\\RoWareCore.dll") || !std::filesystem::exists(".\\bin\\RoWareHelper.dll"))
			return 1;

		const auto res = send_get_request("https://script-ware.com/api/roware/info");

		if (!res)
			return 1;

		const auto text_size_server = std::atoi(res);

		{
			std::uint32_t attempts = 0;

			auto handle = get_process_handle(L"RobloxPlayerBeta.exe", true);

			while (!handle)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				handle = get_process_handle(L"RobloxPlayerBeta.exe", true);

				++attempts;

				if (attempts > 30)
					return 1;
			}

			if (!handle || handle == INVALID_HANDLE_VALUE)
				return 3;

			auto mem = erw(handle);

			auto base_address = mem.get_process_module("RobloxPlayerBeta.exe");

			const auto dos_header = mem.read_memory<IMAGE_DOS_HEADER>(base_address);

			const auto nt_headers = mem.read_memory<IMAGE_NT_HEADERS>(base_address + dos_header.e_lfanew);

			auto text_section = mem.read_memory<IMAGE_SECTION_HEADER>(base_address + dos_header.e_lfanew + sizeof(IMAGE_NT_HEADERS));

			const auto text_size = text_section.Misc.VirtualSize;

			if (text_size_server != text_size)
				return 2;

			const auto inj_status = manual_map::map_dll(mem, ".\\bin\\RoWareCore.dll", "RobloxPlayerBeta.exe", 0, base, size);

			//if (!inj_status)
				//return 1;
		}

		/*{
			std::uint32_t attempts = 0;

			auto handle = get_process_handle(L"RobloxPlayerBeta.exe", false);

			while (!handle)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				handle = get_process_handle(L"RobloxPlayerBeta.exe", false);

				++attempts;

				if (attempts > 30)
					return 1;
			}

			if (!handle || handle == INVALID_HANDLE_VALUE)
				return 3;

			auto mem = erw(handle);

			mem.load_dll("bin\\RoWareHelper.dll");

			const auto gamer = mem.get_export("RoWareHelper.dll", "?gamer@@YGXI@Z");

			rbx_data data;

			data.base = base;
			data.size = size;

			mem.call_function<cc_stdcall, std::uint32_t, rbx_data*>(gamer, { mem.allocate_object(data) });
		}*/

		return 0;
	}
}

//
//BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
//{
//	return TRUE;
//}