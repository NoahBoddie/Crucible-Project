#pragma once

#include "Resources.h"
#include "TimeManager.h"
#include "SerializableObject.h"
#include "SerializationTypePlayground.h"//This is the thing that breaks it.

#include "EventSystem.h"

namespace Crucible
{
	enum class TimerState {kInactive, kActive, kPaused, kFinished };



	
	class ITimerCallback
	{
	public:
		//change the name of this before you use it anymore, it can be confusing.
		virtual void	OnTimerFinish(Timer* timer_source, float total_time) = 0;
		ITimerCallback* GetNext() { return next; }
		void			SetNext(ITimerCallback* item) { next = item; }

		
		void AddCallback(ITimerCallback* item)
		{
			//I'm thinking of doing this in reverse, instead of these keeping their child they keep their parent.
			// That way, breaking out of a link is pretty easy? That wouldn't.
			// If I really need timer subscriptions I'll just make a special head object that goes first, and stores
			// other temporary callbacks.
			if (!item)
				return;

			ITimerCallback* chain = this;
			while (chain)
			{
				if (chain->next == nullptr) {
					chain->next = item;
					chain = nullptr;
				}

				chain = chain->next;
			}
		}

	private:

		ITimerCallback* next;
	};
	
	//On temporary timers
	// This timer would also need to store values for reference so maybe have something for that.
	// I think the timer should take itself a type, something to store a pointer of, and then once its finished
	// it submits that to the lambda function
	//Personally, all this describes something that task timer needs to derive from an "auto timer" or something of the like.

	
	//An object used to mark when something happens, is to be serializable.
	// it's serialized version takes the time stamp value minus current runtime, and sets that as it's
	// value. Time stamps are global objects that are not attached to the passing of time.
	struct TimeStamp : public ISerializable
	{
	private:
		//Merge init and value.
		bool _init;//I can merge init by making the starting value All bits on. Or make it infinity or something like that.

		float _value;
	
	public:
		void Start(bool safe = false)
		{
			if (safe && _init)
				return;

			auto v = TimeManager::GetRuntime();
			SetValue(v);
		}
		
		void SetValue(float new_value)
		{
			_value = new_value;
			_init = true;
		}

		void Clear() { _init = false; _value = 0; }

		float GetValue() { return _init ? _value : 0; }

		float GetTime() { return _init ? TimeManager::GetRuntime() - _value : 0; }

		bool IsActive() { return _init; }


		//*
		void OnSerialize(SerialArgument& serializer, bool& success, ISerializable* pre_serialized)
		{
			float time = -GetTime();


			//float old_value = _value;
			if (_init)
				_value = -GetTime();
			
			//RE::DebugMessageBox(std::format("Serializing Timestamp {} - {}  = {}({}), {}", TimeManager::GetRuntime(), old_value, -GetTime(), _value, _init));
		}
		//*/

		/*
		SerializableBytes GetSerialized(bool& success)
		{
			
			return;
			TimeStamp send = TimeStamp(*this);

			float time = -GetTime();

			if (time)
				send._value = time;//-GetTime();

			RE::DebugMessageBox(std::format("Serializing Timestamp {} - {}  = {}({}), {}", TimeManager::GetRuntime(), _value, time, send._value, _init));

			return send;
		}
		//*/

		//void OnDeserialize(SerialArgument& serializer, bool& success, ISerializable* pre_serialized)
		//{
			//RE::DebugMessageBox(std::format("{} - {} = {}, {}",
			//	TimeManager::GetRuntime(), _value, TimeManager::GetRuntime() - _value, _init));
		//}

		operator bool() { return _init; }

		operator float() { return _init ? GetTime() : 0; }
	};


	enum class TimerType { Trival, Local, Global };//If I can, constant would be a good timer type as well,
													// updating on an ummutable delta, with global doing it on a time scaled one.

