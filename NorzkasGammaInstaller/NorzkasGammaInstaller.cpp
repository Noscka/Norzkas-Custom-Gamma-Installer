#define CPPHTTPLIB_OPENSSL_SUPPORT
#define BIT7Z_AUTO_FORMAT

#include "EXTERNAL\httplib.h"
#include "EXTERNAL\json.hpp"

#include <NosLib/Console.hpp>

#include <bit7z\bit7z.hpp>
#include <bit7z\bit7zlibrary.hpp>
#include <bit7z\bitfileextractor.hpp>

#include <windows.h>
#include <conio.h>
#include <fstream>
#include <format>
#include <filesystem>

#include "Headers\HTTPLogging.hpp"
#include "Headers\AccountToken.hpp"
#include "Headers\ModMakerParsing.hpp"
#include "Headers\HTMLParsing.h"
#include "Headers\Directories.h"
#include "Headers\HTTPOperations.hpp"

const std::wstring outputDirectory = L"mods\\";

void StandardModProcess(ModPackMaker::ModInfo* mod)
{
	httplib::Client downloadClient(mod->Link.Host);

	downloadClient.set_follow_location(false);
	downloadClient.set_keep_alive(false);
	downloadClient.set_default_headers({{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

	switch (DetermineHostType(mod->Link.Host))
	{
	case HostType::ModDB:
		ModDBDownload(&downloadClient, mod);
		break;

	case HostType::Github:
		GithubDownload(&downloadClient, mod);
		break;

	default:
		wprintf(L"Unknown host type\ncontinuing to next\n");
		return;
	}
}

void SeperatorModProcess(ModPackMaker::ModInfo* mod)
{
	createDirectoryRecursively(outputDirectory + mod->GetFullFileName());
}

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
	NosLib::Console::InitializeModifiers::EnableANSI();
	NosLib::Console::InitializeModifiers::BeatifyConsole<wchar_t>(L"Norzka's Gamma Installer");
	NosLib::Console::InitializeModifiers::InitializeEventHandler();

	NosLib::DynamicArray<ModPackMaker::ModInfo*> modInfoArray = ModPackMaker::ModpackMakerFile_Parse(L"modpack_maker_list.txt");

	wprintf(L"Press any button to continue"); _getch();
	return 0;

	for (ModPackMaker::ModInfo* mod : modInfoArray)
	{
		switch (mod->ModType)
		{
		case ModPackMaker::ModInfo::Type::Seperator:
			SeperatorModProcess(mod);
			break;

		case ModPackMaker::ModInfo::Type::Standard:
			StandardModProcess(mod);
			break;

		default:
			wprintf(L"Unknown Mod Type\ncontinuing to next\n");
			continue;
		}
	}

	wprintf(L"Press any button to continue"); _getch();
	return 0;

	/* LATER SETUP */

	AccountToken::GetAccountToken();

	httplib::Client downloader("https://file120.gofile.io");

	downloader.set_follow_location(false);
	downloader.set_logger(&LoggingFunction);
	downloader.set_keep_alive(false);
	downloader.set_default_headers({
		{"Cookie", std::format("accountToken={}", AccountToken::AccountToken)},
		{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

	GetAndSaveFile(&downloader, "/download/86137f5a-201e-41ff-86b7-47fa42ba2d11/GAMMA%20RC3.7z", "GAMMA RC3.7z");

	wprintf(L"Press any button to continue"); _getch();
    return 0;
}