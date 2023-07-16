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
#include "Headers\HTTPOperations.hpp"

const std::wstring ModDirectory = L"mods\\";
const std::wstring ExtractedDirectory = L".\\Extracted\\";

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

	std::wstring extractedOutDirectory = ExtractedDirectory + NosLib::String::ToWstring(mod->GetFullFileName(false));

	wprintf((extractedOutDirectory + L"\n").c_str());

	std::filesystem::create_directories(extractedOutDirectory);

	bit7z::BitFileExtractor extractor(bit7z::Bit7zLibrary("7z.dll"));
	try
	{
		extractor.extract(mod->GetFullFileName(true), NosLib::String::ToString(extractedOutDirectory));
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}
}

void SeperatorModProcess(ModPackMaker::ModInfo* mod)
{
	std::filesystem::create_directories(ModDirectory + NosLib::String::ToWstring(mod->GetFullFileName(false)));
}

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
	NosLib::Console::InitializeModifiers::EnableANSI();
	NosLib::Console::InitializeModifiers::BeatifyConsole<wchar_t>(L"Norzka's Gamma Installer");
	NosLib::Console::InitializeModifiers::InitializeEventHandler();

	NosLib::DynamicArray<ModPackMaker::ModInfo*> modInfoArray = ModPackMaker::ModpackMakerFile_Parse("modpack_maker_list.txt");


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

	//GetAndSaveFile(&downloader, "/download/86137f5a-201e-41ff-86b7-47fa42ba2d11/GAMMA%20RC3.7z", "GAMMA RC3.7z");

	wprintf(L"Press any button to continue"); _getch();
    return 0;
}