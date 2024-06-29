#pragma once

#include <NosLib/DynamicArray.hpp>

#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>
#include <QPropertyAnimation>
#include <QProgressBar>

class MultiThreadProgress : public QWidget
{
	Q_OBJECT
private:
	QGridLayout MainLayout;
	QToolButton ToggleButton;
	QFrame HeaderLine;
	QParallelAnimationGroup ToggleAnimation;
	QScrollArea ContentArea;
	QVBoxLayout ContentLayout;
	int animationDuration{ 300 };

	NosLib::DynamicArray<QProgressBar*> ThreadProgressBars;
public:
	inline MultiThreadProgress(QWidget* parent = nullptr, const QString& title = "ABC", const int animationDuration = 300) : QWidget(parent)
	{
		ToggleButton.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		ToggleButton.setArrowType(Qt::ArrowType::RightArrow);
		ToggleButton.setText(title);
		ToggleButton.setCheckable(true);
		ToggleButton.setChecked(false);

		HeaderLine.setFrameShape(QFrame::HLine);
		HeaderLine.setFrameShadow(QFrame::Sunken);
		HeaderLine.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

		ContentArea.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

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
			ToggleButton.setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
			ToggleAnimation.setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
			ToggleAnimation.start();
		});

		ContentLayout.setSpacing(5);
		ContentLayout.setContentsMargins(5, 5, 5, 5);
		ContentArea.setLayout(&ContentLayout);
	}

	inline QProgressBar* RegisterProgressBar()
	{
		ThreadProgressBars.Append(new QProgressBar);
		QProgressBar* newProgressBar = ThreadProgressBars[ThreadProgressBars.GetLastArrayIndex()];
		newProgressBar->setMinimum(0);
		newProgressBar->setMaximum(100);
		newProgressBar->setValue(0);
		newProgressBar->setAlignment(Qt::AlignmentFlag::AlignCenter);
		AddWidget(newProgressBar);

		return newProgressBar;
	}

	inline void UnregisterProgressBar(QProgressBar* progressBar)
	{
		RemoveWidget(progressBar);

		ThreadProgressBars.ObjectRemove(progressBar);
	}

protected:
	inline void RemoveWidget(QWidget* newWidget)
	{
		QLayout* contentLayout = ContentArea.layout();
		contentLayout->removeWidget(newWidget);
		int contentHeight = contentLayout->sizeHint().height();

		UpdateAnimation(contentHeight);
	}

	inline void AddWidget(QWidget* newWidget)
	{
		QLayout* contentLayout = ContentArea.layout();
		contentLayout->addWidget(newWidget);
		int contentHeight = contentLayout->sizeHint().height();

		UpdateAnimation(contentHeight);
	}

	inline void UpdateAnimation(const int& contentHeight)
	{
		const auto collapsedHeight = sizeHint().height() - ContentArea.maximumHeight();

		for (int i = 0; i < ToggleAnimation.animationCount() - 1; ++i)
		{
			QPropertyAnimation* spoilerAnimation = static_cast<QPropertyAnimation*>(ToggleAnimation.animationAt(i));
			spoilerAnimation->setDuration(animationDuration);
			spoilerAnimation->setStartValue(collapsedHeight);
			spoilerAnimation->setEndValue(collapsedHeight + contentHeight);
		}

		QPropertyAnimation* contentAnimation = static_cast<QPropertyAnimation*>(ToggleAnimation.animationAt(ToggleAnimation.animationCount() - 1));
		contentAnimation->setDuration(animationDuration);
		contentAnimation->setStartValue(0);
		contentAnimation->setEndValue(contentHeight);
	}
};