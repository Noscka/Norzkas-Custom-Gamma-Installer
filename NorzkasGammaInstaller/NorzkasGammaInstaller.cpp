#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "EXTERNAL/httplib.h"
#include "EXTERNAL/json.hpp"

#include <NosLib/Console.hpp>

#include <windows.h>
#include <conio.h>
#include <fstream>
#include <format>

#include "Headers/Global.hpp"
#include "Headers/AccountToken.hpp"
#include "Headers/ModMakerParsing.hpp"
#include "Headers\HTMLParsing.h"

void InitializeClient(httplib::Client* client)
{
	client->set_follow_location(false);
	client->set_logger(&LoggingFunction);
	client->set_keep_alive(true);
	client->set_default_headers({
		{"Cookie", std::format("accountToken={}", AccountToken::AccountToken)},
		{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});
}

void GetAndSaveFile(httplib::Client* client, const std::string& filepath, const std::string& filename)
{
	std::ofstream DownloadFile(filename, std::ios::binary | std::ios::trunc);

	httplib::Result res = client->Get(filepath,
		[&](const char* data, size_t data_length)
		{
			DownloadFile.write(data, data_length);
			return true;
		},
		[](uint64_t len, uint64_t total)
		{
			wprintf(L"%lld / %lld bytes => %d%% complete\n",
			len, total,
			(int)(len * 100 / total));
	return true; // return 'false' if you want to cancel the request.
		});

	PrintServerResponse(res);

	DownloadFile.close();
}

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
	NosLib::Console::InitializeModifiers::EnableANSI();

	NosLib::DynamicArray<ModPackMaker::ModInfo*> modInfoArray = ModPackMaker::ModpackMakerFile_Parse(L"modpack_maker_list.txt");
	
	for (ModPackMaker::ModInfo* mod : modInfoArray)
	{
		if(mod->Link.Host == "")
		{
			continue;
		}

		httplib::Client modDB(mod->Link.Host);

		modDB.set_follow_location(false);
		modDB.set_keep_alive(true);
		modDB.set_default_headers({{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

		httplib::Result res = modDB.Get(mod->Link.Path);

		ModDBParsing::HTMLParseReturn LinkOutput = ModDBParsing::ParseHtmlForLink(res->body);

		if (LinkOutput.Link == "No Link Found")
		{
			continue;
		}

		modDB.set_follow_location(true);
		modDB.set_logger(&LoggingFunction);

		GetAndSaveFile(&modDB, LinkOutput.Link, NosLib::String::ConvertString<char, wchar_t>((mod->OutName + L".zip")));
	}

	//for (ModPackMaker::ModInfo* mod : modInfoArray)
	//{
	//	wprintf(ModPackMaker::StringModInfo(*mod).c_str());
	//}

	wprintf(L"Press any button to continue"); _getch();
	return 0;

	/* LATER SETUP */

	AccountToken::GetAccountToken();

	httplib::Client downloader("https://file120.gofile.io");

	InitializeClient(&downloader);

	GetAndSaveFile(&downloader, "/download/86137f5a-201e-41ff-86b7-47fa42ba2d11/GAMMA%20RC3.7z", "GAMMA RC3.7z");

	wprintf(L"Press any button to continue"); _getch();
    return 0;
}