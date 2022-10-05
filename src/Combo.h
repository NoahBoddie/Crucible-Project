#pragma once

#include "CombatDataBehaviour.h"
#include "Timer.h"
#include "Utility.h"
#include "EventSystem.h"
#include "EventObject.h"
#include "IRegenSubscriber.h"
#include "CompetitiveEvent.h"

namespace Crucible
{
	/// <summary>
			/// The value to determine how the combo end should be percieved.
			/// </summary>
	enum class ResolveType { kNormal, kNegative, kNegativeBonus, kPositive, kPositiveBonus, kClear };
	//Normal is normal.
	//Negative returns less gradual, and damages the return a bit. Minimum internally is half.
	//Negative bonus prevents the immediate return, causing a penalty immediately if via
	//Clear voids the combo's benefits.
	//Positive forces the gradual return to give less. I think?
	//PositiveBonus ???

	inline bool IsResolvePositive(ResolveType resolve_type) { return resolve_type == ResolveType::kPositive || resolve_type == ResolveType::kPositiveBonus; }
	inline bool IsResolveNegative(ResolveType resolve_type)
{ 
		return resolve_type == ResolveType::kNegative || resolve_type == ResolveType::kNegativeBonus || resolve_type == ResolveType::kClear;
	}


	//I would like to rework these values into something that 1, lives in their associated combo, and 2 that has object specifically
	// attached to that the combo that uses those values.
	//Like MeleeCombo::Counter::ComboCount or something. I don't know how to lock that sort of thing however, or to make it so it's functionality is
	// collective.

	enum ComboValue
	{
		usedResource = 0,
		resolveDuration = 1
	};

	enum IncrementalCounter
	{
		comboCount = 0,
		successCount = 1
	};

	enum MeleeValue
	{
		usedStamina = 0,
		powerAttackReduction = 1,
		staminaLostRate = 2,
		climbingValue = 3
	};


	enum BlockValue
	{
		actionStamina = 0,
		raiseStamina = 1,
		tempStamina = 2,
		blockDamage = 3
	};


//#define primaryBits primaryBit//2
#define secondaryBit 0b10//2
#define waitingBit 0b100//4
#define primaryBit 0b1000//8

	enum struct ComboStateFlag : std::uint8_t
	{
		//While the name is flags, to be changed at a later point, the existence of being primary should overlap being active.
		None		= 0,
		Active		= 1 << 0,
		SecondaryBit = 1 << 1,
		WaitingBit = 1 << 2,
		PrimaryBit = 1 << 3,
		Secondary	= SecondaryBit,
		Waiting		= WaitingBit | Active,
		Primary		= PrimaryBit | Active
	};



		

	class IComboResource 
	{


		//friend class ComboBase;
	};



	using Counter = std::int8_t;


	template<RE::ActorValue SUB_AV, unsigned int COUNTER_NUM, unsigned int VALUE_NUM>
	class ComboBase;

	

	static const unsigned int	k_BaseCounterNum = 0;//Currently, there's nothing all combos need to store in this regard.
	static const unsigned int	k_BaseValueNum = 1;//All combos have a primary resource to store

	constexpr std::size_t get_counter_size(unsigned int COUNTER_NUM) { return COUNTER_NUM + k_BaseCounterNum; }
	constexpr std::size_t get_value_size(unsigned int VALUE_NUM) { return VALUE_NUM + k_BaseValueNum; }

	template <RE::ActorValue AV, unsigned int COUNTER_NUM, unsigned int VALUE_NUM>
	struct ComboData
	{
		//I would like to undo the incredible amount of references to these at some point, but since it doesn't throw off the weight any,
		// optimize later.
	private:
		friend class ComboBase<AV, COUNTER_NUM, VALUE_NUM>;//I would like ComboBase to be something that only holds and manipulates counters.

		ComboStateFlag	_flags = ComboStateFlag::None;

	public:
		Counter		_counterArray[get_counter_size(COUNTER_NUM)];
		float		_valueArray[get_value_size(VALUE_NUM)];

		float _resolveDuration = 0.f;

		
	};


	class ICombo
	{
		class ComboCore
		{
			//Relink pointer would be useless here, so I will make it a serializer and serialize the string id, deserialize it and run it through the map
			ICombo* primaryCombo;

