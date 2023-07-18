#define CPPHTTPLIB_OPENSSL_SUPPORT
#define BIT7Z_AUTO_FORMAT
#define BIT7Z_REGEX_MATCHING

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

void copyIfExists(const std::wstring& from, const std::wstring& to)
{
	/* if DOESN'T exist, go to next path (this is to remove 1 layer of nesting */
	if (!std::filesystem::exists(from))
	{
		return;
	}
	
	/* repeat the previous step but this time with "fomod" sub directory */
	std::filesystem::create_directories(to);
	std::filesystem::copy(from, to, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
}

void InitializeInstaller()
{
	/* create directory for downloaded files */
	std::filesystem::create_directories(ModPackMaker::DownloadedDirectory);
	std::filesystem::create_directories(ModPackMaker::ModDirectory);

	NosLib::DynamicArray<std::string> innerDefinitionPaths2;
	innerDefinitionPaths2.Append("\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_data\\modlist.txt");
	innerDefinitionPaths2.Append("\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_data\\modpack_maker_list.txt");
	ModPackMaker::ModInfo initiazizeMod("https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip", innerDefinitionPaths2, ".\\", "G.A.M.M.A. modpack definition");

	initiazizeMod.ProcessMod();

	NosLib::DynamicArray<std::string> innerSetupPaths;
	innerSetupPaths.Append("\\gamma_setup-main\\modpack_addons");
	ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo("https://github.com/Grokitach/gamma_setup/archive/refs/heads/main.zip", innerSetupPaths, NosLib::String::ToString(ModPackMaker::ModDirectory), "G.A.M.M.A. setup files"));

	/* parse modpack maker file, put it into global static array */
	ModPackMaker::ModInfo::ModpackMakerFile_Parse("modpack_maker_list.txt");

	NosLib::DynamicArray<std::string> innerDefinitionPaths;
	innerDefinitionPaths.Append("\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_addons");
	ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo("https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip", innerDefinitionPaths, NosLib::String::ToString(ModPackMaker::ModDirectory), "G.A.M.M.A. modpack definition"));
}

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
	NosLib::Console::InitializeModifiers::EnableANSI();
	NosLib::Console::InitializeModifiers::BeatifyConsole<wchar_t>(L"Norzka's Gamma Installer");
	NosLib::Console::InitializeModifiers::InitializeEventHandler();

	InitializeInstaller();

	/* go through all mods in global static array */
	for (ModPackMaker::ModInfo* mod : ModPackMaker::ModInfo::modInfoList)
	{
		mod->ProcessMod();
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