#include <QtWidgets/QApplication>
#include "InstallerWindow/InstallerWindow.hpp"

#include <NosLib/Logging.hpp>

#include <conio.h>
#include <fstream>
#include <format>

#include "Headers\ModMakerParsing.hpp"

int main(int argc, char* argv[])
{
	NosLib::Logging::SetVerboseLevel(NosLib::Logging::Verbose::Debug);

	QApplication a(argc, argv);
	InstallerWindow w;
	w.show();
	return a.exec();
}