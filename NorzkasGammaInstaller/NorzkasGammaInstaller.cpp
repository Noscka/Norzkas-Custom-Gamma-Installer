﻿#define CPPHTTPLIB_OPENSSL_SUPPORT
#define BIT7Z_AUTO_FORMAT
#define BIT7Z_REGEX_MATCHING
#define BIT7Z_GENERATE_PIC
#define BIT7Z_STATIC_RUNTIME
#define BIT7Z_USE_NATIVE_STRING

#include "EXTERNAL\httplib.h"

#include <NosLib\Console.hpp>
#include <NosLib\DynamicMenuSystem.hpp>
#include <NosLib\DynamicLoadingScreen.hpp>
#include <NosLib\FileManagement.hpp>

#include <bit7z\bit7z.hpp>
#include <bit7z\bit7zlibrary.hpp>
#include <bit7z\bitfileextractor.hpp>

#include <windows.h>
#include <conio.h>
#include <fstream>
#include <format>

#include "Headers\ModMakerParsing.hpp"

bool AddOverwriteFiles = true;
bool RemoveGAMMADefaultInstaller = true;

void InitializeInstaller()
{
	ModPackMaker::ModInfo modOrganizer(L"https://file10.gofile.io/download/86137f5a-201e-41ff-86b7-47fa42ba2d11/GAMMA%20RC3.7z", NosLib::DynamicArray<std::wstring>({L"\\"}), L"GAMMA\\", L"GAMMA RC3");
	modOrganizer.ProcessMod();

	if (RemoveGAMMADefaultInstaller)
	{
		std::filesystem::remove_all(ModPackMaker::InstallPath + ModPackMaker::GammaFolderName + L".Grok's Modpack Installer");
	}

	std::filesystem::remove_all(ModPackMaker::InstallPath + ModPackMaker::DownloadedDirectory);
	std::filesystem::remove_all(ModPackMaker::InstallPath + ModPackMaker::ModDirectory);
	std::filesystem::remove_all(ModPackMaker::InstallPath + ModPackMaker::ExtractedDirectory);

	/* insert the GAMMA folder name to the front of all of the directories  */
	ModPackMaker::ModDirectory.insert(0, ModPackMaker::GammaFolderName);
	ModPackMaker::ExtractedDirectory.insert(0, ModPackMaker::GammaFolderName);
	ModPackMaker::DownloadedDirectory.insert(0, ModPackMaker::GammaFolderName);

	ModPackMaker::ModInfo initializeMod(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
		NosLib::DynamicArray<std::wstring>({L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_data\\", L"\\Stalker_GAMMA-main\\G.A.M.M.A_definition_version.txt"}),
		L".\\", L"G.A.M.M.A. modpack definition", false);
	initializeMod.ProcessMod();

	std::filesystem::create_directories(ModPackMaker::InstallPath + ModPackMaker::GammaFolderName + L"profiles\\Default\\");
	std::filesystem::rename(L"modlist.txt", ModPackMaker::InstallPath + ModPackMaker::GammaFolderName + L"profiles\\Default\\modlist.txt");
	std::filesystem::rename(L"modpack_icon.ico", ModPackMaker::InstallPath + ModPackMaker::GammaFolderName + L"modpack_icon.ico");

	ModPackMaker::ModInfo patchMod(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
		NosLib::DynamicArray<std::wstring>({L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_patches"}), ModPackMaker::StalkerAnomalyPath, L"G.A.M.M.A. modpack definition", false);
	patchMod.ProcessMod();
	
	ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo(L"https://github.com/Grokitach/gamma_setup/archive/refs/heads/main.zip",
		NosLib::DynamicArray<std::wstring>({L"\\gamma_setup-main\\modpack_addons"}), ModPackMaker::ModDirectory, L"G.A.M.M.A. setup files"));

	/* parse modpack maker file, put it into global static array */
	ModPackMaker::ModInfo::ModpackMakerFile_Parse(L"modpack_maker_list.txt");
	
	ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
		NosLib::DynamicArray<std::wstring>({L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_addons"}),ModPackMaker::ModDirectory, L"G.A.M.M.A. modpack definition"));

	if (AddOverwriteFiles)
	{
		ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo(L"https://github.com/Noscka/Norzkas-GAMMA-Overwrite/archive/refs/heads/main.zip",
			NosLib::DynamicArray<std::wstring>({L".\\Norzkas-GAMMA-Overwrite-main\\"}), L".\\GAMMA\\", L"Norzkas G.A.M.M.A. files"));
	}
}

void GetInstallPath()
{
	bool gotValidPath = false;

	std::wstring installPath;

	while (!gotValidPath)
	{
		wprintf(L"Input an install path [will install here if left blank]: ");
		std::getline(std::wcin, installPath);

		if (installPath == L"")
		{
			installPath = L".\\";
		}

		if (installPath.back() != L'\\')
		{
			installPath += L"\\";
		}

		try
		{
			std::filesystem::create_directories(installPath);
			gotValidPath = true;
		}
		catch (...)
		{
			wprintf(L"Something went wrong, couldn't create directory, Please input another path\n");
		}
	}

	ModPackMaker::InstallPath = installPath;

	gotValidPath = false;
	installPath = L"";

	while (!gotValidPath)
	{
	restartAsk:
		wprintf(L"Input a Stalker Anomaly path [default: C:\\ANOMALY]: ");
		std::getline(std::wcin, installPath);

		if (installPath == L"")
		{
			installPath = L"C:\\ANOMALY";
		}

		if (installPath.back() != L'\\')
		{
			installPath += L"\\";
		}

		if (!std::filesystem::exists(installPath))
		{ /* IF the path doesn't exist */
			wprintf(L"Path doesn't exist, please input a valid path\n");
			continue;
		}

		for (std::wstring subdirectory : ModPackMaker::StalkerSubDirectories)
		{ /* go through all the subdirectories that Stalker Anomaly should have */
			if (!std::filesystem::exists(installPath + subdirectory))
			{ /* IF the path doesn't exist */
				wprintf(L"Directory isn't a valid Stalker Anomaly installation\n");
				goto restartAsk;
			}
		}

		/* if got this far, then installation must be valid */
		gotValidPath = true;
	}

	ModPackMaker::StalkerAnomalyPath = installPath;
}

void loadingScreenManager(NosLib::LoadingScreen* loadingScreenObject)
{
	ModPackMaker::ModInfo::LoadingScreenObjectPointer = loadingScreenObject;

	InitializeInstaller();

	/* go through all mods in global static array */
	for (ModPackMaker::ModInfo* mod : ModPackMaker::ModInfo::modInfoList)
	{
		mod->ProcessMod();
	}

	static wchar_t path[MAX_PATH + 1];
	SHGetSpecialFolderPath(HWND_DESKTOP, path, CSIDL_DESKTOP, FALSE);
	NosLib::FileManagement::CreateFileShortcut((ModPackMaker::InstallPath + ModPackMaker::GammaFolderName + L"ModOrganizer.exe").c_str(), (std::wstring(path) + L"\\G.A.M.M.A..lnk").c_str(), (ModPackMaker::InstallPath + ModPackMaker::GammaFolderName + L"modpack_icon.ico").c_str(), 0);
}

const std::wstring Splash =LR"(
              ███████▄        ▄████▀  ▄▀          ▄▄██████████▄   ▄▀         ▄███████████▄   ▓        ████▀ ▄█
             ▐████████       ▄██▀  ▄█▀         ▄█████████████▀ ▄██        ▄████████████▀ ▄▄██        ██▀ ▄██▀ 
             ██████████     █▀  ▄███▀        ▄██████▀▀  ▓█▀ ▄█████      ███████▀▀  ▓▀  ▄█████      ▓▀ ▄████▌  
            ▐██████████▌     ▄█████▌        ██████▀        ███████     ██████▀        ███████       ███████   
            ███████████▀    ███████        ██████▌                    ██████▌                       ██████▌   
           ▓██████ █▀  ▄█   ██████▌       ███████                    ██████▀     ▄▄▄▄▄▄▄▄▄         ███████    
           ██████▀  ▄█████ ███████       ▄████▀                     ████▀       ███████▀  ▄█      ▐██████     
          ████▀     ▐████████████▌      ▄██▀ ▄██               ▄▀  ██▀  ▄██     ▀▀██▀  ▄███       ████▀  ▄    
          █▀  ▄█▌    ▀███████████      █▀ ▄█████         █▀ ▄██▒  ▀  ▄█████         ▄█████▌      ▓█▀  ▄█▀     
           ▄████      ██████████         ███████▄        ▄████      ████████       ███████      ▀  ▄████      
        ▄██████▌       █████████          ██████████▀ ▄█████▀        ██████▀  ▄██████████       ▄██████       
     ▄█████████        ▐███████            ▀█████▀ ▄█████▀             ▀▀  ▄█████████▀▀         ███████       
  ▐▀▀▀                                           ▀▀                     ▐▀▀                                   

)";

void StartDownloadEntry(NosLib::Menu::DynamicMenu* parentMenu)
{
	parentMenu->StopMenu();
}

void Exit()
{
	NosLib::LoadingScreen::TerminateFont();
	exit(0);
}

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
	NosLib::Console::InitializeModifiers::EnableANSI();
	NosLib::Console::InitializeModifiers::BeatifyConsole<wchar_t>(L"Norzkas Custom Gamma Installer (NCGI)");
	NosLib::Console::InitializeModifiers::InitializeEventHandler();

	NosLib::LoadingScreen::InitilizeFont();

	GetInstallPath();

	NosLib::Menu::DynamicMenu setUpMenu(Splash, false, false, true);

	setUpMenu.AddMenuEntry(new NosLib::Menu::MenuEntry(L"Start Download", new NosLib::Functional::FunctionStore(&StartDownloadEntry, &setUpMenu)));
	//setUpMenu.AddMenuEntry(new NosLib::Menu::MenuEntry(L"Extra Files", &AddOverwriteFiles));
	setUpMenu.AddMenuEntry(new NosLib::Menu::MenuEntry(L"Remove GAMMA default installer (saves 9GBs)", &RemoveGAMMADefaultInstaller));
	setUpMenu.AddMenuEntry(new NosLib::Menu::MenuEntry(L"Exit", new NosLib::Functional::FunctionStore(&Exit)));
	setUpMenu.StartMenu();

	NosLib::LoadingScreen mainLoadingScreen(NosLib::LoadingScreen::LoadType::Known, Splash);
	mainLoadingScreen.StartLoading(&loadingScreenManager);

	NosLib::LoadingScreen::TerminateFont();

	wprintf(L"Press any button to continue"); _getch();
	return 0;
}