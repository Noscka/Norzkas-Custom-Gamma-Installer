#include "../Headers/InstallManager.hpp"

#include <NosLib/ThreadPool.hpp>

#include "../Headers/ModOrganizer.hpp"
#include "../Headers/ModInfo.hpp"
#include "../Headers/File.hpp"
#include "../Headers/ModProcessorThread.hpp"

void InstallManager::InitializeInstaller()
{
	File::SetDirectories(InstallOptions::GammaInstallPath + InstallInfo::DownloadDirectory, InstallOptions::GammaInstallPath + InstallInfo::ExtractDirectory);
	RegisteredStatusProgress = ProgressContainer->RegisterProgressBar();

	/* Set to 0 to disable the Initial set up and only download mods */
	#if 0
	connect(this, &InstallManager::ModUpdateProgress, RegisteredStatusProgress, &ProgressStatus::UpdateProgress);
	connect(this, &InstallManager::ModUpdateStatus, RegisteredStatusProgress, &ProgressStatus::UpdateStatus);

	ModInfo modOrganizer = MO::GetModOrganizerModObject();
	modOrganizer.ProcessMod(nullptr);

	MO::WriteConfigFile(InstallOptions::GammaInstallPath, InstallOptions::StalkerAnomalyPath);

	ModInfo::AddMod(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
					NosLib::DynamicArray<std::wstring>({ L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_patches" }), InstallOptions::StalkerAnomalyPath, L"G.A.M.M.A. modpack definition", true, false);

	ModInfo initializeMod(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
						  NosLib::DynamicArray<std::wstring>({ L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_data\\", L"\\Stalker_GAMMA-main\\G.A.M.M.A_definition_version.txt" }),
						  InstallOptions::GammaInstallPath + InstallInfo::ExtractDirectory, L"G.A.M.M.A. modpack definition", false);
	initializeMod.ProcessMod(nullptr);

	std::filesystem::create_directories(InstallOptions::GammaInstallPath + L"profiles\\Default\\");
	std::filesystem::rename(InstallOptions::GammaInstallPath + InstallInfo::ExtractDirectory + L"modlist.txt", InstallOptions::GammaInstallPath + L"profiles\\Default\\modlist.txt");
	std::filesystem::rename(InstallOptions::GammaInstallPath + InstallInfo::ExtractDirectory + L"modpack_icon.ico", InstallOptions::GammaInstallPath + L"modpack_icon.ico");
	NormalizeModList(InstallOptions::GammaInstallPath + L"profiles\\Default\\modlist.txt");
	#endif // 0

	ModInfo::AddMod(L"https://github.com/Grokitach/gamma_setup/archive/refs/heads/main.zip",
					NosLib::DynamicArray<std::wstring>({ L"\\gamma_setup-main\\modpack_addons" }), InstallInfo::ModDirectory, L"G.A.M.M.A. setup files", true);

	ModInfo::AddMod(L"https://github.com/Grokitach/gamma_large_files_v2/archive/refs/heads/main.zip",
					NosLib::DynamicArray<std::wstring>({ L"\\gamma_large_files_v2-main" }), InstallInfo::ModDirectory, L"Gamma Large Files", true);

				/* parse modpack maker file, put it into global static array */
	ModInfo::ModpackMakerFile_Parse(InstallOptions::GammaInstallPath + InstallInfo::ExtractDirectory + L"modpack_maker_list.txt");

	ModInfo::AddMod(L"https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
					NosLib::DynamicArray<std::wstring>({ L"\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_addons" }), InstallInfo::ModDirectory, L"G.A.M.M.A. modpack definition");

	if (InstallOptions::AddOverwriteFiles)
	{
		ModInfo::AddMod(L"https://github.com/Noscka/Norzkas-GAMMA-Overwrite/archive/refs/heads/main.zip",
						NosLib::DynamicArray<std::wstring>({ L"\\Norzkas-GAMMA-Overwrite-main\\" }), L"", L"Norzkas G.A.M.M.A. files");
	}

	ProgressContainer->UnregisterProgressBar(RegisteredStatusProgress);
}

void InstallManager::MainInstall()
{
	NosLib::ThreadPool modProcessThreadPool;

	NosLib::MemberFunctionStore<ModProcessorThread, void(ModProcessorThread::*)()> processFunction(&ModProcessorThread::ProcessMod);

	modProcessThreadPool.StartThreadPool(processFunction, false, 1.0f);
}