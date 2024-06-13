#pragma once

#include <QtWidgets>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLayout>
#include <QtWidgets/QVBoxLayout>
#include <QSpacerItem>

#include <QFileDialog>

#include <functional>

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

	QSpacerItem* MiddleVerticalSpacer;
	QLabel* ErrorLabel;

	bool ValidPath = true;
	std::function<bool(const QString&)> DirectoryValidateFunction;

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

		MiddleVerticalSpacer = new QSpacerItem(10, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
		MainGridLayout->addItem(MiddleVerticalSpacer, 2, 0);

		ErrorLabel = new QLabel(this);
		ErrorLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
		/* Add Error type color */
		MainGridLayout->addWidget(ErrorLabel, 3, 0, 1, 2);

		SetUiDefaultText();
		ConnectButtons();
	}

	protected:
		inline void SetUiDefaultText()
		{
			PathInput->setText(QCoreApplication::translate("InstallerWindow", "", nullptr));
			PathLabel->setText(QCoreApplication::translate("InstallerWindow", "Path", nullptr));
			PathBrowseButton->setText(QCoreApplication::translate("InstallerWindow", "Browse...", nullptr));
		}

		inline void ConnectButtons()
		{
			/* Browse Path */
			connect(PathBrowseButton, &QPushButton::released, this, [&]()
			{
				QString stalkerAnomalyPath = 
					QFileDialog::getExistingDirectory(this, QCoreApplication::translate("InstallerWindow", "Path", nullptr), QDir::currentPath());

				if (!stalkerAnomalyPath.isEmpty())
				{
					PathInput->setText(stalkerAnomalyPath);
				}
			});

			connect(PathInput, &QLineEdit::textChanged, this, &InnerInstallPathFrame::ValidatePath);
		}

		inline void ValidatePath(const QString& text)
		{
			if (DirectoryValidateFunction == nullptr)
			{
				ErrorLabel->setText("");
				return;
			}

			ValidPath = DirectoryValidateFunction(text);

			if (ValidPath)
			{
				ErrorLabel->setText("Directory is valid");
			}
			else
			{
				ErrorLabel->setText("Directory isn't valid\n Are you sure this it the right path?");
			}
		}

		inline void SelfValidatePath()
		{
			ValidatePath(PathInput->text());
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
		InnerInstallPathWidget->SelfValidatePath();
	}

	void SetDirectoryValidateFunction(const std::function<bool(const QString&)> directoryValidateFunction)
	{
		InnerInstallPathWidget->DirectoryValidateFunction = directoryValidateFunction;
		InnerInstallPathWidget->SelfValidatePath();
	}

	bool IsPathValid()
	{
		InnerInstallPathWidget->SelfValidatePath();
		return InnerInstallPathWidget->ValidPath;
	}

	std::wstring GetPath()
	{
		return MakeSystemPaths(InnerInstallPathWidget->PathInput->text().toStdWString());
	}

private:
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
};