#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QString>

#include <NosLib/Logging.hpp>

#include "Headers/Validation.hpp"
#include "Headers/InstallManager.hpp"

#include "ui_InstallerWindow.h"

class InstallerWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui_InstallerWindow ui;

public:
	inline InstallerWindow(QWidget* parent = nullptr) : QMainWindow(parent)
	{
		ui.setupUi(this);

		ui.AnomalyPathInput->SetLabelText("Stalker Anomaly Path");
		ui.AnomalyPathInput->SetInputText("D:/Games/Anomaly");
		ui.AnomalyPathInput->SetDirectoryValidateFunction(&Validation::ValidateStalkerAnomalyPath);

		ui.GammaPathInput->SetLabelText("Gamma Install Path");
		ui.GammaPathInput->SetInputText("D:/Games/Gamma");

		StartupChecks();

		/* Bottom Buttons */
		connect(ui.BackButton, &QPushButton::released, this, [&]()
		{
			int currentIndex = ui.tabWidget->currentIndex();
			currentIndex--;
			ui.tabWidget->setCurrentIndex(currentIndex);
		});

		connect(ui.NextButton, &QPushButton::released, this, [&]()
		{
			int currentIndex = ui.tabWidget->currentIndex();
			currentIndex++;
			ui.tabWidget->setCurrentIndex(currentIndex);
		});

		connect(ui.CancelButton, &QPushButton::released, this, [&]()
		{
			QCoreApplication::exit(0);
		});

		/* Tab Widget */
		connect(ui.tabWidget, &QTabWidget::currentChanged, this, [&](int index)
		{
			BottomControlButtonChecks();
			InstallButtonCheck();
		});

		/* Options */
		connect(ui.OptionAddOverwriteFiles, &QCheckBox::checkStateChanged, this, [&](Qt::CheckState state)
		{
			InstallOptions::AddOverwriteFiles = (state == Qt::Checked);
		});

		connect(ui.OptionSaveSpace, &QCheckBox::checkStateChanged, this, [&](Qt::CheckState state)
		{
			InstallOptions::RemoveGAMMADefaultInstaller = (state == Qt::Checked);
		});

		/* Install Start */
		connect(ui.StartInstallButton, &QPushButton::released, this, &InstallerWindow::PreStartInstall);
	}

	inline ~InstallerWindow()
	{
	}

protected:
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

	/* Make Global */
	inline std::wstring MakeSystemPaths(std::wstring inPath)
	{
#ifdef _WIN32
		std::wstring prefix = LR"(\\?\)";
		wchar_t usedSeperator = L'\\';
		wchar_t wrongSeperator = L'/';
#elif unix
		std::wstring prefix = "";
		wchar_t usedSeperator = L'/';
		wchar_t wrongSeperator = L'\\';
#endif
		
		inPath.insert(0, prefix);

		if (inPath.back() != L'/' || inPath.back() != L'\\')
		{
			inPath.push_back(usedSeperator);
		}

		for (wchar_t& pathLetter : inPath)
		{
			/* if wrong seperator is used, use the right one */
			if (pathLetter == wrongSeperator)
			{
				pathLetter = usedSeperator;
			}
		}

		return inPath;
	}

	inline void PreStartInstall()
	{
		bool stalkerAnomalyPathValidity = ui.AnomalyPathInput->IsPathValid();
		bool stalkerGammaValidity = ui.GammaPathInput->IsPathValid();

		InstallOptions::StalkerAnomalyPath = MakeSystemPaths(ui.AnomalyPathInput->GetPath());

		InstallOptions::GammaInstallPath = MakeSystemPaths(ui.GammaPathInput->GetPath());

		NosLib::Logging::CreateLog<wchar_t>(std::format(L"Stalker Anomaly Path: \"{}\" | Gamma Install Path: \"{}\"",
														InstallOptions::StalkerAnomalyPath,
														InstallOptions::GammaInstallPath),
											NosLib::Logging::Severity::Debug);


		/* Invalid Paths */
		if (!(stalkerAnomalyPathValidity && stalkerGammaValidity))
		{
			NosLib::Logging::CreateLog<wchar_t>(std::format(L"Invalid Stalker Anomaly: \"{}\" or Gamma Install Path: \"{}\"",
														 InstallOptions::StalkerAnomalyPath,
														 InstallOptions::GammaInstallPath),
											 NosLib::Logging::Severity::Error);
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
		connect(InstallClass, &InstallManager::SingularUpdateProgress, this, &InstallerWindow::SingularUpdateProgress);
		connect(InstallClass, SIGNAL(UpdateStatus(const QString&)), this, SLOT(UpdateStatus(const QString&)));

		ui.TotalProgressBar->setValue(0);
		ui.TotalProgressBar->setMaximum(0);
		UpdateStatus("Initializing");

		connect(InstallThread, &QThread::started, InstallClass, &InstallManager::StartInstall);
		InstallClass->moveToThread(InstallThread);
		InstallThread->start();
	}

public slots:
	void FinishInstallerInitializing()
	{
		ui.TotalProgressBar->setValue(0);
		ui.TotalProgressBar->setMaximum(100);
		UpdateStatus("Starting Mod Installs");
	}

	void FinishInstalling()
	{
		ui.stackedWidget->setCurrentIndex(2);
	}

	void TotalUpdateProgress(const int& newProgress)
	{
		ui.TotalProgressBar->setValue(newProgress);
	}

	void SingularUpdateProgress(const int& newProgress)
	{
		ui.SingularProgressBar->setValue(newProgress);
	}

	void UpdateStatus(const QString& newStatus)
	{
		ui.StatusLabel->setText(newStatus);
	}
};