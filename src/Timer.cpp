#include "Timer.h"
#include "TimeManager.h"

namespace Crucible
{
	/*
	void TaskTimer::stop_internal()
	{
		__super::stop_internal();
		timeStored = 0;
		auto timerHandler = TaskTimerHandler::GetSingleton();
		timerHandler->RemoveTimer(this);
	}

	void TaskTimer::update_internal()
	{
		__super::update_internal();
		if (tState == TimerState::kFinished) {
			HandleCallback();

			auto timerHandler = TaskTimerHandler::GetSingleton();
			timerHandler->RemoveTimer(this);
		}
	}

	void TaskTimer::start_internal()
	{
		auto timerHandler = TaskTimerHandler::GetSingleton();

		timerHandler->RegisterTimer(this);
	}
	//*/
}