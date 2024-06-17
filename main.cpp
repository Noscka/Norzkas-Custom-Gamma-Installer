#include <QtWidgets/QApplication>
#include "InstallerWindow/InstallerWindow.hpp"

#include <NosLib/Logging.hpp>
#include <NosLib/HttpClient.hpp>

#include <conio.h>
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
	NosLib::Logging::SetVerboseLevel(NosLib::Logging::Verbose::Debug);
	NosLib::SetUserAgent("NCGI");

	QApplication app(argc, argv);
	app.setStyleSheet(GetStyleSheet());
	app.setWindowIcon(QIcon(":/Icon/icon.ico"));

	InstallerWindow window;
	window.show();

	return app.exec();
}