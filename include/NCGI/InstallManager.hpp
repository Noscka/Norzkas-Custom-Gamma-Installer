#pragma once

#include <QObject>
#include <QString>

#include "InstallOptions.hpp"

#include <fstream>
#include <chrono>
#include <mutex>

class InstallManager : public QObject
{
	Q_OBJECT

private:
	inline static InstallManager* Instance = nullptr;

signals:
	void FinishInstallerInitializing();
	void FinishInstalling(const QString&);

	void TotalUpdateProgress(const int&);

public:

	inline InstallManager(QObject* parent = nullptr) : QObject(parent)
	{}

	inline static InstallManager* GetInstallManager()
	{
		if (Instance == nullptr)
		{
			Instance = new InstallManager();
		}

		return Instance;
	}

public slots:
	void start_install();
	static void normalize_mod_list(const std::filesystem::path& mod_list_path);

protected:
	void InitializeInstaller();
	void MainInstall();

	inline void FinishInstall()
	{
		#if 0
		static wchar_t path[MAX_PATH + 1];
		SHGetSpecialFolderPath(HWND_DESKTOP, path, CSIDL_DESKTOP, FALSE);

		std::wstring targetFile = (InstallOptions::GammaInstallPath + L"ModOrganizer.exe");
		std::wstring outputFile = (std::wstring(path) + L"\\G.A.M.M.A..lnk");
		std::wstring iconFile = (InstallOptions::GammaInstallPath + L"modpack_icon.ico");

		NosLib::FileManagement::CreateFileShortcut(targetFile.c_str(), outputFile.c_str(), iconFile.c_str(), 0);
		#endif // _WIN32
	}
};