	//When in the trival state, it works as you see here.
	// When in the global state, it has to work similar to trival. Taking a start time and comparing.
	// When it's local, instead it has to update in occordance with with a timer.
	//In order to make sure that it doesn't get updated multiple times, combat data has a friend function to
	// the task timer manager, and the task timer manager is a friend to this update function.


	//Here's how this timer will work now. Instead of each of these storing their own accumulations, 
	// each task timer will have their collective time passing by be collected.

	//Similarly, pausing will record the place in time in which they stopped just like before.

	//So basically, while global use runtime, local uses a diffferent value that is stored in the task timer handler


	//Also, this needs a deconstructor to remove itself from the list.
	class Timer final : public ISerializable
	{
	public:
		//Use these to set what a timer is supposed to be. An initializer list would be useful.
		// Also, having a second value for it would be good.
		// Something to the effect of, what it sees it's global time as.
		//Also also, an initializer list would be decent, cause as you can see, there be
		// some fuckin repeats in here.
		static Timer Trival(TimeScale scale = TimeScale::Real)
		{
			Timer timer;
			timer._type = TimerType::Trival;
			timer._scale = scale;
			return timer;
		}
		//Local timing is niche, so global time only I guess.
		static Timer Local(TimeScale scale = TimeScale::Real)
		{
			Timer timer;
			timer._type = TimerType::Local;
			timer._scale = scale;
			return timer;
		}
		static Timer Global(TimeScale scale = TimeScale::Real)
		{
			Timer timer;
			timer._type = TimerType::Global;
			timer._scale = scale;
			return timer;
		}

		//Local timers won't exist no more, forgot, the shit will just have to 
		// be bound to the update of whoever the fuck is on it. For all that, might as well
		// stick with the global, with an option of being time scaled or not
		// or having that time scale bound to the players time scale.
		// So basically, NoScale, WorldScale, PlayerScale
		//TLDR, get rid of personal time banks.
	protected:
		using timer_callback = void(Timer*, float);

		void stop_internal()
		{
			tState = TimerState::kInactive;
			timeStarted = 0;
			timePaused = 0;
			timeLimit = 0;

			//auto timerHandler = TaskTimerHandler::GetSingleton();
			//timerHandler->RemoveTimer(this);
			if (_type != TimerType::Trival)
				//In the future, remove regardless once the manager gates these,
				// if a timer gets switched out, it should be removed.
				TimeManager::RemoveTimer(this);
		}

		void check_finish()
		{
			if (tState == TimerState::kActive && GetTimeElapsed() >= timeLimit)
				tState = TimerState::kFinished;
		}
	public:
		bool isActive() { auto ts = GetState(); return ts == TimerState::kActive || ts == TimerState::kPaused; }
		bool isUpdating() { return GetState() == TimerState::kActive; }
		bool isEnabled() { auto ts = GetState(); return ts == TimerState::kActive || ts == TimerState::kFinished; }
		bool isPaused() { return GetState() == TimerState::kPaused; }
		bool isIdle() { auto ts = GetState(); return ts == TimerState::kInactive || ts == TimerState::kPaused || ts == TimerState::kFinished; }
		//this isn't always automatic, so it should be checking to see if its fullfilled.

		bool IsCallbackTimer()
		{
			//If this thing has callbacks, it gets registered on start.
			return onTimerFinish || onTimerUpdate;
		}



		float GetRuntime()
		{
			//That which wants to be returned is the same either way.
			return TimeManager::GetRuntime(_scale);
		}

		bool isFinished()
		{
			return GetState() == TimerState::kFinished;
		}

		TimerState GetState()
		{
			check_finish();
			return tState;
		}

		float GetTimeElapsed()
		{
			switch (tState)
			{
			case TimerState::kPaused:
				return timePaused - timeStarted;
			case TimerState::kInactive:
				return 0;
			default:
				return GetRuntime() - timeStarted;

			}

			//return *g_runTime - timeStarted;
		}


