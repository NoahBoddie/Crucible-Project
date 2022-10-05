
#include "Timer.h"
#include "TimeManager.h"
#include "Resources.h"

namespace Crucible
{

	/*
	TaskTimerHandler* TaskTimerHandler::GetSingleton()
	{
		static TaskTimerHandler singleton;
		return &singleton;
	}

	void TaskTimerHandler::Update()
	{
		//static float previousDelta = 0;

		float delta = *g_deltaTime;

		Resources::sessionRuntime += delta;


		for (auto timer = activeList.rbegin(); timer != activeList.rend(); ++timer)
			(*timer)->Update(delta);
		

		//for (int i = activeList.size(); i >= 0; i++)
		//{auto timer = activeList[i];
		//	timer.Update(delta);}

		CompleteRegister();
		//CompleteRemoval();
	}

	void TaskTimerHandler::RegisterTimer(TaskTimer* timer)
	{
		registerList.push_back(timer);
	}

	void TaskTimerHandler::RemoveTimer(TaskTimer* timer)
	{
		activeList.remove(timer);
	}

	void  TaskTimerHandler::Clear()
	{
		activeList.clear();
		registerList.clear();
	}

	void TaskTimerHandler::CompleteRegister()
	{
		activeList.merge(registerList);
		registerList.clear();
	}

	void TaskTimerHandler::CompleteRemoval()
	{
		auto canRemove = [](TaskTimer* timer) {return !timer || !timer->isActive(); };
		activeList.remove_if(canRemove);
	}
	//*/

	void TimeManager::UpdateDelta(float delta)
	{
		for (int i = 0; i < TimeScale::Total; i++)
		{
			//At a later point, there should be a function to handle the scale. For now, its just 3
			// copies of the same number.
			_sessionRuntime[i] += delta;
		}
		//After wards, iterate through the list.


		//CompleteRegister();

	}

	void TimeManager::UpdateTimer(RE::FormID id, float baseDelta)
	{
		std::list<Timer*>& timerEntry = _timerContainer[id];

		for (auto timer = timerEntry.rbegin(); timer != timerEntry.rend(); ++timer) {
			auto scaledDelta = baseDelta;//Use the scalar function later to augment this.
			
			(*timer)->Update(scaledDelta);
		}

		//CompleteRegister();

	}


	float TimeManager::GetRuntime(TimeScale scale)
	{
		if (scale <= 0 || scale >= TimeScale::Total)
			return _sessionRuntime[TimeScale::Real];

		return _sessionRuntime[scale];
	}

	void TimeManager::RegisterTimer(Timer* timer)
	{
		//Gate this.
		auto id = timer->GetID();
		//Looks messy, but it should be safe.
		//registerList.push_back(timer);//Here to remember I used registering lists, they are abandoned for now, but should the need arrise.
		_timerContainer[id].push_back(timer);
	}

	void TimeManager::RemoveTimer(Timer* timer)
	{
		//In the future, I want this to check so it doesn't open a record where there is none.
		auto id = timer->GetID();
		_timerContainer[id].remove(timer);
		//activeList.remove(timer);
	}

	void TimeManager::ClearAll()
	{
		_timerContainer.clear();
		ClearTime();
	}

	void TimeManager::ClearTime()
	{
		for (int i = 0; i < (int)TimeScale::Total; i++){
			_sessionRuntime[i] = 0;
		}
	}

	//0 for global, anything else for an object.
	void TimeManager::ClearAt(RE::FormID id)
	{
		_timerContainer.erase(id);
	}
}