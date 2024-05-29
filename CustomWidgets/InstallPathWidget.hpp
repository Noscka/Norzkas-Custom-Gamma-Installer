#pragma once

#include <QtWidgets>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLayout>
#include <QtWidgets/QVBoxLayout>
#include <QSpacerItem>

class InstallPathWidget;

/*
 Don't use, use InstallPathWidget
 */
class InnerInstallPathFrame : public QFrame
{
	Q_OBJECT

	friend InstallPathWidget;
protected:
	QGridLayout* MainGridLayout;

	QLineEdit* PathInput;
	QLabel* PathLabel;
	QPushButton* PathBrowseButton;

public:
	inline InnerInstallPathFrame(QWidget* parent = nullptr) : QFrame(parent)
	{
		QSizePolicy sizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		setSizePolicy(sizePolicy);
		setFrameShape(QFrame::Shape::StyledPanel);

		MainGridLayout = new QGridLayout(this);
		MainGridLayout->setSpacing(6);
		MainGridLayout->setContentsMargins(11, 11, 11, 11);
		PathInput = new QLineEdit(this);

		MainGridLayout->addWidget(PathInput, 1, 0, 1, 1);

		PathLabel = new QLabel(this);
		QSizePolicy sizePolicy1(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Maximum);
		sizePolicy1.setHorizontalStretch(0);
		sizePolicy1.setVerticalStretch(0);
		sizePolicy1.setHeightForWidth(PathLabel->sizePolicy().hasHeightForWidth());
		PathLabel->setSizePolicy(sizePolicy1);

		MainGridLayout->addWidget(PathLabel, 0, 0, 1, 1);

		PathBrowseButton = new QPushButton(this);

		MainGridLayout->addWidget(PathBrowseButton, 1, 1, 1, 1);

		SetUiDefaultText();
	}

	protected:
		inline void SetUiDefaultText()
		{
			PathInput->setText(QCoreApplication::translate("InstallerWindow", "", nullptr));
			PathLabel->setText(QCoreApplication::translate("InstallerWindow", "Path", nullptr));
			PathBrowseButton->setText(QCoreApplication::translate("InstallerWindow", "Browse...", nullptr));
		}
	};

/*
 Widget made to gather install path Info
 */
class InstallPathWidget : public QWidget
{
	Q_OBJECT

private:
	QVBoxLayout* MainVerticalLayout;
	QSpacerItem* TopVerticalSpacer;
	QSpacerItem* BottomVerticalSpacer;

	InnerInstallPathFrame* InnerInstallPathWidget;

public:
	inline InstallPathWidget(QWidget* parent = nullptr) : QWidget(parent)
	{
		MainVerticalLayout = new QVBoxLayout(this);
		MainVerticalLayout->setSpacing(0);
		MainVerticalLayout->setContentsMargins(10, 10, 10, 10);

		TopVerticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
		MainVerticalLayout->addItem(TopVerticalSpacer);

		InnerInstallPathWidget = new InnerInstallPathFrame(this);
		MainVerticalLayout->addWidget(InnerInstallPathWidget);

		BottomVerticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
		MainVerticalLayout->addItem(BottomVerticalSpacer);
	}

	void SetLabelText(const QString& newText)
	{
		InnerInstallPathWidget->PathLabel->setText(newText);
	}

	void SetInputText(const QString& newText)
	{
		InnerInstallPathWidget->PathInput->setText(newText);
	}
};