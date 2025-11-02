#include <QtWidgets/QApplication>
#include <QFile>
#include <NCGI/ui/InstallerWindow.hpp>

#include <NosLib/Logging.hpp>
#include <NosLib/Http/HttpClient.hpp>

#include <fstream>
#include <format>

QString GetStyleSheet()
{
	QFile manjaroMix(":/Theme/vs15 Dark-Red.qss");
	manjaroMix.open(QFile::ReadOnly | QFile::Text);
	QTextStream manjaroMixTs(&manjaroMix);
	return manjaroMixTs.readAll();
}

int main(int argc, char* argv[])
{
	/* Stops system from going to idle sleep */
	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);

	NosLib::Logging::SetVerboseLevel(NosLib::Logging::Verbose::Debug);

	QApplication app(argc, argv);
	app.setStyleSheet(GetStyleSheet());
	app.setWindowIcon(QIcon(":/Icon/icon.ico"));

	InstallerWindow window;
	window.show();

	/* Allows Idle sleep again */
	SetThreadExecutionState(ES_CONTINUOUS);
	return app.exec();
}