			std::unordered_map<char*, ICombo*> comboMap;
		};
	};

	//So, combo base will be a thing that holds a deal of the array stuff, just so there's some interface between data and this.
	// Then Combo will be the very base line of what's supposed to handle the counter moving stuff, from there, the template takes args
	// designating what enum types it's willing to take.
		
	//The last thing the combo base should take would be an actor value, this denotes the value that stops
	// when restoration is happening. I will hook that up later, won't be too much an issue to do.
	// I'm defaulting to stamina so I don't have to change the existing one.
	template<RE::ActorValue SUB_AV, unsigned int COUNTER_NUM, unsigned int VALUE_NUM>
	class ComboBase : 
		public IRegenSubscriber,
		public SerialComponent<ComboData<SUB_AV, COUNTER_NUM, VALUE_NUM>>
	{
	public:
		using ComboComponent = SerialComponent<ComboData<SUB_AV, COUNTER_NUM, VALUE_NUM>>;
		using Combo = ComboBase<SUB_AV, COUNTER_NUM, VALUE_NUM>;

		//I want to rework all exclusively private functions to be like_this()
	private:
		static inline const std::size_t counter_size = get_counter_size(COUNTER_NUM);
		static inline const std::size_t value_size = get_value_size(VALUE_NUM);

		std::mutex _flagLock;
	protected:
		const float		k_minResolveDuration = 0.15f;//This serves as the minimum time as well.

	private:
		
		Timer			_comboTimer = Timer::Local();

			
		ComboStateFlag&	_flags = this->ComboComponent::data._flags;

	public:
		
	protected:
		ComboData<SUB_AV, COUNTER_NUM, VALUE_NUM>& GetData(){ return this->ComboComponent::data; }

		//Counter		_counterArray[counter_size]; 
		Counter		(&_counterArray) [counter_size] = this->ComboComponent::data._counterArray;
		float		(&_valueArray) [value_size] = this->ComboComponent::data._valueArray;

		float& _usedResource = _valueArray[0];
		float& _resolveDuration = this->ComboComponent::data._resolveDuration;


	private:

		void SetActive(bool value)
		{
			std::lock_guard<std::mutex> flag_guard(_flagLock);

			if (value)
				_flags = ComboStateFlag::Active;
			else
				_flags = ComboStateFlag::None;
		}

		void SetPrimary(bool value = true)
		{
			std::lock_guard<std::mutex> flag_guard(_flagLock);

			if (value)
				_flags = ComboStateFlag::Primary;//_flags |= (ComboStateFlag)primaryBit;
			else
				//No fucking idea why &= breaks this. It actually breaks counters, I am shidding and crying rn omg
				_flags = _flags & ~ComboStateFlag::PrimaryBit;
		}

		void SetSecondary(bool value = true)
		{
			std::lock_guard<std::mutex> flag_guard(_flagLock);

			if (value)
				_flags |= ComboStateFlag::Secondary;
			else
				_flags &= ~ComboStateFlag::Secondary;
		}


		void SetWaiting(bool value = true)
		{
			std::lock_guard<std::mutex> flag_guard(_flagLock);

			if (value)
				_flags = ComboStateFlag::Waiting;//_flags |= (ComboStateFlag)primaryBit;
			else
				_flags &= ~ComboStateFlag::WaitingBit;
		}


		//FIX PLEASE. CONFUTE
		void OnStartResolve()
		{
			if (IsActive() == false && IsWaiting() == false)
				return;

				
			float resolve_timer = GetResolveDuration();
				
			_comboTimer.Start(resolve_timer);
				

			SetActive(true);//Naturally clears all states and flags but active.
		}

		void OnStopResolve()
		{
			if (IsActive() == false)
				return;

			//if (IsResolving() == true)
			//	Utility::DebugMessageBox("resolution stopped", IsPlayerRef());

			//This might just pause instead of stop outright. I can handle it from here if I change the manner it works.
			_comboTimer.Stop();
				
			SetPrimary(false);
				
			if (IsWaiting() == false)
				SetSecondary(true);

		}


		//This is fired when the timer is finished


	protected:
		virtual constexpr const char* ID() { return nullptr; }
		


		//CONFUTE, these events will need more proper gating. For now, I don't fucking care.
		//Legacy
		//virtual void OnInterrupt(EventObject& params, EventControl& control) { FinishCombo(ResolveType::kNegative); }
		virtual void OnInterrupt() { FinishCombo(ResolveType::kNegative); }

		virtual void OnSetResolveDuration(float& duration) {}

		virtual void HandleResolve(ResolveType resolve_type, bool& do_resolve) {}//This is gonna have to force a value later.

		virtual void OnCounterRelease(unsigned int index, Counter& counter) { counter = 0; }
		virtual void OnValueRelease(unsigned int index, float& value) { value = 0.f; }

		virtual void OnModifyCounter(unsigned int index, int curr_value, int& mod_value, bool& progress) {};
		virtual void OnModifyValue(unsigned int index, float curr_value, float& mod_value, bool& progress) {};

		virtual void OnComboStart() {}
		virtual void OnComboResolve(ResolveType resolve_type) = 0;
		virtual void OnComboFinish(ResolveType resolve_type, bool resolved) {}//Always fires, even when cleared



		void Initialize_INTERNAL() override
		{
			_comboTimer.AddCallback(this);
			CreateMemberEvent("OnInterrupt", &Combo::OnInterrupt);
			CreateMemberEvent("OnStopResolve", &Combo::OnStopResolve);
			CreateMemberEvent("OnStartResolve", &Combo::OnStartResolve);
			__super::Initialize_INTERNAL();
		}


		void ReleaseCounter()
		{
			int count = COUNTER_NUM + k_BaseCounterNum;

			for (int i = 0; i < count; i++) {
				OnCounterRelease(i, _counterArray[i]);
			}
		}

		void ReleaseValue()
		{
			int count = VALUE_NUM + k_BaseValueNum;

			for (int i = 0; i < count; i++) {
				OnValueRelease(i, _valueArray[i]);
			}
		}



			

	public:
		void FinishCombo(ResolveType resolve_type)
		{
			//First point of issue, this doesn't seem to be protected the below from firing off.
			if (IsActive() == false)
				return;

			_comboTimer.Stop();

			bool do_resolve = resolve_type != ResolveType::kClear;

			if (do_resolve)
			{
				if (resolve_type != ResolveType::kNormal)
					HandleResolve(resolve_type, do_resolve);

				if (do_resolve)
					OnComboResolve(resolve_type);
			}


			//Utility::DebugMessageBox(std::format("Finishing {}", ID()), IsPlayerRef());


			//Need to still let this happen, there's important stuff in there. I may make a distinction between
			// on resolve and on finish, with resolve voiding clear, and finish always happening.
			OnComboFinish(resolve_type, do_resolve);


			if constexpr (SUB_AV != RE::ActorValue::kNone) {
				SetRegenSubscription(SUB_AV, false);
			}


			ReleaseCounter();
			ReleaseValue();

			_resolveDuration = 0;

			bool was_primary = IsPrimary();

			SetActive(false);

			//Basically if everything is waiting on this to end, send them all a resolution.
			if (was_primary)
				SendOwnerEvent("OnStartResolve");

			if (IsActive() == true)
				Utility::DebugMessageBox(std::format("{} did not shut down", ID()), IsPlayerRef());
		}

		virtual void OnTimerFinish(Timer* timer_source, float total_time) { FinishCombo(ResolveType::kNormal); }

		//bool& const isActive = _active;
		//If it must not be none, and if so it must not equal just secondary
		bool IsActive() { return _flags != ComboStateFlag::None && _flags != ComboStateFlag::Secondary; }
		bool IsResolving() { return _comboTimer.isActive(); }//Is this is active? I don't remember.
		bool IsPrimary() { return _flags == ComboStateFlag::Primary; }
		bool IsWaiting() { return _flags == ComboStateFlag::Waiting; }
		bool IsSecondary() { return (int)(_flags & ComboStateFlag::Secondary) == (int)ComboStateFlag::Secondary; }

		float GetResolveDuration(bool raw = false) { return raw ? _resolveDuration : fmax(k_minResolveDuration, _resolveDuration); }
		virtual void SetResolveDuration(float duration)
		{
			OnSetResolveDuration(duration);
			_resolveDuration = duration;
		}



		virtual bool StartCombo()
		{
			if (IsActive() == true) {
				return false;
			}
			SetActive(true);
				
			StopResolve();

			//Utility::DebugMessageBox(std::format("Starting {}", ID()), IsPlayerRef());

			if constexpr(SUB_AV != RE::ActorValue::kNone) {
				SetRegenSubscription(SUB_AV, true);
			}


			OnComboStart();

			return true;
		}

		void StopResolve()
		{
			//if (IsPlayerRef() == false)
			//	Utility::DebugNotification(std::format("combo resolve stopping: {}, {}", ID(), (int)GetActorOwner()->GetAttackState()), true, 0, true);
				

			SendOwnerEvent("OnStopResolve");
			SetPrimary(true);
		}

		void ResolveCombo(bool ignore_primary = false)
		{
			if (IsActive() == false || _comboTimer.isActive() == true)
				return;

			if (!ignore_primary && IsSecondary() == true) {
				//Utility::DebugNotification(std::format("non-primary cannot resolve {}, secondary&: {}, test: {}", (int)_flags, (int)(_flags & ComboStateFlag::Secondary),  _flags & ComboStateFlag::Secondary == ComboStateFlag::Secondary), IsPlayerRef());
				SetWaiting(true);
				return;
			}
			//Do I make it so only primary does this or not is the question?
			//if (IsPlayerRef() == false)
			//	Utility::DebugNotification(std::format("Resolving other combo: {}", ID()), true, 0, true);
				
			SendOwnerEvent("OnStartResolve");
		}

		//These can and should be templated to accept enums.
		template<class IndexType> requires(std::is_enum<IndexType>::value || std::is_integral<IndexType>::value)
		void ModifyCounter(IndexType raw_index, int mod_value, bool guard = false)
		{
			if (IsActive() && guard)
				return;

			auto index = static_cast<unsigned int>(raw_index);

			if (index < 0 || index >= counter_size)
				return;

			auto& value = _counterArray[index];
			bool progress = true;
			OnModifyCounter(index, value, mod_value, progress);

			if (progress)
			{
				if (IsActive() == false)
					StartCombo();
				else
					StopResolve();

			}
				
			value += mod_value;
		}


		template<class IndexType> requires(std::is_enum<IndexType>::value || std::is_integral<IndexType>::value)
		void ModifyValue(IndexType raw_index, float mod_value, bool guard = false)
		{
			if (IsActive() && guard)
				return;

			auto index = static_cast<unsigned int>(raw_index);


			if (index < 0 || index >= value_size)
				return;

			auto& value = _valueArray[index];
			bool progress = true;
				
			OnModifyValue(index, value, mod_value, progress);

			if (progress)
			{
				if (IsActive() == false)
					StartCombo();
				else
					StopResolve();

			}
			
			value += mod_value;
		}





		template<class IndexType> requires(std::is_enum<IndexType>::value || std::is_integral<IndexType>::value)
		constexpr Counter GetCounter(IndexType raw_index)
		{
			auto index = static_cast<unsigned int>(raw_index);

			if (index < 0 || index >= counter_size)
				return 0;

			return _counterArray[index];
		}


		template<class IndexType> requires(std::is_enum<IndexType>::value || std::is_integral<IndexType>::value)
		constexpr float GetValue(IndexType raw_index)
		{
			auto index = static_cast<unsigned int>(raw_index);

			if (index < 0 || index >= value_size)
				return 0;//or throw exception?

			return _valueArray[index];
		}




		template<class IndexType> requires(std::is_enum<IndexType>::value || std::is_integral<IndexType>::value)
		void SetCounter(IndexType raw_index, int set_value, bool guard = false)
		{
			float cur_value = GetCounter(raw_index);
			ModifyCounter(raw_index, set_value - cur_value, guard);
		}

		template<class IndexType> requires(std::is_enum<IndexType>::value || std::is_integral<IndexType>::value)
		void SetValue(IndexType raw_index, float set_value, bool guard = false)
		{
			float cur_value = GetValue(raw_index);
			ModifyValue(raw_index, set_value - cur_value, guard);
		}



		virtual void SerializeData(SerialArgument& serializer, bool& success) override
		{ 
			serializer.Serialize(_comboTimer);

			//Utility::DebugMessageBox(std::format("used resource {} for {}", _usedResource, ID()), IsPlayerRef());
		}
	};

