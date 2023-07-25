#define CPPHTTPLIB_OPENSSL_SUPPORT
#define BIT7Z_AUTO_FORMAT
#define BIT7Z_REGEX_MATCHING
#define BIT7Z_GENERATE_PIC
#define BIT7Z_STATIC_RUNTIME
#define BIT7Z_USE_NATIVE_STRING

#include "EXTERNAL\httplib.h"

#include <NosLib\Console.hpp>
#include <NosLib\DynamicMenuSystem.hpp>
#include <NosLib\DynamicLoadingScreen.hpp>

#include <bit7z\bit7z.hpp>
#include <bit7z\bit7zlibrary.hpp>
#include <bit7z\bitfileextractor.hpp>

#include <windows.h>
#include <conio.h>
#include <fstream>
#include <format>

#include "Headers\ModMakerParsing.hpp"

bool addOverwriteFiles = true;

void InitializeInstaller(NosLib::LoadingScreen* loadingScreenObject)
{
	ModPackMaker::ModInfo::LoadingScreenObjectPointer = loadingScreenObject;
	
	ModPackMaker::ModInfo modOrganizer("https://file10.gofile.io/download/86137f5a-201e-41ff-86b7-47fa42ba2d11/GAMMA%20RC3.7z", NosLib::DynamicArray<std::string>({"\\"}), "GAMMA\\", "GAMMA RC3");
	modOrganizer.ProcessMod();

	std::filesystem::remove_all(ModPackMaker::InstallPath + ModPackMaker::DownloadedDirectory);
	std::filesystem::remove_all(ModPackMaker::InstallPath + ModPackMaker::ModDirectory);
	std::filesystem::remove_all(ModPackMaker::InstallPath + ModPackMaker::ExtractedDirectory);

	ModPackMaker::ModDirectory = L"GAMMA\\mods\\";
	ModPackMaker::ExtractedDirectory = L"GAMMA\\extracted\\";
	ModPackMaker::DownloadedDirectory = L"GAMMA\\downloads\\";

	ModPackMaker::ModInfo initializeMod("https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
		NosLib::DynamicArray<std::string>({"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_data\\modlist.txt", "\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_data\\modpack_maker_list.txt"}),
		".\\", "G.A.M.M.A. modpack definition", false);
	initializeMod.ProcessMod();

	std::filesystem::create_directories(ModPackMaker::InstallPath + L"GAMMA\\profiles\\Default\\");
	std::filesystem::rename(L"modlist.txt", ModPackMaker::InstallPath + L"GAMMA\\profiles\\Default\\modlist.txt");

	ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo("https://github.com/Grokitach/gamma_setup/archive/refs/heads/main.zip",
		NosLib::DynamicArray<std::string>({"\\gamma_setup-main\\modpack_addons"}), NosLib::String::ToString(ModPackMaker::ModDirectory), "G.A.M.M.A. setup files"));

	/* parse modpack maker file, put it into global static array */
	ModPackMaker::ModInfo::ModpackMakerFile_Parse("modpack_maker_list.txt");
	
	ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo("https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
		NosLib::DynamicArray<std::string>({"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_addons"}),NosLib::String::ToString(ModPackMaker::ModDirectory), "G.A.M.M.A. modpack definition"));

	if (addOverwriteFiles)
	{
		ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo("https://github.com/Noscka/Norzkas-GAMMA-Overwrite/archive/refs/heads/main.zip",
			NosLib::DynamicArray<std::string>({".\\Norzkas-GAMMA-Overwrite-main\\"}), ".\\GAMMA\\", "Norzkas G.A.M.M.A. files"));
	}
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

void loadingScreenManager(NosLib::LoadingScreen* loadingScreenObject)
{
	InitializeInstaller(loadingScreenObject);

	/* go through all mods in global static array */
	for (ModPackMaker::ModInfo* mod : ModPackMaker::ModInfo::modInfoList)
	{
		mod->ProcessMod();
	}
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
	//setUpMenu.AddMenuEntry(new NosLib::Menu::MenuEntry(L"Extra Files", &addOverwriteFiles));
	setUpMenu.AddMenuEntry(new NosLib::Menu::MenuEntry(L"Exit", new NosLib::Functional::FunctionStore(&Exit)));
	setUpMenu.StartMenu();

	NosLib::LoadingScreen mainLoadingScreen(NosLib::LoadingScreen::LoadType::Known, Splash);
	mainLoadingScreen.StartLoading(&loadingScreenManager);

	NosLib::LoadingScreen::TerminateFont();

	wprintf(L"Press any button to continue"); _getch();
	return 0;
}