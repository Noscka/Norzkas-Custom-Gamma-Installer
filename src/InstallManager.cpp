#include "NCGI/InstallManager.hpp"

#include <NosLib/ThreadPool.hpp>

#include "NCGI/ModOrganizer.hpp"
#include "NCGI/ModInfo.hpp"
#include "NCGI/ModFile.hpp"
#include <filesystem>

void InstallManager::start_install()
{
	auto start = std::chrono::system_clock::now();

	InitializeInstaller();
	emit FinishInstallerInitializing();

	MainInstall();

	FinishInstall();

	auto end = std::chrono::system_clock::now();
	auto elapsed = end - start;
	std::string timeTaken = std::format("Install Took: {:%H:%M}\n", elapsed);

	emit FinishInstalling(QString::fromStdString(timeTaken));
}

void normalize_mod_list(const std::filesystem::path& mod_list_path)
{
	std::ifstream list_read_stream(mod_list_path, std::ios::binary);

	std::string out;
	std::string line;
	while (std::getline(list_read_stream, line))
	{
		out += NosLib::String::Reduce(line);
		out += "\n";
	}
	list_read_stream.close();

	std::ofstream mod_write_stream(mod_list_path, std::ios::binary | std::ios::trunc);
	mod_write_stream.write(out.c_str(), out.size());
	mod_write_stream.close();
}

void InstallManager::InitializeInstaller()
{
	ModFile::set_directories(InstallOptions::GammaInstallPath / InstallInfo::DownloadDirectory, InstallOptions::GammaInstallPath / InstallInfo::ExtractDirectory);
	//RegisteredStatusProgress = ProgressContainer->RegisterProgressBar();

	/* Set to 0 to disable the Initial set up and only download mods */
	#if 1
	//connect(this, &InstallManager::ModUpdateProgress, RegisteredStatusProgress, &ProgressStatus::UpdateProgress);
	//connect(this, &InstallManager::ModUpdateStatus, RegisteredStatusProgress, &ProgressStatus::UpdateStatus);

	ModInfo modOrganizer = MO::GetModOrganizerModObject();
	modOrganizer.process_mod();

	MO::WriteConfigFile(InstallOptions::GammaInstallPath, InstallOptions::StalkerAnomalyPath);

	ModInfo::add_mod("https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
					std::vector<std::filesystem::path>({ "\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_patches" }), InstallOptions::StalkerAnomalyPath.string(), "G.A.M.M.A. modpack definition", false);

	ModInfo initializeMod("https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
						  std::vector<std::filesystem::path>({ "\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_data\\", "\\Stalker_GAMMA-main\\G.A.M.M.A_definition_version.txt" }),
						  (InstallOptions::GammaInstallPath / InstallInfo::ExtractDirectory).string(), "G.A.M.M.A. modpack definition", false);
	initializeMod.process_mod();

	std::filesystem::create_directories(InstallOptions::GammaInstallPath / "profiles\\Default\\");
	std::filesystem::rename(InstallOptions::GammaInstallPath / InstallInfo::ExtractDirectory / "modlist.txt", InstallOptions::GammaInstallPath / "profiles\\Default\\modlist.txt");
	std::filesystem::rename(InstallOptions::GammaInstallPath / InstallInfo::ExtractDirectory / "modpack_icon.ico", InstallOptions::GammaInstallPath / "modpack_icon.ico");
	normalize_mod_list(InstallOptions::GammaInstallPath / "profiles\\Default\\modlist.txt");
	#endif // 0

	ModInfo::add_mod("https://github.com/Grokitach/gamma_setup/archive/refs/heads/main.zip",
					std::vector<std::filesystem::path>({ "\\gamma_setup-main\\modpack_addons" }), InstallInfo::ModDirectory.string(), "G.A.M.M.A. setup files", true);

	ModInfo::add_mod("https://github.com/Grokitach/gamma_large_files_v2/archive/refs/heads/main.zip",
					std::vector<std::filesystem::path>({ "\\gamma_large_files_v2-main" }), InstallInfo::ModDirectory.string(), "Gamma Large Files", true);

				/* parse modpack maker file, put it into global static array */
	ModInfo::parse_modpack_file(InstallOptions::GammaInstallPath / InstallInfo::ExtractDirectory / "modpack_maker_list.txt");

	ModInfo::add_mod("https://github.com/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip",
					std::vector<std::filesystem::path>({ "\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_addons" }), InstallInfo::ModDirectory.string(), "G.A.M.M.A. modpack definition");

	if (InstallOptions::AddOverwriteFiles)
	{
		ModInfo::add_mod("https://github.com/Noscka/Norzkas-GAMMA-Overwrite/archive/refs/heads/main.zip",
						std::vector<std::filesystem::path>({ "\\Norzkas-GAMMA-Overwrite-main\\" }), "", "Norzkas G.A.M.M.A. files");
	}

	//ProgressContainer->UnregisterProgressBar(RegisteredStatusProgress);
}

void InstallManager::MainInstall()
{
	/* loop through mods */
}