	template<RE::ActorValue SUB_AV, unsigned int COUNTER_NUM = 0, unsigned int VALUE_NUM = 0>
	class IncrementalCombo : public ComboBase<SUB_AV, COUNTER_NUM + 2, VALUE_NUM>
	{
		//Includes behaviours to keep track of how many pieces of a combo have been happening.
	private:
		bool _startIncementsCombo = true;
	protected:
		Counter& _comboCount = this->_counterArray[0];
		Counter& _successCount = this->_counterArray[1];

		//These use references so this can be a conditioning function, as well as an on increment one.
		virtual void OnComboIncrease(unsigned int& inc) = 0;
		virtual void OnSuccessIncrease(unsigned int& inc) = 0;

		void ComboIncrease_Base(unsigned int inc)
		{
			if (!inc)
				return;
			 //std::uint8_t real_old = __super::GetData()._counterArray[0];
			 //auto real_old2 = _comboCount;
			OnComboIncrease(inc);

			if (inc)
				this->StopResolve();

			//auto old = __super::GetData()._counterArray[0];
			//auto old2 = _comboCount;
			_comboCount += inc;
			/*
			Utility::DebugMessageBox(std::format("1: old:{} vs new:{}\n2: old:{} vs new:{}\n3: old old:{} vs old old 2:{}", 
				old, __super::GetData()._counterArray[0],
				old2, _comboCount,
				real_old, real_old2,
				real_old2, _comboCount), true);
			//*/
		}

