#pragma once

#include <string>
#include <filesystem>

namespace InstallInfo
{
	inline std::filesystem::path ModDirectory = "mods\\";
	inline std::filesystem::path ExtractDirectory = "extracted\\";
	inline std::filesystem::path DownloadDirectory = "downloads\\";
}

namespace InstallOptions
{
	inline std::filesystem::path StalkerAnomalyPath;
	inline std::filesystem::path GammaInstallPath;

	inline bool AddOverwriteFiles = true;
}