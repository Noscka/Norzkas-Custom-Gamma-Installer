#pragma once

#include <QString>
#include <vector>
#include <NosLib/String.hpp>

#include <filesystem>

namespace Validation
{
	/* directories which can be used to identify a stalker folder */
	static inline std::vector<QString> stalker_sub_directories({ "appdata\\", "bin\\", "db\\", "gamedata\\", "tools\\" });

	inline bool ValidateStalkerAnomalyPath(const QString& q_path)
	{
#ifdef _DEBUG
		return true;
#endif // _DEBUG

		std::filesystem::path path(q_path.toStdWString());

		if (normalizedPath.back() != L'/' || normalizedPath.back() != L'\\')
		{
			normalizedPath.append(L"/");
		}

		/* IF the path doesn't exist */
		if (!std::filesystem::exists(normalizedPath))
		{
			return false;
		}

		/* go through all the subdirectories that Stalker Anomaly should have */
		for (std::wstring subdirectory : stalker_sub_directories)
		{
			/* IF the path doesn't exist */
			if (!std::filesystem::exists(normalizedPath + subdirectory))
			{
				return false;
			}
		}

		return true;
	}
}