	public:

		//These will not use modify to increment, since it uses its own event for it.
		void ComboIncrease(unsigned int inc = 1)
		{
			bool start_success = true;
			//Might do this after?
			if (this->IsActive() == false){
				_startIncementsCombo = false;
				start_success = StartCombo();
			}
					
			if (!start_success)
				return;
				
			//Bit jank, but it'll increment AFTER the combo has started.
			ComboIncrease_Base(inc);
		}

		void SuccessIncrease(unsigned int inc = 1)
		{
			OnSuccessIncrease(inc);
			_successCount += inc;
		}

		bool StartCombo() override
		{
			bool increment = _startIncementsCombo;
			
			_startIncementsCombo = true;

			bool success = __super::StartCombo();

			if (success && increment) {
				ComboIncrease_Base(1);
			}

			return success;
		}
	};
	


//*
		
	class MeleeCombo : 
		public IncrementalCombo<RE::ActorValue::kStamina, 0, 3>,
		public CompetitiveEvent<RE::TESObjectWEAP*>, 
		public SerialComponent<bool>//this is likely the unimplemented.
	{
		//Has a serial component so I can serialize the bool and not override the main serialize function.

		//Things of note
		// *It is unknown how dual wielding will react to the system, especially the hit system, needs testing.
		//IMPLEMENT(MeleeCombo)
	protected:
		float& _powerAttackReduction = _valueArray[powerAttackReduction];
		float& _staminaLostRate = _valueArray[staminaLostRate];
		float& _climbingValue  = _valueArray[climbingValue];
			
		//could make this a variable which would undo itself
		bool& _confirmedHit = SerialComponent<bool>::data;//This is to prevent cleaves from proccing more than needed.

		
	//public:
		static constexpr float k_preemptiveTime = 0.3f;
		static constexpr float k_comboDropTime = 1.55f;

		static constexpr float posInc = 0.5f;
		static constexpr float negInc = 0.5f;
			
		const char* ID() override { return "MELEE"; }


	public:

		bool IsInFirstBlow() { return _comboCount == 1; }

		Counter GetHits() { return _successCount; }
		Counter GetCount() { return _comboCount; }

		void OnComboIncrease(unsigned int& inc) override;


		void OnSuccessIncrease(unsigned int& inc) override;


		void OnInterrupt() override;

		//While I want this to be it's own function, it would functionally only be used on end, so you know.
		void HandleResourceReturn(ResolveType resolve);


		void OnComboResolve(ResolveType resolve) override;

		void OnComboFinish(ResolveType resolve, bool resolved) override;

		bool OnResult(bool success, RE::TESObjectWEAP* weap) override;
			
	};
		

