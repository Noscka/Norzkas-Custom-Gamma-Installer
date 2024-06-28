#pragma once

#include <mutex>
#include <atomic>

/* Each class instance is a thread */
class ModProcessorThread
{
protected:
	inline static std::atomic<int> ModCount = 0;
	inline static std::atomic<int> CompleteCount = 0;

public:
	ModProcessorThread() = default;

	void ProcessMod();
};