		void Start(float _timer)
		{
			//*
			//auto calendar = RE::Calendar::GetSingleton();

			if (isActive() || _timer <= 0)// || !calendar)
				return;



			timeStarted = GetRuntime();

			timeLimit = _timer;

			tState = TimerState::kActive;
			if (_type != TimerType::Trival)
				TimeManager::RegisterTimer(this);
		}

		void Restart(float _timer = -1)
		{
			if (isActive() == false)
				stop_internal();

			Start(_timer < 0 ? timeLimit : _timer);
		}


		void Update(float delta)//std::optional<float> delta = std::nullopt_t)
		{
			//Gate the update function, it doesn't need to happen for trival, rather it would
			// be better for it to just not.
			if (tState != TimerState::kActive || delta <= 0)
				return;

			check_finish();

			if (onTimerUpdate)
				onTimerUpdate(this, delta);
			//Interestingly enough, this goes off before it should stop updating.

			//Utility::DebugNotification(std::format("{} time elapsed", delta).c_str());

			//CONFUTE, needs to check if this is one of the update types to start sending this.
			if (tState == TimerState::kFinished) {
				TimeManager::RemoveTimer(this);

				if (onTimerFinish)
					onTimerFinish(this, GetTimeElapsed());
			}
		}


		float Stop()
		{
			if (isEnabled() == false)
				return 0.f;

			float returnValue = GetTimeElapsed();

			stop_internal();

			return returnValue;
		}

		void Pause()
		{
			if (isIdle() == true)
				return;

			timePaused = GetRuntime();

			tState = TimerState::kPaused;
		}

		void Resume()
		{
			if (isPaused() == false)
				return;

			timeStarted -= GetRuntime() - timePaused;
			timePaused = 0;

			tState = TimerState::kActive;

		}

		
		bool SetTimerFinishCallback(std::function<timer_callback> finish_callback)
		{
			//static_assert(_type != TimerType::Trival, "Cannot register callbacks on trival timers.");

			onTimerFinish = finish_callback;
			return true;
		}

		
		bool SetTimerUpdateCallback(std::function<timer_callback> update_callback)
		{
			//static_assert(_type != TimerType::Trival, "Cannot register callbacks on trival timers.");

			onTimerUpdate = update_callback;
			return true;
		}


		template<class This>
		bool SetTimerFinishCallback(This* a_this, void(This::* function)(Timer*, float) = nullptr)
		{
			//static_assert(_type != TimerType::Trival, "Cannot register callbacks on trival timers.");
			//The function will naturally look for an OnTimerUpdate function.

			if (!function)
				function = &This::OnTimerFinish;

			auto evt = [a_this, function](Timer* timer_source, float total_time)
			{
				std::invoke(function, static_cast<This*>(a_this), timer_source, total_time);
			};

			onTimerFinish = evt;

			return true;
		}

		template<class This>
		bool SetTimerUpdateCallback(This* a_this, void(This::* function)(Timer*, float) = nullptr)
		{
			//The function will naturally look for an OnTimerUpdate function.
			//static_assert(_type != TimerType::Trival, "Cannot register callbacks on trival timers.");

			if (!function)
				function = &This::OnTimerUpdate;

			auto evt = [a_this, function](Timer* timer_source, float time_update)
			{
				std::invoke(function, static_cast<This*>(a_this), timer_source, time_update);
			};

			onTimerUpdate = evt;

			return true;
		}

		template<class This>
		void AddCallback(This* a_this)
		{
			//This function is intended to be a method to add a callback
			// I want perhaps a few different overloads to this, of what it intends to add
			// for compilings sake.

			//Legacy support cause I'm fucking lazy.
			SetTimerFinishCallback(a_this);

			auto function = &This::GetID;

			//Let this not set up if its a trival timer.

			if (_type != TimerType::Local)
				return;

			RE::FormID id = std::invoke(function, a_this);//function(a_this);

			SetID(id);
		}

		void SetID(RE::FormID id)
		{
			////static_assert(_type == TimerType::Local);

			_updateID = id;
		}

