#pragma once

#include <NosLib/DynamicArray.hpp>
#include <NosLib/Logging.hpp>

#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>
#include <QPropertyAnimation>
#include <QProgressBar>
#include <QLabel>

#include "FlowLayout.hpp"

#include <mutex>

class ProgressStatus : public QFrame
{
	Q_OBJECT

private:
	QProgressBar ProgressBar;
	QLabel StatusLabel;
	QVBoxLayout ContainerLayout;

public:
	inline ProgressStatus(QWidget* parent = nullptr) : QFrame(parent)
	{
		setFrameShape(QFrame::Shape::Box);
		setFrameShadow(QFrame::Shadow::Raised);

		ContainerLayout.setSpacing(0);
		ContainerLayout.setContentsMargins(0, 0, 0, 0);

		StatusLabel.setAlignment(Qt::AlignmentFlag::AlignCenter);
		StatusLabel.setText("Thread");
		ContainerLayout.addWidget(&StatusLabel);

		ProgressBar.setMinimum(0);
		ProgressBar.setMaximum(100);
		ProgressBar.setValue(0);
		ProgressBar.setAlignment(Qt::AlignmentFlag::AlignCenter);
		ContainerLayout.addWidget(&ProgressBar);

		setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
		setLayout(&ContainerLayout);
	}

public slots:
	void UpdateProgress(const int& value)
	{
		ProgressBar.setValue(value);
	}

	void UpdateStatus(const std::wstring& value)
	{
		StatusLabel.setText(QString::fromStdWString(value));
	}
};

class MultiThreadProgress : public QWidget
{
	Q_OBJECT

private:
	QGridLayout MainLayout;
	QToolButton ToggleButton;
	QFrame HeaderLine;
	QParallelAnimationGroup ToggleAnimation;
	QWidget ContentArea;
	FlowLayout ContentLayout;
	int AnimationDuration = 300;

	inline static std::mutex registerMutex;

	NosLib::DynamicArray<ProgressStatus*> ThreadProgressBars;

private slots:
	ProgressStatus* RequestRegister();
	void RequestUnregister(ProgressStatus* progressBar);

public:
	inline MultiThreadProgress(QWidget* parent = nullptr, const QString& title = "MultiThreaded Progress") : QWidget(parent)
	{
		ToggleButton.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		ToggleButton.setArrowType(Qt::ArrowType::RightArrow);
		ToggleButton.setText(title);
		ToggleButton.setCheckable(true);
		ToggleButton.setChecked(false);

		HeaderLine.setFrameShape(QFrame::HLine);
		HeaderLine.setFrameShadow(QFrame::Sunken);
		HeaderLine.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

		ContentArea.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

		// start out collapsed
		ContentArea.setMaximumHeight(0);
		ContentArea.setMinimumHeight(0);

		// let the entire widget grow and shrink with its content
		ToggleAnimation.addAnimation(new QPropertyAnimation(this, "minimumHeight"));
		ToggleAnimation.addAnimation(new QPropertyAnimation(this, "maximumHeight"));
		ToggleAnimation.addAnimation(new QPropertyAnimation(&ContentArea, "maximumHeight"));

		// don't waste space
		MainLayout.setVerticalSpacing(0);
		MainLayout.setContentsMargins(0, 0, 0, 0);
		int row = 0;
		MainLayout.addWidget(&ToggleButton, row, 0, 1, 1, Qt::AlignLeft);
		MainLayout.addWidget(&HeaderLine, row++, 2, 1, 1);
		MainLayout.addWidget(&ContentArea, row, 0, 1, 3);
		setLayout(&MainLayout);
		QObject::connect(&ToggleButton, &QToolButton::clicked, [this](const bool checked)
		{
			UpdateAnimation();
			ToggleButton.setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
			ToggleAnimation.setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
			ToggleAnimation.start();
		});

		ContentLayout.setSpacing(5);
		ContentLayout.setContentsMargins(5, 5, 5, 5);
		ContentArea.setLayout(&ContentLayout);
	}

	ProgressStatus* RegisterProgressBar();
	void UnregisterProgressBar(ProgressStatus* progressBar);

protected:
	inline void AddWidget(QWidget* newWidget)
	{
		QLayout* contentLayout = ContentArea.layout();
		contentLayout->addWidget(newWidget);
	}

	inline void RemoveWidget(QWidget* newWidget)
	{
		QLayout* contentLayout = ContentArea.layout();
		contentLayout->addWidget(newWidget);
	}

	inline void UpdateAnimation()
	{
		QLayout* contentLayout = ContentArea.layout();
		int contentHeight = contentLayout->sizeHint().height();
		const auto collapsedHeight = sizeHint().height() - ContentArea.maximumHeight();

		for (int i = 0; i < ToggleAnimation.animationCount() - 1; ++i)
		{
			QPropertyAnimation* spoilerAnimation = static_cast<QPropertyAnimation*>(ToggleAnimation.animationAt(i));
			spoilerAnimation->setDuration(AnimationDuration);
			spoilerAnimation->setStartValue(collapsedHeight);
			spoilerAnimation->setEndValue(collapsedHeight + contentHeight);
		}

		QPropertyAnimation* contentAnimation = static_cast<QPropertyAnimation*>(ToggleAnimation.animationAt(ToggleAnimation.animationCount() - 1));
		contentAnimation->setDuration(AnimationDuration);
		contentAnimation->setStartValue(0);
		contentAnimation->setEndValue(contentHeight);
	}
};

