#include "../Headers/InstallManager.hpp"

#include <NosLib/ThreadPool.hpp>

#include "../Headers/ModOrganizer.hpp"
#include "../Headers/ModInfo.hpp"
#include "../Headers/File.hpp"
#include "../Headers/ModProcessorThread.hpp"

void InstallManager::InitializeInstaller()
{
	File::SetDirectories(InstallOptions::GammaInstallPath + InstallInfo::DownloadDirectory, InstallOptions::GammaInstallPath + InstallInfo::ExtractDirectory);
	
	#if 1
	ModInfo modOrganizer = MO::GetModOrganizerModObject();
	modOrganizer.ProcessMod();

	MO::WriteConfigFile(InstallOptions::GammaInstallPath, InstallOptions::StalkerAnomalyPath);

	ModInfo::modInfoList.Append(new ModInfo(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
								   NosLib::DynamicArray<std::wstring>({ L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_patches" }), InstallOptions::StalkerAnomalyPath, L"G.A.M.M.A. modpack definition", false));

	ModInfo initializeMod(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
										NosLib::DynamicArray<std::wstring>({ L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_data\\", L"\\Stalker_GAMMA-main\\G.A.M.M.A_definition_version.txt" }),
										InstallOptions::GammaInstallPath + InstallInfo::ExtractDirectory, L"G.A.M.M.A. modpack definition", false);
	initializeMod.ProcessMod();

	std::filesystem::create_directories(InstallOptions::GammaInstallPath + L"profiles\\Default\\");
	std::filesystem::rename(InstallOptions::GammaInstallPath + InstallInfo::ExtractDirectory + L"modlist.txt", InstallOptions::GammaInstallPath + L"profiles\\Default\\modlist.txt");
	std::filesystem::rename(InstallOptions::GammaInstallPath + InstallInfo::ExtractDirectory + L"modpack_icon.ico", InstallOptions::GammaInstallPath + L"modpack_icon.ico");
	NormalizeModList(InstallOptions::GammaInstallPath + L"profiles\\Default\\modlist.txt");
	#endif // 0

	ModInfo::modInfoList.Append(new ModInfo(L"https://github.com/Grokitach/gamma_setup/archive/refs/heads/main.zip",
																		NosLib::DynamicArray<std::wstring>({ L"\\gamma_setup-main\\modpack_addons" }), InstallInfo::ModDirectory, L"G.A.M.M.A. setup files"));

	ModInfo::modInfoList.Append(new ModInfo(L"https://github.com/Grokitach/gamma_large_files_v2/archive/refs/heads/main.zip",
																		NosLib::DynamicArray<std::wstring>({ L"\\gamma_large_files_v2-main" }), InstallInfo::ModDirectory, L"Gamma Large Files"));

																	/* parse modpack maker file, put it into global static array */
	ModInfo::ModpackMakerFile_Parse(InstallOptions::GammaInstallPath + InstallInfo::ExtractDirectory + L"modpack_maker_list.txt");

	ModInfo::modInfoList.Append(new ModInfo(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
																		NosLib::DynamicArray<std::wstring>({ L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_addons" }), InstallInfo::ModDirectory, L"G.A.M.M.A. modpack definition"));

	if (InstallOptions::AddOverwriteFiles)
	{
		ModInfo::modInfoList.Append(new ModInfo(L"https://github.com/Noscka/Norzkas-GAMMA-Overwrite/archive/refs/heads/main.zip",
																			NosLib::DynamicArray<std::wstring>({ L"\\Norzkas-GAMMA-Overwrite-main\\" }), L"", L"Norzkas G.A.M.M.A. files"));
	}
}

void InstallManager::MainInstall()
{
	NosLib::ThreadPool modProcessThreadPool;

	NosLib::MemberFunctionStore<ModProcessorThread, void(ModProcessorThread::*)()> processFunction(&ModProcessorThread::ProcessMod);

	modProcessThreadPool.StartThreadPool(processFunction, false);
}