		RE::FormID GetID()
		{
			if (_type == TimerType::Local)
				return _updateID;
			else
				return 0;
		}

		Timer& operator=(const Timer& a_rhs) {
			//This effectively makes it so the timer's events are always preserved.
			_type = a_rhs._type;

			_updateID = a_rhs._updateID;
			_scale = a_rhs._scale;
			tState = a_rhs.tState;

			timeStarted = a_rhs.timeStarted;
			timePaused = a_rhs.timePaused;
			timeLimit = a_rhs.timeLimit;

			return *this;
		}


		void OnSerialize(SerialArgument& serializer, bool& success, ISerializable* pre_process)
		{
			onTimerUpdate = nullptr;
			onTimerFinish = nullptr;
			timeStarted = -GetTimeElapsed();
		}

		/*
		SerializableBytes GetSerialized(bool& success)
		{
			Timer send = Timer(*this);

			send.onTimerUpdate = nullptr;
			send.onTimerFinish = nullptr;
			send.timeStarted = -GetTimeElapsed();

			return send;
		}
		//*/


		void OnDeserialize(SerialArgument& serializer, bool& success, ISerializable* pre_serialized)
		{
			auto* pre_timer = pre_serialized->As(this);

			//RE::DebugMessageBox(std::format("deserial {}", test_obj->_value));

			//Correcting the timer pointers from deserialize is super important, timers are always to be 
			// set via initialize
			
			onTimerUpdate = pre_timer->onTimerUpdate;
			onTimerFinish = pre_timer->onTimerFinish;
			RE::FormID pre = _updateID;

			//serializer.ResolveFormID(_updateID);
			serializer.Serialize(_updateID);

			//Forget why this is here, just gonna comment it out for a bit.
			//pre_timer->_updateID = _updateID;

			RE::FormID post = _updateID;
			//pre_timer
			if (isActive() == true && _type != TimerType::Trival) {
				TimeManager::RegisterTimer(pre_timer);//Register pretimer, because that's where this is going.
				RE::DebugMessageBox("Registering going to location");
			}
			
			//RE::DebugMessageBox(std::format("Timer: {}, id before: {}, after: {}",
			//	GetTimeElapsed(), pre, post));
		}

		Timer() = default;

		Timer(TimerType timer_type, TimeScale time_scale)
		{
			_type = timer_type;
			_scale = time_scale;
		}


		~Timer()
		{
			//To dissociate it's pointer if it's deleted.
			stop_internal();
		}

	protected:
		//This is moved by what it's registered to. so if it's nothing,
		// trival as it started, if local, it's above 0, if global it's zero.
		TimerType _type = TimerType::Trival;
		
		SerialFormID _updateID;
		//RE::FormID _updateID;
		
		TimeScale _scale = TimeScale::Real;//The timescale being used on. Defaults to realtime passage.
		//ScaleType _scale;// Used to get what scaled runtime is being used.
		// 
		//bool _locked;

		std::function<timer_callback> onTimerUpdate;
		std::function<timer_callback> onTimerFinish;

		TimerState tState;

		float timeStarted;
		float timePaused;
		float timeLimit;

	};


	class UpdateTimerBase : protected SimpleEventHandler<"OnTimerUpdate", "OnTimerFinish">, public ISerializable {};

	template <bool Is_Update>
	class NewTimer : public std::conditional_t<Is_Update, UpdateTimerBase, ISerializable>
	{
		//where I can, I'd like to shave this class down.
		//Simple event handling is a suggestion, I think if I really want something trival, I can turn that off. This wil bring the size down some.
	protected:
		//The state of the timer can be determined by if it's registered, and whether it has a form id.
		// A trival timer will not posess have it's simple event handler turned off. While a global will have them, but no updateID. A local one will have
		// a local update ID and it will have it's timer turned off.
		//Thus, this variable is not needed.
		TimerType _type = TimerType::Trival;

