#include <QtWidgets/QApplication>
#include "InstallerWindow/InstallerWindow.hpp"

#include <NosLib\DynamicArray.hpp>
#include <NosLib\Console.hpp>
#include <NosLib\FileManagement.hpp>

#include <bit7z\bit7z.hpp>
#include <bit7z\bit7zlibrary.hpp>
#include <bit7z\bitfileextractor.hpp>

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
		printf("Input an install path for GAMMA [will install here if left blank]: ");
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
			printf("Something went wrong, couldn't create directory, Please input another path\n");
		}
	}

	ModPackMaker::InstallPath = installPath;

	gotValidPath = false;
	installPath = L"";

	while (!gotValidPath)
	{
	restartAsk:
		printf("Input a Stalker Anomaly path [default: C:\\ANOMALY]: ");
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
			printf("Path doesn't exist, please input a valid path\n");
			continue;
		}

		/* go through all the subdirectories that Stalker Anomaly should have */
		for (std::wstring subdirectory : ModPackMaker::StalkerSubDirectories)
		{
			if (!std::filesystem::exists(installPath + subdirectory))
			{ /* IF the path doesn't exist */
				printf("Directory isn't a valid Stalker Anomaly installation\n");
				goto restartAsk;
			}
		}

		/* if got this far, then installation must be valid */
		gotValidPath = true;
	}

	ModPackMaker::StalkerAnomalyPath = installPath;
}

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	InstallerWindow w;
	w.show();
	return a.exec();

#if 0
	GetInstallPath();

	//RemoveGAMMADefaultInstaller;

	InitializeInstaller();

	/* go through all mods in global static array */
	for (ModPackMaker::ModInfo* mod : ModPackMaker::ModInfo::modInfoList)
	{
		mod->ProcessMod();
	}

	static wchar_t path[MAX_PATH + 1];
	SHGetSpecialFolderPath(HWND_DESKTOP, path, CSIDL_DESKTOP, FALSE);

	std::wstring targetFile = (ModPackMaker::InstallPath + ModPackMaker::GammaFolderName + L"ModOrganizer.exe");
	std::wstring outputFile = (std::wstring(path) + L"\\G.A.M.M.A..lnk");
	std::wstring iconFile = (ModPackMaker::InstallPath + ModPackMaker::GammaFolderName + L"modpack_icon.ico");

	NosLib::FileManagement::CreateFileShortcut(targetFile.c_str(), outputFile.c_str(), iconFile.c_str(), 0);
#endif // 0

}