	//So now, GuardStatController owns both WardCombo and block combo.
	//Set it up so the 2 of these can be used interchangably via a ComboBase*. This way, I can have a function
	// for get relevant combo, and both sorta function (though not without some active 
	class WardCombo
	{
		//WardCombo is different than block combo, and similar how guard combo overrides melee's control of bash,
	};

	class BlockCombo : public ComboBase<RE::ActorValue::kStamina, 1, 3>
	{
		//Things of note
		// *It is unknown how dual wielding will react to the system, especially the hit system, needs testing.
		//IMPLEMENT(MeleeCombo)
	protected:
#define _actionStamina _usedResource
		//raise stamina is the stamina used to raise the shield, it's reimmursed immediately
		//temp stamina is held to evaluate something, I would like to use a competitive event for this.
		// when a bash attack is used, I'd like to store the stamina used and give it once they hit something.
		// if temp stamina is modified again, it will dump the current value of what it's trying to edit and supplant the new one.
		//Block damage is how much burst damage you've blocked from shielding, everytime you get hit it will begin to collect 
		// (might work weirdly for melee with the sos shield mod) and it will be stored in block damage.
		float& _raiseStamina = _valueArray[raiseStamina];
		float& _tempStamina = _valueArray[tempStamina];
		float& _blockDamage = _valueArray[blockDamage];