		//This variable is needed.
		SerialFormID _updateID;
		//This variable is needed, but I should consider how I implement it. I could possibly merge it with timer state, as thatonly uses 2 bits, then I can have these
		// progress from 4 for ez storage.
		TimeScale _scale = TimeScale::Real;//The timescale being used on. Defaults to realtime passage.
		
		TimerState tState;

		//ENUMS
		//-TimerState, needs 2 bits to represent 0-3
		// TimerType, needs none. can be based on context. Something that isn't bound to an update is ultimately trival. And if it's bound to a form then it's local.
		//-TimeScale, has 3 options, as such only needs 2 bits. 
		//*Total: 4 bits. They both get 2 extra, of fear for expansion.

		//floats
		// Time started. Contextually, this used to measure between now and then, to see when it's to cross over.
		// time paused. Needs to exist to measure when the time has stopped.
		// time limit, needs to exist to tell when we've hit our measured time.

		//It is possible I can get away with just 2 values, one for time remaining, and another for time paused.
		//But I could probably get away with even less.
		
		//Picture this. The timer merely has a time stamp, waiting for when the run time crosses over into that. What we then also have is when this thing is paused
		// we take the current "time left" and force that to be the value. Then, once it's unpaused we get CurrentRuntime + TimeLeft and that's how much longer it should go.
		// If we save, we just set the time to the time left.

		//Issue is, there's no way to then know what time it started, and there by no way to know how much time has elasped. Such a thing could be useful for resuming a timer.

		// Brass tacks are, the size of this object demands something larger. 
		// So, here's what I'm going to do. Timer will become a template class again. With simple timers that take up so little space they quite seriously
		// are just reverse timestamps, and active timers that update. I think I probably should just make simple timers a form of time stamp, they don't
		// need a portion of this data.

		float timeStarted;
		float timePaused;
		float timeLimit;


		//Float system. 
		//I think I can get away with a 2 float system that would get me all the information I need such as the time elasped.
		//While active the first value is the timeStarted, and the second value is the timeLimit

		//When paused, the first value becomes the time elasped, while the second value becomes the time remaining. For update timers, this isn't a big deal.

		//When it's freed from this, the first value is an inaccurate time started, that is Interpreted runtime - timeElapsed value, 
		// and the timeLimit is time elapsed + time remaining.
		//When paused and saving, nothing needs to be done.
		// When it's active what will it need to do? It will need to do the same thing it's been doing, -timeElapsed becomes value1, and timeLimit doesn't change.


		//Size sum for now?
		//SimpleEventHandler: 16 bytes, optional
		//UpdateID: 4 bytes, optional, I don't think I'll make local timers handle something like that.
		//TimerType/State, 4 bits = 1 Byte
		//tracking floats, 8 bytes.
		
		//The size of simple timers are 9 bytes, while the size of update timers are 29 bytes. Which note, is a step down from the size of function and it's
		// overhead. When functional, callpackages will take up an extra 24 bytes each, for a total of 48 bytes. But only the parts you need.

		//For the sake of stuff like action point's timers, I will make 2 classes for this.


		//In addition, I'll make a simple count down timer. This would be an alternate version of timestamp, created in the way that Timer used to be created,
		// making it with an enum that gives it a mode. With this, the time stamp will store the time it's looking forward to being past, and will return
		// will be successful if it is currently past that time. When saving, it will take that value - currentRuntime and set the value to that.
		//Generally speaking, I will be able to tell if it's not in use if it's value is NaN.





		//ON TIMERS AGAIN
		//I'm thinking I may store all timers of a certain type within a map, and have a lot of their time focused stuff happen through there. Like the registering
		// for timer update stuff. The main reason for this would be I'm thinking of creating a timer that progress when game time is progressed. This is something
		// I've long since considered. Now is probably the time to do it.
	};

	static_assert(sizeof(NewTimer<true>) < sizeof(Timer), "fnnnnnnnnnnnn");
	static_assert(sizeof(NewTimer<false>) < sizeof(Timer), "fuuuuuuuuuuuu");
}