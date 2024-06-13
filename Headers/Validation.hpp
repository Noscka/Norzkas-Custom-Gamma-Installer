#pragma once

#include <QString>
#include <NosLib/DynamicArray.hpp>
#include <NosLib/String.hpp>

#include <filesystem>

namespace Validation
{
	static inline NosLib::DynamicArray<std::wstring> StalkerSubDirectories({ L"appdata\\", L"bin\\", L"db\\", L"gamedata\\", L"tools\\" });

	inline bool ValidateStalkerAnomalyPath(const QString& path)
	{
#ifdef _DEBUG
		return true;
#endif // _DEBUG

		std::wstring normalizedPath = path.toStdWString();

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
		for (std::wstring subdirectory : StalkerSubDirectories)
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