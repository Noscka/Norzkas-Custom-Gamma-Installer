#include <NCGI/ui/InstallerWindow.hpp>

InstallerWindow::InstallerWindow(QWidget* parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	setup_defaults();
	setWindowTitle(QString::fromStdString(generate_title()));
	setup_connections();
}

void InstallerWindow::setup_defaults()
{
	ui.AnomalyPathInput->SetLabelText("Stalker Anomaly Path");
	ui.AnomalyPathInput->SetInputText("C:/Games/Anomaly");
	ui.AnomalyPathInput->SetDirectoryValidateFunction(&Validation::ValidateStalkerAnomalyPath);

	ui.GammaPathInput->SetLabelText("Gamma Install Path");
	ui.GammaPathInput->SetInputText("C:/Games/Gamma");
}

void InstallerWindow::setup_connections()
{
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

	/* Install Start */
	connect(ui.StartInstallButton, &QPushButton::released, this, &InstallerWindow::PreStartInstall);

	/* Finish Install */
	connect(ui.FinishInstallButton, &QPushButton::released, this, [&]()
	{
		QCoreApplication::exit(0);
	});
}

std::string InstallerWindow::generate_title()
{
	return std::format("Norzka's Custom Gamma Installer - {}", generate_title_branch());
}

std::string InstallerWindow::generate_title_branch()
{
	std::string extras;
	if (NCGI_BRANCH != "master")
	{
		extras += std::format(" | Branch: {}", NCGI_BRANCH);
	}

	#ifdef _DEBUG
	extras += " | DEBUG";
	#endif // _DEBUG


	return std::format("v{}{}", NCGI_VERSION, extras);
}