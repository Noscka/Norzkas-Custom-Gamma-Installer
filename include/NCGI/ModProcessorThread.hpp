#pragma once

#include <QProgressBar>
#include <QObject>

#include <mutex>
#include <atomic>

class ProgressStatus;

/* Each class instance is a thread */
class ModProcessorThread : public QObject
{
	Q_OBJECT
signals:
	void ModUpdateProgress(const int&);
	void ModUpdateStatus(const std::wstring&);

public:
	void UpdateModProgress(const int& value)
	{
		emit ModUpdateProgress(value);
	}

	void UpdateModStatus(const std::wstring& value)
	{
		emit ModUpdateStatus(value);
	}

protected:
	inline static std::atomic<int> ModCount = 0;
	inline static std::atomic<int> CompleteCount = 0;

	ProgressStatus* RegisteredStatusProgress = nullptr;

public:
	ModProcessorThread();
	~ModProcessorThread();

	void ProcessMod();
};