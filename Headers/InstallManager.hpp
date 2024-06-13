#pragma once

#include <QObject>
#include <QString>

#include <NosLib/FileManagement.hpp>
#include "InstallOptions.hpp"

class InstallManager : public QObject
{
	Q_OBJECT

private:
	inline static InstallManager* Instance = nullptr;

signals:
	void FinishInstallerInitializing();
	void FinishInstalling();

	void TotalUpdateProgress(const int&);
	void SingularUpdateProgress(const int&);
	void UpdateStatus(const QString&);

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

	void UpdateTotalProgress(const int& value)
	{
		emit TotalUpdateProgress(value);
	}

	void UpdateSingularProgress(const int& value)
	{
		emit SingularUpdateProgress(value);
	}

	void UpdateStatus(const std::wstring& value)
	{
		emit UpdateStatus(QString::fromStdWString(value));
	}

public slots:
	inline void StartInstall()
	{
		InitializeInstaller();
		emit FinishInstallerInitializing();

		MainInstall();
		emit FinishInstalling();

		FinishInstall();
	}

protected:
	void InitializeInstaller();
	void MainInstall();

	inline void FinishInstall()
	{
		#ifdef _WIN32
static wchar_t path[MAX_PATH + 1];
		SHGetSpecialFolderPath(HWND_DESKTOP, path, CSIDL_DESKTOP, FALSE);

		std::wstring targetFile = (InstallOptions::GammaInstallPath + L"ModOrganizer.exe");
		std::wstring outputFile = (std::wstring(path) + L"\\G.A.M.M.A..lnk");
		std::wstring iconFile = (InstallOptions::GammaInstallPath + L"modpack_icon.ico");

		NosLib::FileManagement::CreateFileShortcut(targetFile.c_str(), outputFile.c_str(), iconFile.c_str(), 0);
		#endif // _WIN32
	}
};