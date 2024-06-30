#pragma once

#include <QProgressBar>
#include <QObject>

#include <mutex>
#include <atomic>

/* Each class instance is a thread */
class ModProcessorThread : public QObject
{
	Q_OBJECT
signals:
	void ModUpdateProgress(const int&);

public:
	void UpdateModProgress(const int& value)
	{
		emit ModUpdateProgress(value);
	}

protected:
	inline static std::atomic<int> ModCount = 0;
	inline static std::atomic<int> CompleteCount = 0;

	QProgressBar* RegisteredProgressBar = nullptr;

public:
	ModProcessorThread();
	~ModProcessorThread();

	void ProcessMod();
};