		//This largely controls how the ends will be interpreted,

		void OnTimerFinish(Timer* timer_source, float total_time) override { FinishCombo(endType); }
			
		const char* ID() override { return "BLOCK"; }

	public:
		ResolveType endType = ResolveType::kNormal;



		//RVICE
		virtual void HandleResolve(ResolveType resolve_type, bool& do_resolve) override
		{
			float give = 0.15f;

			switch (resolve_type)
			{
				//Revision, I think I want negative to make everything come back slow,
				// and negative bonus void action stamina.

			case ResolveType::kNegativeBonus:
				//Do penalty
					
			case ResolveType::kNegative:
					
				_actionStamina *= 0.75f;
				_actionStamina += _raiseStamina;
				_raiseStamina = 0;
				break;
			case ResolveType::kPositiveBonus:
				give = 0.3f;
				
			case ResolveType::kPositive:
				_raiseStamina += _actionStamina * give;
				_actionStamina -= _actionStamina * give;
				break;
			}
		}
			
		//minimize all values I guess.
		void OnModifyValue(unsigned int index, float curr_value, float& mod_value, bool& progress) override
		{
			if (index != tempStamina)
				return;

			Utility::DebugMessageBox("is temp stamina", IsPlayerRef());
			progress = false;
			//If it's a temp value, clear what ever the current value is.
			mod_value -= curr_value;
		};



		void HandleBlockStamina(RE::Actor* actorOwner);
		void HandleRaiseStamina(RE::Actor* actorOwner)
		{
			if (!_raiseStamina || !actorOwner)
				return;

			actorOwner->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, _raiseStamina);
		}

			
		//Required for release
		void OnComboResolve(ResolveType resolve_type) override
		{
			RE::Actor* actorOwner = GetActorOwner();

			//Utility::DebugNotification(std::format("{} relaxed a block, raise {}, action {}", actorOwner->GetName(), _raiseStamina, _actionStamina), IsPlayerRef());

			HandleRaiseStamina(actorOwner);
			HandleBlockStamina(actorOwner);
		}

		//Required for release
		void OnComboFinish(ResolveType resolve_type, bool resolved) override { endType = ResolveType::kNormal; }
	};
	

