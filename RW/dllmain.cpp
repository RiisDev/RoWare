#include <Windows.h>

#include "roblox/roblox.hpp"
#include "task_scheduler/task_scheduler.hpp"

#include "dependencies/exception_handler/ExceptionHandler.hpp"
#include "dependencies/oxorany/oxorany.h"

#include <thread>
#include <set>
#include <iostream>
#include <filesystem>

#ifdef MAIN_DEBUG

#define Log printf

#define VMProtectDecryptStringA

#else

#define Log __noop

#endif

HMODULE mod;

std::uintptr_t jmp;

void open_console()
{
	const auto lib = LoadLibraryA("KERNEL32.dll");

	if (!lib)
		return;

	const auto free_console = reinterpret_cast<std::uintptr_t>(GetProcAddress(lib, "FreeConsole"));

	if (free_console)
	{
		jmp = free_console + 0x6;

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

	SetConsoleTitleA("Ro-Ware");
}

void callback(ExceptionManager::EHReport report)
{
	std::string messageString = report.report_string;/* (
		"You've encountered a crash! :(\n"
		"\n"
		"In order for us to look into this crash, please DM this crash log to a RoWare developer gogo1000#1977 or iivillian#7652 (https://script-ware.com/discord)"
		"\n"
		"-------------------------------------"
		"\n"
		+ AES::encryption(report.report_string, oxorany("5ujTDzHuH3kRJzMt4Sns5g8v5M4VY95+t7HrwnhJhko="), oxorany("QmI4WlBxd00xSnFzck5SejNwV0ZXUT09")));*/

	auto autogeneratedCrashReportName = [&]()
	{
		time_t t = time(0);   // get time now
		struct tm* now = localtime(&t);

		wchar_t buffer[80];
		wcsftime(buffer, 80, oxorany(L"\\%d-%m-%Y_%H-%M-%S_RoWareCrash.txt"), now);
		return std::wstring(buffer);
	};

	auto path = std::filesystem::temp_directory_path()
		.parent_path()
		.parent_path();

	path /= "Local";

	std::wstring crash_folder = path.append(L"\\crashlogs");
	if (!std::filesystem::is_directory(crash_folder))
		std::filesystem::create_directory(crash_folder);
	else
	{
		for (auto const& dir_entry : std::filesystem::directory_iterator{ crash_folder })
		{
			if (!dir_entry.is_directory()
				&& dir_entry.path().string().find(oxorany("RoWareCrash")) != std::string::npos)
			{
				struct _stat fileInfo;
				_wstat(dir_entry.path().c_str(), &fileInfo);
				char* string_result = std::ctime(&fileInfo.st_ctime);

				// Delete all logs older than 48 hours
				if (time(NULL) - fileInfo.st_ctime > 86400 * 2) // seconds in a day
				{
					// So we dont get exception
					std::error_code error;
					std::filesystem::remove(dir_entry, error);
				}
			}
		}
	}

	auto fileName = autogeneratedCrashReportName();
	fileName = crash_folder + fileName;

	if (!std::filesystem::exists(fileName))
	{
		std::ofstream F(fileName.c_str(), std::ios::binary);
		if (F.is_open())
		{
			F.write(messageString.c_str(), messageString.size());
			F.close();
		}

		else
			Log("Failed to create file: %08x\n", GetLastError());
	}

	else Log("Something failed very badly\n");

	STARTUPINFOW stinfo = { 0 };
	PROCESS_INFORMATION prinfo = { 0 };
	CreateProcessW(
		oxorany(L"C:\\Windows\\System32\\notepad.exe"),
		(LPWSTR)((oxorany(L"notepad.exe ") + fileName).c_str()),
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&stinfo,
		&prinfo
	);
}

bool d_main()
{
	//open_console();

	//PostToServer(oxorany("https://script-ware.com/api/roware/init"), "LOG");

	/*MEMORY_BASIC_INFORMATION mbi;

	VirtualQuery(mod, &mbi, sizeof(mbi));

	ExceptionManager::EHSettings settings{};

	std::vector<std::uintptr_t> blacklist_code = { 0x80000004, 0x80000006, 0x406D1388, 0xE06D7363, 0x40010006, 0x4001000A };
	std::vector<std::string> blacklist_sym;

	settings.use_veh = true;
	settings.use_seh = false;
	settings.is_prog_dll = true;
	settings.callback = callback;
	settings.prog_name = "Ro-Ware.dll";
	settings.prog_base = reinterpret_cast<std::uintptr_t>(mod);
	settings.prog_size = mbi.RegionSize;
	settings.blacklist_code = blacklist_code;
	settings.blacklist_sym = blacklist_sym;

	ExceptionManager::Init(&settings);*/

	if (roblox::offsets::fflags::is_in_beta())
		std::thread{ [] { while (!task_scheduler::init()) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); } } }.detach();
	else
	{
		std::uint32_t attempts = 0;

		while (!task_scheduler::init())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			++attempts;

			if (attempts > 30)
				return false;
		}
	}

	return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		mod = hModule;

		return d_main();
	}

    return TRUE;
}