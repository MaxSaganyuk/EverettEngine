#pragma once

#include "external/IEverettEngine.h"

class TimerManager
{
	struct TimedCallback : IEverettEngine::TimedCallbackSetup
	{
		std::chrono::steady_clock::time_point tickStartTime = std::chrono::steady_clock::now();
		size_t currentCallCount{};
	};

	std::forward_list<TimedCallback> timedCallbacks;

	auto DeleteTimedCallback(std::forward_list<TimedCallback>::iterator beforeIter)
	{
		return timedCallbacks.erase_after(beforeIter);
	}

public:
	void AddTimedCallback(const IEverettEngine::TimedCallbackSetup& timedCallbackSetup)
	{
		timedCallbacks.push_front(TimedCallback{ timedCallbackSetup });
	}

	void ProcessTimedCallbacks()
	{
		if (timedCallbacks.empty()) return;

		auto currentTime = std::chrono::steady_clock::now();
		auto beforeCurrent = timedCallbacks.before_begin();

		for (auto currentIter = std::next(beforeCurrent); currentIter != timedCallbacks.end();)
		{
			TimedCallback& currentCallback = *currentIter;

			if ((currentCallback.endTrigger && *currentCallback.endTrigger) ||
				(currentCallback.amountOfCalls && currentCallback.currentCallCount == *currentCallback.amountOfCalls))
			{
				currentIter = DeleteTimedCallback(beforeCurrent);
			}
			else 
			{
				if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - currentCallback.tickStartTime)
					>= currentCallback.period)
				{
					currentCallback.tickStartTime = currentTime;

					currentCallback.callback(++currentCallback.currentCallCount);
				}

				beforeCurrent = currentIter;
				currentIter = std::next(currentIter);
			}
		}
	}

	void CleanTimedCallbacks()
	{
		timedCallbacks.clear();
	}
};