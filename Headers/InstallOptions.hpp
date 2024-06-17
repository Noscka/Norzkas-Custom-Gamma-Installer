#pragma once

#include <string>

namespace InstallInfo
{
	inline std::wstring ModDirectory = L"mods\\";
	inline std::wstring ExtractedDirectory = L"extracted\\";
	inline std::wstring DownloadDirectory = L"downloads\\";
}

namespace InstallOptions
{
	inline std::wstring StalkerAnomalyPath;
	inline std::wstring GammaInstallPath;

	inline bool AddOverwriteFiles = true;
}