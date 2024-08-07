#pragma once

#include <QObject>
#include <QString>

#include <NosLib/FileManagement.hpp>
#include "InstallOptions.hpp"

#include "../CustomWidgets/MultiThreadProgress.hpp"

#include <fstream>
#include <chrono>
#include <mutex>

class InstallManager : public QObject
{
	Q_OBJECT

private:
	inline static InstallManager* Instance = nullptr;

	inline static std::mutex InstanceMutex;
	inline static std::mutex TotalProgressMutex;

	ProgressStatus* RegisteredStatusProgress;

signals:
	void FinishInstallerInitializing();
	void FinishInstalling(const std::wstring&);

	void TotalUpdateProgress(const int&);
	void ModUpdateProgress(const int&);
	void ModUpdateStatus(const std::wstring&);

public:
	void UpdateModProgress(const int& value)
	{
		emit ModUpdateProgress(value);
	}

	void UpdateModStatus(const std::wstring& value)
	{
		emit ModUpdateStatus(value);
	}

	MultiThreadProgress* ProgressContainer = nullptr;

	inline InstallManager(QObject* parent = nullptr) : QObject(parent)
	{}

	inline static InstallManager* GetInstallManager()
	{
		std::lock_guard<std::mutex> lk(InstanceMutex);

		if (Instance == nullptr)
		{
			Instance = new InstallManager();
		}

		return Instance;
	}

	void UpdateTotalProgress(const int& value)
	{
		std::lock_guard<std::mutex> lk(TotalProgressMutex);
		emit TotalUpdateProgress(value);
	}

public slots:
	inline void StartInstall()
	{
		auto start = std::chrono::system_clock::now();

		InitializeInstaller();
		emit FinishInstallerInitializing();

		MainInstall();

		FinishInstall();

		auto end = std::chrono::system_clock::now();
		auto elapsed = end - start;
		std::wstring timeTaken = std::vformat(L"Install Took: {:%H:%M}\n", std::make_wformat_args(elapsed));

		std::wofstream installTimeWrite(L"InstallTime.txt", std::ios::binary | std::ios::app);
		installTimeWrite.write(timeTaken.c_str(), timeTaken.size());
		installTimeWrite.close();

		emit FinishInstalling(timeTaken);
	}

	/* To fix annoying as fuck issue with some creator names having spaces */
	static void NormalizeModList(const std::wstring& modListPath)
	{
		std::wifstream modListFileRead(modListPath, std::ios::binary);

		std::wstring out;

		std::wstring line;
		while (std::getline(modListFileRead, line))
		{
			/* Normalize */
			out += NosLib::String::Reduce(line);
			out += L"\n";
		}
		modListFileRead.close();

		std::wofstream modListFileWrite(modListPath, std::ios::binary | std::ios::trunc);
		modListFileWrite.write(out.c_str(), out.size());
		modListFileWrite.close();
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