#define CPPHTTPLIB_OPENSSL_SUPPORT
#define BIT7Z_AUTO_FORMAT
#define BIT7Z_REGEX_MATCHING

#include "EXTERNAL\httplib.h"

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

void InitializeInstaller()
{
	/* create directory for downloaded files */

	NosLib::DynamicArray<std::string> innerModOrganizerPaths;
	innerModOrganizerPaths.Append("\\");
	ModPackMaker::ModInfo modOrganizer("https://file10.gofile.io/download/86137f5a-201e-41ff-86b7-47fa42ba2d11/GAMMA%20RC3.7z", innerModOrganizerPaths, "GAMMA\\", "GAMMA RC3");

	modOrganizer.ProcessMod();

	std::filesystem::remove_all(ModPackMaker::InstallPath + ModPackMaker::DownloadedDirectory);
	std::filesystem::remove_all(ModPackMaker::InstallPath + ModPackMaker::ModDirectory);
	std::filesystem::remove_all(ModPackMaker::InstallPath + ModPackMaker::ExtractedDirectory);

	ModPackMaker::ModDirectory = L"GAMMA\\mods\\";
	ModPackMaker::ExtractedDirectory = L"GAMMA\\extracted\\";
	ModPackMaker::DownloadedDirectory = L"GAMMA\\downloads\\";

	NosLib::DynamicArray<std::string> innerInitializeDefinitionPaths;
	innerInitializeDefinitionPaths.Append("\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_data\\modlist.txt");
	innerInitializeDefinitionPaths.Append("\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_data\\modpack_maker_list.txt");
	ModPackMaker::ModInfo initializeMod("https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip", innerInitializeDefinitionPaths, ".\\", "G.A.M.M.A. modpack definition", false);

	initializeMod.ProcessMod();

	std::filesystem::create_directories(ModPackMaker::InstallPath + L"GAMMA\\profiles\\Default\\");
	std::filesystem::rename(L"modlist.txt", ModPackMaker::InstallPath + L"GAMMA\\profiles\\Default\\modlist.txt");

	NosLib::DynamicArray<std::string> innerSetupPaths;
	innerSetupPaths.Append("\\gamma_setup-main\\modpack_addons");
	ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo("https://github.com/Grokitach/gamma_setup/archive/refs/heads/main.zip", innerSetupPaths, NosLib::String::ToString(ModPackMaker::ModDirectory), "G.A.M.M.A. setup files"));

	/* parse modpack maker file, put it into global static array */
	ModPackMaker::ModInfo::ModpackMakerFile_Parse("modpack_maker_list.txt");

	NosLib::DynamicArray<std::string> innerDefinitionPaths;
	innerDefinitionPaths.Append("\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_addons");
	ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo("https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip", innerDefinitionPaths, NosLib::String::ToString(ModPackMaker::ModDirectory), "G.A.M.M.A. modpack definition"));
}

void GetInstallPath()
{
	bool gotValidPath = false;

	std::wstring installPath;

	while (!gotValidPath)
	{
		wprintf(L"Input a valid install path [will install here if left blank]: ");
		std::getline(std::wcin, installPath);

		if (installPath == L"")
		{
			installPath = L".\\";
		}

		try
		{
			std::filesystem::create_directories(installPath);
			gotValidPath = true;
		}
		catch (...) {}
	}

	if (installPath.back() != L'\\')
	{
		installPath += L"\\";
	}

	ModPackMaker::InstallPath = installPath;
}

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
	NosLib::Console::InitializeModifiers::EnableANSI();
	NosLib::Console::InitializeModifiers::BeatifyConsole<wchar_t>(L"Norzka's Gamma Installer");
	NosLib::Console::InitializeModifiers::InitializeEventHandler();

	GetInstallPath();
	InitializeInstaller();

	/* go through all mods in global static array */
	for (ModPackMaker::ModInfo* mod : ModPackMaker::ModInfo::modInfoList)
	{
		mod->ProcessMod();
	}

	wprintf(L"Press any button to continue"); _getch();
	return 0;
}