#pragma once

#include <QtWidgets/QMainWindow>
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
		ui.AnomalyPathInput->SetInputText("C:/Games/Anomaly");

		ui.GammaPathInput->SetLabelText("Gamma Install Path");
		ui.GammaPathInput->SetInputText("C:/Games/Gamma");
	}

	inline ~InstallerWindow()
	{
	}
};