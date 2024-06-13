#include "InstallManager.hpp"

#include "ModMakerParsing.hpp"

void InstallManager::InitializeInstaller()
{
	/* TODO: Self Gather */
	ModPackMaker::ModInfo modOrganizer(L"https://file5.gofile.io/download/web/65cb42c0-9d71-436b-84a5-8738313443ae/GAMMA%20RC3.7z", NosLib::DynamicArray<std::wstring>({ L"\\" }), L""/* Root */, L"GAMMA RC3");
	modOrganizer.ProcessMod();

	if (InstallOptions::RemoveGAMMADefaultInstaller)
	{
		std::filesystem::remove_all(InstallOptions::GammaInstallPath + L".Grok's Modpack Installer");
	}

	ModPackMaker::ModInfo initializeMod(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
										NosLib::DynamicArray<std::wstring>({ L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_data\\", L"\\Stalker_GAMMA-main\\G.A.M.M.A_definition_version.txt" }),
										InstallOptions::GammaInstallPath + ModPackMaker::ExtractedDirectory, L"G.A.M.M.A. modpack definition", false);
	initializeMod.ProcessMod();

	std::filesystem::create_directories(InstallOptions::GammaInstallPath + L"profiles\\Default\\");
	std::filesystem::rename(InstallOptions::GammaInstallPath + ModPackMaker::ExtractedDirectory + L"modlist.txt", InstallOptions::GammaInstallPath + L"profiles\\Default\\modlist.txt");
	std::filesystem::rename(InstallOptions::GammaInstallPath + ModPackMaker::ExtractedDirectory + L"modpack_icon.ico", InstallOptions::GammaInstallPath + L"modpack_icon.ico");

	ModPackMaker::ModInfo patchMod(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
								   NosLib::DynamicArray<std::wstring>({ L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_patches" }), InstallOptions::StalkerAnomalyPath, L"G.A.M.M.A. modpack definition", false);
	patchMod.ProcessMod();

	ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo(L"https://github.com/Grokitach/gamma_setup/archive/refs/heads/main.zip",
																		NosLib::DynamicArray<std::wstring>({ L"\\gamma_setup-main\\modpack_addons" }), ModPackMaker::ModDirectory, L"G.A.M.M.A. setup files"));

																	/* parse modpack maker file, put it into global static array */
	ModPackMaker::ModInfo::ModpackMakerFile_Parse(InstallOptions::GammaInstallPath + ModPackMaker::ExtractedDirectory + L"modpack_maker_list.txt");

	ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
																		NosLib::DynamicArray<std::wstring>({ L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_addons" }), ModPackMaker::ModDirectory, L"G.A.M.M.A. modpack definition"));

	if (InstallOptions::AddOverwriteFiles)
	{
		ModPackMaker::ModInfo::modInfoList.Append(new ModPackMaker::ModInfo(L"https://github.com/Noscka/Norzkas-GAMMA-Overwrite/archive/refs/heads/main.zip",
																			NosLib::DynamicArray<std::wstring>({ L"\\Norzkas-GAMMA-Overwrite-main\\" }), L"", L"Norzkas G.A.M.M.A. files"));
	}
}

void InstallManager::MainInstall()
{
	/* go through all mods in global static array */
	for (ModPackMaker::ModInfo* mod : ModPackMaker::ModInfo::modInfoList)
	{
		mod->ProcessMod();
	}
}