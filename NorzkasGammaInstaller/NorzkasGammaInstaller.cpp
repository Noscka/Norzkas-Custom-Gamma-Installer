#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "EXTERNAL/httplib.h"

#include <NosLib/Console.hpp>

#include <windows.h>
#include <conio.h>
#include <fstream>
#include <format>

void PrintServerResponse(const httplib::Result& serverResult)
{
	if (serverResult)
	{
		wprintf(L"\n\nSuccessful\n");
	}
	else
	{
		wprintf(std::format(L"error code: {}\n", (int)serverResult.error()).c_str());
	}
}

void LoggingFunction(const httplib::Request& req, const httplib::Response& res)
{
	wprintf(L"====================================================================================================================\nRequest\n");
	wprintf(std::format(L":METHOD: {}\n", NosLib::String::ConvertString<wchar_t, char>(req.method)).c_str());
	wprintf(std::format(L":PATH:   {}\n", NosLib::String::ConvertString<wchar_t, char>(req.path)).c_str());
	wprintf(std::format(L":BODY:   {}\n", NosLib::String::ConvertString<wchar_t, char>(req.body)).c_str());

	wprintf(L"======HEADERS======\n");

	for (auto itr = req.headers.begin(); itr != req.headers.end(); itr++)
	{
		wprintf(NosLib::String::ConvertString<wchar_t, char>(std::format("{} : {}\n", itr->first, itr->second)).c_str());
	}
	wprintf(L"====================================================================================================================\nResponse\n");

	wprintf(std::format(L":STATUS: {}\n", res.status).c_str());
	wprintf(std::format(L":REASON: {}\n", NosLib::String::ConvertString<wchar_t, char>(res.reason)).c_str());
	wprintf(std::format(L":BODY:   {}\n", NosLib::String::ConvertString<wchar_t, char>(res.body)).c_str());
	wprintf(std::format(L":LOCATION:   {}\n", NosLib::String::ConvertString<wchar_t, char>(res.location)).c_str());

	wprintf(L"======HEADERS======\n");

	for (auto itr = res.headers.begin(); itr != res.headers.end(); itr++)
	{
		wprintf(NosLib::String::ConvertString<wchar_t, char>(std::format("{} : {}\n", itr->first, itr->second)).c_str());
	}
	wprintf(L"====================================================================================================================\n");
}

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
	NosLib::Console::InitializeModifiers::EnableANSI();

	httplib::Client downloader("https://file120.gofile.io");

	downloader.set_follow_location(false);

	std::ofstream DownloadFile(L"GAMMA RC3.7z", std::ios::binary | std::ios::trunc);

	downloader.set_logger(&LoggingFunction);

	downloader.set_default_headers({
		{"Cookie", "accountToken=0B5OUiFDrjWBrtP7aJI9HLWtMwV7jyb8"},
		{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

	downloader.set_keep_alive(true);
	httplib::Result res = downloader.Get("/download/86137f5a-201e-41ff-86b7-47fa42ba2d11/GAMMA%20RC3.7z",
		[&](const char* data, size_t data_length)
		{
			DownloadFile.write(data, data_length);
			return true;
		},
		[](uint64_t len, uint64_t total)
		{
			if (len%487 != 0)
			{
				return true; /* don't print all the time */
			}

			wprintf(L"%lld / %lld bytes => %d%% complete\n",
			len, total,
			(int)(len * 100 / total));
			return true; // return 'false' if you want to cancel the request.
		});

	PrintServerResponse(res);

	DownloadFile.close();

	wprintf(L"Press any button to continue"); _getch();
    return 0;
}