#ifdef use_combo_v1
	template <int test>
	class First{};

	template<int second_test>
	class Second : public First<second_test + 1> {};


	class MagicCasterData;




	//Thinking about it, the combo itself should be set up around a timer
	// magic ones need it too.
	template <class... IncrementArgs>
	class Combo : 
		public IRegenSubscriber,
		public ITimerCallback
	{
	public:

		std::uint8_t count;
		float usedResource;
	protected:
		Timer _comboTimer = Timer::Local();
		Second<1> second_test;


	public:

		void IncrementCombo(IncrementArgs... args)
		{
			active = true;
			
			if (ShouldComboIncrease(args...) == true)
				count++;
				
			SendOwnerEvent("OnAction");
			
			OnComboIncrease(args...);
		}

		void End(ResolveType resolve)
		{
			//First point of issue, this doesn't seem to be protected the below from firing off.
			if (!active)
				return;
		

			//return;


			ReleaseResource();

			OnEnd(resolve);
			count = 0;
			usedResource = 0;
			active = false;
		}


		virtual void StartEndComboWindow() { }
		
		virtual void StopEndComboWindow() { }

		virtual void ExtendEndComboWindow() { }

		virtual float GetComboDropTime() = 0;

		virtual RE::ActorValue GetResourceType() = 0;

		bool IsComboActive() { return count != 0; }

		void OnTimerFinish(Timer* timer_source, float total_time) override { End(ResolveType::kNormal); }


	protected:
		
		virtual void OnInterrupt(EventObject& params, EventControl& control)
		{
			End(ResolveType::kNegative);
		}

		void Initialize() override 
		{ 
			_comboTimer.AddCallback(this); 
			CreateMemberEvent("OnInterrupt", owner, &Combo::OnInterrupt);
		}

		bool active{ false };

		virtual bool ShouldComboIncrease(IncrementArgs... args) { return true; }
		virtual void OnComboIncrease(IncrementArgs... args) {}

		virtual void OnEnd(ResolveType resolve) {}
		
		virtual void ReleaseResource()
		{
			RE::Actor* actorOwner = GetActorOwner();
			actorOwner->RestoreActorValue(AVModifier::kDamage, GetResourceType(), usedResource);
		}


	};

	class MeleeCombo :
		public Combo<>
	{
		//Things of note
		// *It is unknown how dual wielding will react to the system, especially the hit system, needs testing.
		//IMPLEMENT(MeleeCombo)
	public:
		std::uint8_t hits;
		float staminaLostRate;
		float climbingValue;


		float powerAttackReduction;//This likely isn't gonna be used here, magic has nothing for it.
		const float k_preemptiveTime = 0.3f;
		const float k_comboDropTime = 1.55f;

		const float posInc = 0.5f;
		const float negInc = 0.5f;
	protected:
		bool _confirmedHit = false;//This is to prevent cleaves from proccing more than needed.

	private:
		//Privates are marked as temp
		float _comboDropTime = 0;
	//Functions
	public:

		RE::ActorValue GetResourceType() override { return RE::ActorValue::kStamina; }

		void StartEndComboWindow() override;

		void ExtendEndComboWindow() override;

		void IncrementHit();

		bool IsInFirstBlow() { return count == 1; }
		
		void OnInterrupt(EventObject& params, EventControl& control) override;

		float GetComboDropTime() override
		{
			if (_comboDropTime == 0)
			{
				if (IsInFirstBlow() == true)
					return 0.3f;
				else
					return 1.55f;
			}

			return _comboDropTime;
		}
		void SetComboDropTime(float value) 
		{
			_comboDropTime = value;
		}

		auto GetHits() { return hits; }
		auto GetCount() { return count; }
	protected:
		void ReleaseResource() override;
		void OnEnd(ResolveType resolve) override;
		void OnComboIncrease() override;
	};

	class MagicCombo : public Combo<MagicCasterData*>
	{
	//Variables
	public:
		float magickaSpent;
		float& magickaRestore = usedResource;
		float dualCastReduction;
		float dualCastCooldown;

		std::uint8_t input = 0;

	protected:

		const float k_Temp_MagicComboTimer = 2.f;

	//Functions
	public:
		RE::ActorValue GetResourceType() override { return RE::ActorValue::kMagicka; }

		void StartEndComboWindow() override;
		void StopEndComboWindow() override;
		void ExtendEndComboWindow() override;

		//Want to replace this with something more dynamic
		float GetComboDropTime() override { return 2; }
	protected:
		
		void OnEnd(ResolveType resolve) override;

		bool ShouldComboIncrease(MagicCasterData* castData) override;
		
		void OnComboIncrease(MagicCasterData* castData) override;
		
		void ReleaseResource() override;
	};
#endif
}