#pragma once

#include <QString>
#include <vector>
#include <NosLib/String.hpp>
#include <NosLib/ErrorHandling.hpp>
#include <cstdint>
#include <filesystem>

namespace Validation
{
	/* directories which can be used to identify a stalker folder */
	static inline std::vector<std::string> stalker_sub_directories({ "appdata", "bin", "db", "gamedata", "tools" });

	bool ValidateStalkerAnomalyPath(const QString& q_path);
	NosLib::Result<uint64_t> GetFolderFreeSpace(const QString& folder);
}