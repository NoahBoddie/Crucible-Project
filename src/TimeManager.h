#pragma once

//#include "Timer.h"

#include "Resources.h"

namespace Crucible
{
	/*
	class TaskTimerHandler
	{
	public:

		static TaskTimerHandler* GetSingleton();

		void Update();

		void RegisterTimer(TaskTimer* timer);

		void RemoveTimer(TaskTimer* timer);

		void Clear();
	private:

		void CompleteRegister();
		
		void CompleteRemoval();


		std::list<TaskTimer*> activeList;
		std::list<TaskTimer*> registerList;
	};
	/*/
	class Timer;

	
	//Used to sync with how each percieves time. None is literal time, World is how the world percieves time (like slowed time)
	// and player is how the player percieves it.
	enum TimeScale : std::uint8_t
	{
		Real = 0,
		World,
		Player,
		Total
	};



	



	//The real thing this object is supposed to do is hold various pointers to information
	// update time scale, player time scale, the scaled time passed, the raw time passed,
	// The actor based time passed.

	//It also manages timer updates, incrementing the relevant timers each time it's own
	// delta increments. Maybe I'll change this. I don't fucking know.
	// In time I will change how this works, but for now it's alright because only a set amount
	// of things use this, and I'm not deleting th time manager
	class TimeManager
	{
	private:
		//using Timer = timer_test_hole::Timer;

		//Unlike how you would normally do, these will always exist, but will be deleted when C-data disappears.
		// Basically,
		using LocalTimerMap = std::map<RE::FormID, std::list<Timer*>>;


		
	public:
		
		static inline LocalTimerMap _timerContainer;
		
		static inline float			_sessionRuntime[TimeScale::Total];


		static void UpdateTimer(RE::FormID id, float delta);
		
		static void UpdateDelta(float delta);

		
		static float GetRuntime(TimeScale scale = TimeScale::Real);

		static void RegisterTimer(Timer* timer);

		static void RemoveTimer(Timer* timer);

		static void ClearAll();
		static void ClearTime();

		//0 for global, non-for
		static void ClearAt(RE::FormID id);
	
		static float GetExecutableRuntime()
		{
			return *g_runTime;

		}

		static float GetDeltaTime() { return *g_deltaTime; }

		
	
	private:

		//void CompleteRegister();

		//void CompleteRemoval();

		//void CompleteRegister(){
		//	activeList.merge(registerList);
		//	registerList.clear();
		//}

		//void CompleteRemoval(){
		//	auto canRemove = [](TaskTimer* timer) {return !timer || !timer->isActive(); };
		//	activeList.remove_if(canRemove);
		//}
	};
	//*/
}