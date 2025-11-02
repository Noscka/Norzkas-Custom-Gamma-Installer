#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QString>

#include <NosLib/Logging.hpp>

#include <NCGI/Validation.hpp>
#include <NCGI/InstallManager.hpp>
#include <NCGI/Version.hpp>

#include "ui_InstallerWindow.h"

class InstallerWindow : public QMainWindow
{
	Q_OBJECT

private:
	Ui_InstallerWindow ui;

public:
	InstallerWindow(QWidget* parent = nullptr);

protected:
	void setup_defaults();
	void setup_connections();

	std::string generate_title();
	std::string generate_title_branch();

	inline void StartupChecks()
	{
		BottomControlButtonChecks();
		InstallButtonCheck();
	}

	inline void BottomControlButtonChecks()
	{
		int currentTabbedIndex = ui.tabWidget->currentIndex();

		ui.BackButton->setEnabled(currentTabbedIndex > 0);
		ui.NextButton->setEnabled(currentTabbedIndex < 2);
	}

	inline void InstallButtonCheck()
	{
		bool stalkerAnomalyPathValidity = ui.AnomalyPathInput->IsPathValid();
		bool stalkerGammaValidity = ui.GammaPathInput->IsPathValid();

		ui.StartInstallButton->setEnabled(stalkerAnomalyPathValidity && stalkerGammaValidity);
	}

	inline void PreStartInstall()
	{
		bool stalkerAnomalyPathValidity = ui.AnomalyPathInput->IsPathValid();
		bool stalkerGammaValidity = ui.GammaPathInput->IsPathValid();

		InstallOptions::StalkerAnomalyPath = ui.AnomalyPathInput->GetPath();

		InstallOptions::GammaInstallPath = ui.GammaPathInput->GetPath();

		NosLib::Logging::CreateLog(NosLib::Logging::Severity::Info, "Stalker Anomaly Path: \"{}\" | Gamma Install Path: \"{}\"", InstallOptions::StalkerAnomalyPath.string(), InstallOptions::GammaInstallPath.string());


/* Invalid Paths */
		if (!(stalkerAnomalyPathValidity && stalkerGammaValidity))
		{
			NosLib::Logging::CreateLog(NosLib::Logging::Severity::Error, "Invalid Stalker Anomaly: \"{}\" or Gamma Install Path: \"{}\"", InstallOptions::StalkerAnomalyPath.string(), InstallOptions::GammaInstallPath.string());
			return;
		}

		StartInstall();
	}

	QThread* InstallThread;

	inline void StartInstall()
	{
		ui.stackedWidget->setCurrentIndex(1); /* Go to next Index */

		InstallThread = new QThread;
		InstallManager* InstallClass = InstallManager::GetInstallManager();

		connect(InstallClass, &InstallManager::FinishInstallerInitializing, this, &InstallerWindow::FinishInstallerInitializing);
		connect(InstallClass, &InstallManager::FinishInstalling, this, &InstallerWindow::FinishInstalling);

		connect(InstallClass, &InstallManager::TotalUpdateProgress, this, &InstallerWindow::TotalUpdateProgress);

		ui.TotalProgressBar->setValue(0);
		ui.TotalProgressBar->setMaximum(0);

		connect(InstallThread, &QThread::started, InstallClass, &InstallManager::start_install);
		InstallClass->moveToThread(InstallThread);
		InstallThread->start();
	}

public slots:
	void FinishInstallerInitializing()
	{
		ui.TotalProgressBar->setValue(0);
		ui.TotalProgressBar->setMaximum(100);
	}

	void FinishInstalling(const QString& timeTakenString)
	{
		ui.stackedWidget->setCurrentIndex(2);
		ui.TimeTakenLabel->setText(timeTakenString);
	}

	void TotalUpdateProgress(const int& newProgress)
	{
		ui.TotalProgressBar->setValue(newProgress);
	}
};