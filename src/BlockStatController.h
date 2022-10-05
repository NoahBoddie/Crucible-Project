#pragma once

#include "Combo.h"
#include "Timer.h"
#include "Utility.h"
#include "StatedObject.h"
#include "CombatDataBehaviour.h"
#include "IRegenSubscriber.h"
#include "IUpdateBehaviour.h"
#include "ItemStatHandler.h"

#include "SerializableList.h"
#include "SerializableObject.h"
#include "SerializationTypePlayground.h"


#include "AttackInfo.h"
#include "StrikeEnumerations.h"

#include "ActionPointController.h"


namespace Crucible
{
	enum class GuardState 
	{
		kNone,			//Not blocking.
		kGuard,			//Regular
		kWeakGuard,		//On the edge, when major actions are levied against them, they will go into guard broken state.
		kStrongGuard,	//Minor actions cannot be levied against you and major actions won't weaken your guard, but send you to normal.
		kGuardDrop,		//Guard is dropped for a period of time, but isn't broken. Entered when dropping a weakened guard (short), or holding it too long.
		kGuardBreak		//Guard is broken, down for longer and positive bonuses are frozen.
	};

	enum BlockStage { 
		Stage0, 
		Stage1, 
		Stage2, 
		StageTotal, 
		StageFinal = StageTotal - 1 };

	class CombatData;
	class CombatValueOwner;

	inline BlockStage& operator++(BlockStage& stage, int)
	{
		switch (stage) {
		case Stage0: 
			return stage = Stage1;
		case Stage1:
			return stage = Stage2;
		}

		return stage;
	}



	struct GuardData
	{
		//These are used to measure

		float _prevTime = 0;//This is used to record what value the timer was on last. Used mainly for weakened guard
							// and consumed strong guard. When in normal guard prevTime is assumed to be consumed, when weak
							// assumed to be for the stage timer.


		BlockStage _stage;


		RE::TESForm* _blockForm;//Gets replaced at a later point by a SerializableForm

		Timer _strongTimer = Timer(TimerType::Local, TimeScale::Real);
		Timer _stateTimer = Timer(TimerType::Local, TimeScale::Real);



		//Timer _strongTimer = 

		BlockCombo blockCombo;





	};

	struct CounterEntry : public ISerializable
	{
		SerialFormID attacker;
		float damage;
		float blocked;
		float stamina;

		bool parry;

		//Cause I don't need the extra shit that comes with timers, I just want a thing that ticks to zero.
		float timeLeft;


		CounterEntry() = default;
		
		CounterEntry(RE::FormID id, float dam, float block, float stam, float start_time, bool pry)
		{
			attacker = id;
			damage = dam;
			blocked = block;
			stamina = stam;

			parry = pry;

			timeLeft = start_time;
		}

		void OnDeserialize(SerialArgument& serializer, bool& success, ISerializable* pre_process) 
		{
			serializer.Serialize(attacker);
		}
	};

	struct CounterList : public ISerializer
	{
		SerialVector<CounterEntry> counterContainer;

		//Don't really need this exactly, it's not that bad to iterate through a vector.
		//float totalStaminaSpent = 0;
			
		//*
		void Update(float time)
		{
			for (auto iter = counterContainer->begin(); iter != counterContainer->end(); ) {
				auto& entry = *iter;
					
				entry.timeLeft -= time;

				if (entry.timeLeft <= 0) {
					//totalStaminaSpent -= entry.stamina;
					iter = counterContainer->erase(iter);
				}
				else{
					++iter;
				}
			}
		}

		void Clear()
		{
			counterContainer->clear();
		}

		void AddEntry(RE::Actor* actor, float damage, float block, float stamina, float time, bool parry)
		{
			if (!actor)
				return;

			CounterEntry new_entry(actor->GetFormID(), damage, block, stamina, time, parry);

			counterContainer->push_back(new_entry);
		}

		bool IsViable(float health_req, int count_req) 
		{
			float blocked_damage = 0;
			int count = 0;

			for (auto& entry : *counterContainer) 
			{
				count++;

				if (count >= count_req) {
					return true;
				}
				blocked_damage += entry.blocked;
				
				if (blocked_damage >= health_req)
					return true;
			}

			return false;
		}

		bool IsViable(RE::TESForm* block_form)
		{
			//I may seperate health, and damage blocked.
			ShieldStat* block_stat = ItemStatHandler::GetStatBase<ShieldStat>(block_form);

			if (!block_stat)
				return false;
			//StatValue BIND_VALUE(counterBashStagger, 2);
			//StatValue BIND_VALUE(counterBashHealthReq, 0.15f);
			//StatValue BIND_VALUE(counterBashCountReq, 2);
			//StatValue BIND_VALUE(counterHitDecay, 2);
			//StatValue BIND_VALUE(strongCounterHitDecay, 2);
			float dam_req = block_stat->GetValue(counterBashHealthReq);
			float count_req = block_stat->GetValue(counterBashCountReq);

			return IsViable(dam_req, count_req);
		}

		float GetBashDamage(RE::TESForm* block_form, RE::Actor* defender)
		{
			ShieldStat* block_stat = ItemStatHandler::GetStatBase<ShieldStat>(block_form);

			if (!block_stat)
				return 0;

			float total_damage = 0;

			float regular_mult = block_stat->GetValue(counterBashDamageMult);
			float revenge_mult = block_stat->GetValue(counterBashRevengeDamageMult);
			float parry_mult = block_stat->GetValue(specialValue);

			bool once = false;

			/*
			for (auto& entry : *counterContainer) {
				if (!once && entry.parry) {
					RE::DebugMessageBox("PARRY BLOW!");
				}

				total_damage += entry.damage * (entry.attacker == defender->formID ? revenge_mult : regular_mult) * (entry.parry ? parry_mult : 1);
			}
			//*/
			//RE::DebugMessageBox(std::format("Damage: {}", total_damage));
			//This version is intended to only increase the damage if the very last attacker registered was this dude
			
			int is_revenge = 0;

			for (int i = counterContainer->size() - 1; i >= 0; i--)
			{
				auto& entry = counterContainer[i];

				if (!once && entry.parry) {
					RE::DebugMessageBox("PARRY BLOW!");
				}

				bool attacker_is_defender = entry.attacker == defender->formID;

				if (!is_revenge){
					is_revenge = attacker_is_defender ? 1 : -1;

					if (is_revenge == 1)
						RE::DebugMessageBox("Revenge blow");
				}

				total_damage += entry.damage * (attacker_is_defender && is_revenge == 1 ? revenge_mult : regular_mult) * (entry.parry ? parry_mult : 1);
			}


			return total_damage;
		}

		float GetStaminaReduction(RE::TESForm* block_form)
		{
			ShieldStat* block_stat = ItemStatHandler::GetStatBase<ShieldStat>(block_form);

			if (!block_stat)
				return 0;

			float total_stamina = 0;

			float base = block_stat->GetValue(counterBashStaminaBase);
			float mult = block_stat->GetValue(counterBashStaminaMult);

			for (auto& entry : *counterContainer){
				total_stamina += base + (mult * entry.stamina);
			}

			RE::DebugMessageBox(std::format("Stamina: {}", total_stamina));

			return total_stamina;
		}

		void HandleSerialize(SerialArgument& serializer, bool& success)
		{
			serializer.Serialize(counterContainer);
		}
		//*/
	};

	class GuardStatController :
		public StatedObject<GuardState>,
		public IUpdateBehaviour
	{
		/*
		An event that happens on bash start (and block start if possible). This would be the before event set up I was talking about, so I can catch it

		TryRaise, This thing is what the event will be. It should be used, but it should be used in house, only by the event.


		When you raise your shield, I'd super like to cache what blocking object someone has.
			There's never a situation where that would change without forcing someone out.

		Maybe make an event for block stage incrementing? For sound stuff?

		StartGuard is an important function. This is the function that determines someone's start up settings
		StopGuard juxopposes it, representing when the npc means to put their shield down


		DropGuard, a function that's meant to be what happens when someone involved drops their guard.
			This function however is no longer required, as now interruptions (the only thing I'd want to cast DropGuard but without blockStop sent) work on combos as proper, not block stat controller.
		^ In this sense, DropGuard is completely not needed anymore.

		Can force cancel, kept obviously, this is used for the passive drop, for whatever it needs to take care of it.
		CheckGuarding, not really needed honestly, I just need to proc for block stop, and ask if we are bashing instead.
			But keep it around, if that doesn't work I want a fall back.

		ShouldBlockHitIdle, ShouldRecoilIdle, as well as a new comer in ShouldAllowEvade are core functions for aspects involving block.

		Timer jobs
		Malus, measuring how long before one exits weak guard. Easy to tell it's context by the fact that it's in weak guard.
		BlockStage, measures how long before it moves into the next block stage. Easy to tell it's for this, it will be in any guardable state that isn't weak guard.

		//*/
	public:
		CounterList _viableCounterList;//temporarily public.

		bool _isCounterBash = false;

		bool IsCounterBash() { return _isCounterBash; }

		bool IsCounterBashViable() { return _viableCounterList.IsViable(_blockForm) && !IsWeakGuard(); }

		bool CheckGuarding(RE::Actor* actor = nullptr)
		{
			//As a precaution, if not already in a guarding state this should likely not go through.

			if (!actor) {
				actor = GetActorOwner();

				if (!actor) return false;
			}

			//bool result;
			//actor->GetGraphVariableBool("isBashing", result);

			//if (result)
			//	return true;

			//We use this instead, because getting isBlocking would activate if there was some passive blocking mechanic in place.
			int i_result = 0;
			actor->GetGraphVariableInt("iState", i_result);

			int blockStand = 0;
			int blockCharge = 0;

			actor->GetGraphVariableInt("iState_NPCBlocking", blockStand);
			//RE::DebugMessageBox(std::format("1:{} 2:{}", i_result, blockStand));


			if (i_result == blockStand)
				return true;

			actor->GetGraphVariableInt("iState_NPCBlockingShieldCharge", blockCharge);

			if (i_result == blockCharge)
				return true;

			return false;
		}

		float GetBashDamage(RE::Actor* enemy) { return _isCounterBash ? _viableCounterList.GetBashDamage(_blockForm, enemy) : 0; }

		float GetBashStamina(){ return _isCounterBash ? _viableCounterList.GetStaminaReduction(_blockForm) : 0; }

		void AddCounterHit(RE::Actor* actor, float damage, float block, float stamina)
		{
			//StatValue BIND_VALUE(counterBashStagger, 2);
			//StatValue BIND_VALUE(counterHitDecay, 2);
			//StatValue BIND_VALUE(strongCounterHitDecay, 2);

			//No counter hits are added while guard is weakened, this is pretty much a direct to heavy shields, as they
			// will become far too weak
			if (IsWeakGuard() == true)
				return;

			bool is_strong = IsStrongGuard();
		

			bool is_parry = is_strong && GetDefendingClass() == WeightClass::kMedium;

			float decay_time = ItemStatHandler::GetStatValue(_blockForm, !is_strong ? counterHitDecay : strongCounterHitDecay);

			if (decay_time <= 0)
				return;//This isn't something that should be allowed counterbash

			_viableCounterList.AddEntry(actor, damage, block, stamina, decay_time, is_parry);
		}

	private:
		SerialTESForm _blockForm;

		Timer _strongTimer = Timer(TimerType::Local, TimeScale::Real);
		Timer _stateTimer = Timer(TimerType::Local, TimeScale::Real);
		float _prevTime = 0;//This is used to record what value the timer was on last. Used mainly for weakened guard
							// and consumed strong guard. When in normal guard prevTime is assumed to be consumed, when weak
							// assumed to be for the stage timer.



		//Timer _strongTimer = 

		BlockCombo blockCombo;

			

		//const float k_temp_UpkeepTimer = 4.f;//I won't be making this unique but it might become a setting.
		//const float k_temp_ConsumedTimer = 5.f;

		//These are mults, they will be mults when we get them implemented. Based on weight or something.
		//const float k_temp_UpkeepCost_first = 1.f;
		//const float k_temp_UpkeepCost_second = 1.5f;//is additive.

		//const float k_temp_RaiseReq = 30.f;


		const float k_temp_GuardWeakTime = 1.f;//This gets to stay for now, because the time for the shield to stop being weak is not a value yet.

		//const float k_temp_GuardExhaustTime = 2.f;
		const float k_temp_GuardBreakTime = 5.f;



		const float k_temp_BlockComboEndTime = 1.75f;



		BlockStage _stage;

		bool _ignoreRecoilFlag = false;//Used to ignore recoil. Flash flagged before and after usage to negate the event.


		void StartPreviousTimer(float value)
		{
			float new_time = value - _prevTime;

			if (new_time <= 0)
				//I don't remember if it will immediately resolve if sent with 0, so I'll send it with a really small number.
				_stateTimer.Start(0.0001f);
			else
				_stateTimer.Start(new_time);



			//if (!_prevTime || value) _stateTimer.Start(value - _prevTime);
			//else _stateTimer.Start(_prevTime);

			_prevTime = 0;
		}

		void StartTimer(float value, bool save_prev = false)
		{
			if (_stateTimer.isActive() == true) {
				float time_passed = _stateTimer.GetTimeElapsed();
				_prevTime = save_prev ? time_passed : 0;	
			}

			_stateTimer.Restart(value);
		}

		float GetRaiseStaminaCost(RE::TESForm* block_form)
		{
			StatBase* block_stat = ItemStatHandler::GetStatBase(block_form);

		
			float raise_base = block_stat->GetValue(raiseBaseCost);
			float raise_mult = block_stat->GetValue(raiseWeightMult);
			float weight = block_form->GetWeight();
			//weight = weight ? weight : 5;
			float result = raise_base + (raise_mult * weight);

			return result;
		}

	public://A lot of the shit in here needs to be moved to privates

		WeightClass GetDefendingClass()
		{
			if (!_blockForm)
				return WeightClass::kNone;

			auto* block_armo = _blockForm->As<RE::TESObjectARMO>();

			return Utility::GetDefendingClass(block_armo);
		}

		RE::TESForm* GetDefendingForm() { return _blockForm; }

		//This should 
		void StartGuard(RE::TESForm* b_form, bool met_req)
		{
			//If you're a heavy shield, I will not stack the time for penalty and for start up
			// but the fact remains one is a penalty, so what I'll do is we go with whatever is greater
			// conversely, I could make it so heavy shield has a "buff" in that the weakness lasts a certain
			// amount of time always.

			//MetReq at a later point should be a mult, maybe?

			_blockForm = b_form;

			if (!met_req)
				Utility::DebugNotification("Lack of stamina weakens your guard...", IsPlayerRef());

			float weak_guard_time = ItemStatHandler::GetStatValue(b_form, weakGuardTime);

			float weak_time = met_req ? 0 : weak_guard_time;
			;//Will be dynamic later on.

			if (GetDefendingClass() == WeightClass::kHeavy) {
				Utility::DebugNotification("Heavy shield incurs it's penalty...", IsPlayerRef());
				//weakGuardTime = fmax(4, weakGuardTime);
				float _k_temp_HeavyWeakTimer = 1.5f;
				weak_time = weak_time ? weak_time * 1.5f : weak_guard_time;//Temporary, probably gonna move this to global value.
			}

			if (weak_time) {
				WeakenGuard(weak_time);
			}
			else {
				GoToState(GuardState::kGuard);
					
				float upkeep_timer = ItemStatHandler::GetStatValue(_blockForm, upkeepTime);
				StartTimer(upkeep_timer);
			}
		}

		//NEW VERSION
		void StartGuard(RE::TESForm* b_form, float take_value)
		{
			//If you're a heavy shield, I will not stack the time for penalty and for start up
			// but the fact remains one is a penalty, so what I'll do is we go with whatever is greater
			// conversely, I could make it so heavy shield has a "buff" in that the weakness lasts a certain
			// amount of time always.

			//MetReq at a later point should be a mult, maybe?

			auto actor = GetActorOwner();

			float actorStamina = actor->GetActorValue(RE::ActorValue::kStamina);

			//If its negative, you put a weak guard down.
			bool met_req = true;//!IsResolveNegative(blockCombo.endType);

			if (actorStamina < take_value)
			{
				take_value = actorStamina;
				met_req = false;
			}

			_blockForm = b_form;

			if (!met_req)
				Utility::DebugNotification("Lack of stamina weakens your guard...", IsPlayerRef());

			float weak_guard_time = ItemStatHandler::GetStatValue(b_form, weakGuardTime);

			float weak_time = met_req ? 0 : weak_guard_time;
			;//Will be dynamic later on.

			if (GetDefendingClass() == WeightClass::kHeavy) {
				Utility::DebugNotification("Heavy shield incurs it's penalty...", IsPlayerRef());
				//weakGuardTime = fmax(4, weakGuardTime);
				float _k_temp_HeavyWeakTimer = 1.5f;
				weak_time = weak_time ? weak_time * 1.5f : weak_guard_time;//Temporary, probably gonna move this to global value.
			}

			if (weak_time) {
				WeakenGuard(weak_time);
			}
			else {
				GoToState(GuardState::kGuard);

				float upkeep_timer = ItemStatHandler::GetStatValue(_blockForm, upkeepTime);
				StartTimer(upkeep_timer);
			}



			actor->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, -take_value);

			ModStamina(take_value, false);

		}

		void TryCounterBash()
		{
			//spread this function how more, it really needs it. For example bashing when someone hasn't hit you yet, but I seek to accumulate damages against them
			// because they hit me in the middle of bashing.
			_isCounterBash = IsCounterBashViable();

			//if (_isCounterBash)
			//	Utility::DebugMessageBox("Counter Bash start", IsPlayerRef());
		}

		void StartBash()
		{

			auto actor = GetActorOwner();

			if (!actor) {
				return;
			}

			if (TryStrongGuard() == true){//(GetDefendingClass() == WeightClass::kMedium) {
				actor->SetGraphVariableBool("Isblocking", true);//This may not be required.
			}
			
			
			TryCounterBash();
		}


		void PromoteResolveType(ResolveType resolve_type)
		{
			//This currently has an issue, I don't really want positive to shoot over to negative immediately because of how simple it is to 
			// do so.
			switch (resolve_type)
			{
			
				//If
			case ResolveType::kPositive:
				//Should it already be positive bonus, this should just bail.
				if (blockCombo.endType == ResolveType::kPositiveBonus) {
					break;
				}
				else if (IsResolveNegative(blockCombo.endType) == true){
					blockCombo.endType = ResolveType::kNormal;
				}

				goto regular_resolve;
			
				//If negative setting just negative, don't move, we could make a better negative.
			default://Represents negative and normal
				if (IsResolveNegative(blockCombo.endType) == true) {
					break;
				}
				regular_resolve:
			case ResolveType::kNegativeBonus:
			case ResolveType::kPositiveBonus:
			case ResolveType::kClear:
				blockCombo.endType = resolve_type;
			}
		}


		//This is the end function for the guard state. It cares not to handle the specific of punish
		// nor reward, in being used it just ends the thing.
		bool StopGuard(ResolveType resolve_type = ResolveType::kNormal, bool forced = false)
		{
			//I will automatically tell if something a guard break by it being the most serious resolve.
			//If it wants to do resolve, we set the timer, if not, we immediately bail out.

			//If the combo resolve type is negative, the storage type here must be strong in order for it to be undone.

			//Is technically the proper lower, but if I could have a merge between all of them, that'd be nice.
			if (IsGuardableState() == false)
				return false;

			GuardState goto_state = GuardState::kNone;

			switch (resolve_type)
			{
			case ResolveType::kNegative:
				goto_state = GuardState::kGuardDrop;
				break;
			case ResolveType::kNegativeBonus:
				goto_state = GuardState::kGuardBreak;
				break;
			}

			if (GoToState(goto_state) != goto_state)
				return false;
			//send resolve combo stuff.

			Clear();
			
			//PromoteResolveType(resolve_type);//Ready to replace.
			//*
			switch (resolve_type)
			{
			case ResolveType::kPositive:
				//Should it already be positive bonus, this should just bail.
				if (blockCombo.endType == ResolveType::kPositiveBonus) {
					blockCombo.endType = ResolveType::kNormal;
				}
				break;

			default:
				if (IsResolveNegative(blockCombo.endType) == true) {
					break;
				}

			case ResolveType::kNegativeBonus:
			case ResolveType::kPositiveBonus:
			case ResolveType::kClear:
				blockCombo.endType = resolve_type;
			}
			//*/
				

			blockCombo.SetResolveDuration(ItemStatHandler::GetStatValue(_blockForm, IsWeakGuard() ? weakBlockComboTime : blockComboTime));

			if (forced) {
				blockCombo.FinishCombo(blockCombo.endType);
			}
			else {
				blockCombo.ResolveCombo();
			}

			return true;
		}

		//Supposed to be when people lower their shields willingly, so default stop guard. Superceded.
		bool LowerGuard()
		{
			if (blockCombo.endType == ResolveType::kPositiveBonus && GetDefendingClass() == WeightClass::kMedium) {
				ActionPointController& action_control = GetSibling<ActionPointController>();//temp_GetActionPointController();

				if (action_control.StartDodge(ActionPoint::guardDropDodge) == true)
					Utility::DebugMessageBox("Guard Drop light dodge", IsPlayerRef());
			}
			
			//Functionally same? No need maybe?
			return StopGuard();

		}

		void WeakenGuard(float time = 0)//if zero, it's natural, otherwise it's not.
		{
			if (!time)
				time = ItemStatHandler::GetStatValue(_blockForm, weakGuardTime);

			if (GoToState(GuardState::kWeakGuard) != GuardState::kWeakGuard) {
				return;
			}
			Utility::DebugNotification("Your guard weakens...", IsPlayerRef());

			PromoteResolveType(ResolveType::kNegative);

			StartTimer(time, true);
		}

		//Need they be different? What even specific happens in them to warrent it?
		void BreakGuard(float time = 0)
		{
			if (!time)
				time = k_temp_GuardBreakTime;

			if (StopGuard(ResolveType::kNegativeBonus, true) == false)
				return;

			auto actor = GetActorOwner();

			static constexpr float k_temp_guardBreakMagnitude = 0.75f;

			Utility::DebugNotification("Your guard breaks!", IsPlayerRef());



			//swap all this out for a real stagger function btw.

			//No idea why this being like this works, but fuck it, I guess.
			NULL_CONDITION(actor)->SetGraphVariableFloat("staggerMagnitude", k_temp_guardBreakMagnitude);
			NULL_CONDITION(actor)->NotifyAnimationGraph("staggerStart");
			
			StartTimer(time);
		}

		//The effects of drop guard should be used in just about all other cases where someone 
		// drops their shield invuluntarily.
		void DropGuard(float time, bool force_end)
		{
			if (StopGuard(ResolveType::kNegative, force_end) == false)
				return;

			auto actor = GetActorOwner();

			StartTimer(time);

			if (actor && force_end)
				actor->NotifyAnimationGraph("blockStop");
		}



		void ModStamina(float stamina, bool action = true)
		{
			blockCombo.ModifyValue(action ? actionStamina : raiseStamina, stamina);
		}

		float GetStamina(bool action = true)
		{
			return blockCombo.GetValue(action ? actionStamina : raiseStamina);
		}

		void TryTempStamina(float stamina){ blockCombo.ModifyValue(tempStamina, stamina, true); }

		void SendTempStamina(float percent = 1.f, bool action = true)
		{

			//There is no need to be worried over success or not, it overrides each time after all.
			float temp_stamina = blockCombo.GetValue(tempStamina);
				
			//if (temp_stamina)
			//	Utility::DebugMessageBox(std::format("Giving {} of temp stamina", temp_stamina * percent), IsPlayerRef());

			if (!temp_stamina)
				return;

			ModStamina(temp_stamina * percent, action);

			blockCombo.SetValue(tempStamina, 0);
		}

		void OnBlockStageProgress(bool start_timer = true)
		{
			if (start_timer && _stage != StageFinal) {
				float upkeep_timer = ItemStatHandler::GetStatValue(_blockForm, upkeepTime);
				StartTimer(upkeep_timer);
			}

			//Utility::DebugNotification(std::format("{} stage", (int)_stage), IsPlayerRef());


			auto actor = GetActorOwner();

			if (!actor) {
				return;
			}

			if (GetDefendingClass() == WeightClass::kHeavy) {
				TryStrongGuard();
			}
		}
		
		bool TryBlock()
		{
			//if we have stamina, and aren't guard crushed, raise shield. Expend stamina.
			// If we can't expend all, go to weak guard, set time off by how far behind we are.
			
			auto state = GetCurrentState();

			if (IsGuardableState(state) == true) {
				return true;
			}

			//Remove this, it can cause a bit of a locking issue.
			if (IsUnguardableState(state) == true){// != GuardState::kNone) {
				return false;
			}

			return true;
			//===================================================

			auto actor = GetActorOwner();

			if (!actor) {
				return false;
			}

			float actorStamina = actor->GetActorValue(RE::ActorValue::kStamina);

			auto defendForm = GetBlockingForm(true);//will be needed later.

			float takeValue = GetRaiseStaminaCost(defendForm);//k_temp_RaiseReq;

			//If its negative, you put a weak guard down.
			bool metReq = true;//!IsResolveNegative(blockCombo.endType);

			if (actorStamina < takeValue)
			{
				takeValue = actorStamina;
				metReq = false;
			}

			StartGuard(defendForm, metReq);


			actor->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, -takeValue);

			ModStamina(takeValue, false);

			return true;
		}

		void TryGuardPoint(WeightClass block_class)
		{
			ActionPointController& action_control = GetSibling<ActionPointController>();//temp_GetActionPointController();
			
			if (action_control.IsGuarding() == true)
				return;

			bool guarded = false;


			switch (block_class)
			{
			case WeightClass::kHeavy:
				guarded = action_control.StartGuard(ActionPoint::heavyStrongGuard);
				break;

			case WeightClass::kMedium:
				guarded = action_control.StartGuard(ActionPoint::lightStrongGuard);
				break;
			}
			
			Utility::DebugNotification("block guard point successful", IsPlayerRef() && guarded);
		}

		bool TryStrongGuard()
		{

			if (IsStrongGuard() == true)
				return false;

			GuardState set_state = GuardState::kStrongGuard;
			
			WeightClass block_class = GetDefendingClass();
			
			auto going_state = GetGoingToState();
			bool is_transition = std::nullopt != going_state;

			GuardState default_going = is_transition ? going_state.value() : GuardState::kGuard;

			if (IsStrongGuardConsumed() == true) {
				set_state = default_going;
				goto decision;
			}



			//If we are in weak guard and not in a transition, stay in weakguard.
			if (!is_transition && IsWeakGuard() == true) {
				set_state = GuardState::kWeakGuard;
				goto decision;
			}
			
			
			switch (block_class)
			{
			case WeightClass::kHeavy:
				if (_stage == BlockStage::Stage0)
					set_state = GuardState::kGuard;
				break;
			
			case WeightClass::kMedium:
				if (is_transition)
					set_state = going_state.value();
				break;
			case WeightClass::kLight:
				set_state = default_going;
				break;
			}

			decision:

			if (is_transition){
				SetGoingState(set_state);
			}
			else{
				GoToState(set_state);
			}

			bool result = set_state == GuardState::kStrongGuard;

			if (result)
			{
				TryGuardPoint(block_class);
				PromoteResolveType(ResolveType::kPositive);
			}
			//if (result)
			//	Utility::DebugMessageBox("Strong guard enabled", IsPlayerRef());

			return result;
		}




		//A temporary test function
		bool TryBash()
		{
			return true;

			auto actor = GetActorOwner();

			if (!actor) {
					return true;
			}

			if (TryStrongGuard() == true){//(GetDefendingClass() == WeightClass::kMedium){
				actor->SetGraphVariableBool("Isblocking", true);
			}
				
				
			_isCounterBash = IsCounterBashViable();

			//if (_isCounterBash)
			//	Utility::DebugMessageBox("Counter Bash start", IsPlayerRef());

			return true;
		}


		void ConsumeStrongGuard(float time)
		{
			if (IsStrongGuard() == false)
				return;

			Utility::DebugMessageBox("Strong guard consumed...", IsPlayerRef());

			_stateTimer.Pause();

			_strongTimer.Start(time);
		}
		
		void AllowIdle(EventArgument& argument, RE::TESIdleForm* idle, RE::Actor* actor, RE::TESObjectREFR* target)
		{
			//The experience that spawned this event dictates that I need other methods to actually START blocking
			// the idea should be that a successful try block here puts us in the state to actually start blocking,
			// however we wait to see where the block actually starts

			//Might make try block innocuous and then make the notify graph stuff the thing that does the business.
			if (TryBlock() == false) {
				argument.control = EventControl::kStop;
				//notify_event = "staggerStart";
			}
		}
		//Not really needed honestly.
		void OnCombatIdle(EventArgument& argument, RE::TESIdleForm* idle, RE::Actor* actor, const RE::BGSAttackData* attack_data)
		{
			if (attack_data->data.flags.all(AttackDataFlags::kBashAttack) == false) {
				return;
			}
			//For this purpose, try bash has to be innocuous, because it isn't the promise of it happening.
			//This fact may need to see it revised, but EH. I'll go through that when I get to it.
			if (TryBash() == false){
				argument.control = EventControl::kStop;
			}
		}


		//*

		void BeforeGraphInputEvent(EventArgument& argument, std::string event_name)
		{
			//This was an after event, I'm changing this to a before, prayign that nothing tries to intercept these 2.

			//Things we may want to stop go here, bash start, bash power start, block start

			//BashFail, bashPowerStart, bashStart
			//Legacy eventage.
			//std::string event_name = parameters[0].AsString();


			switch (hash(event_name))
			{
			case "bashStart"_h:
			case "bashPowerStart"_h:
				StartBash();
				break;

			case "recoilStart"_h:
			{
				if (_ignoreRecoilFlag) {
					argument.control = EventControl::kStop;
					_ignoreRecoilFlag = false;
				}

				break;
			}

			case "blockStart"_h://Will likely use an update and check guarding to tell this, bashing causes this pain especially
			{
				auto defend_form = GetBlockingForm(true);//will be needed later.
				float take_value = GetRaiseStaminaCost(defend_form);//k_temp_RaiseReq;
				StartGuard(defend_form, take_value);
			}
			break;
			}
		}
		//Don't actually think output is different in any way.
		void OnGraphOutputEvent(std::string event_name)
		{
			//The ones that aren't intended on being stopped go here
			// Bash and block stop

			auto actor = GetActorOwner();

			//Legacy
			//std::string event_name = parameters[0].AsString();
			bool blocking = false;

			bool medium_guard = false;
			switch (hash(event_name))
			{
			case "bashExit"_h:
			{
				if (GetCurrentState() == GuardState::kStrongGuard && GetDefendingClass() == WeightClass::kMedium) {
					medium_guard = true;

					GoToState(GuardState::kGuard);

					Utility::DebugMessageBox("unguard", IsPlayerRef());
				}


				if (CheckGuarding(actor) == false)
				{
					if (medium_guard)
						actor->SetGraphVariableBool("Isblocking", false);

					goto lowering_guard;
				}

				if (_isCounterBash)
					_viableCounterList.Clear();//Either way,

				break;
			}
			case "blockStop"_h:
			{

				if (IsGuardableState() == false)
					return;

				//Get the combo ready to end
				bool result;//We can use attack states for this.
				if (!actor->GetGraphVariableBool("isBashing", result) || !result)
				{
				lowering_guard:

					if (IsWeakGuard() == true) {
						float drop_guard_time = ItemStatHandler::GetStatValue(_blockForm, dropGuardTime);
						//Utility::DebugNotification(std::format("drop {}", drop_guard_time), IsPlayerRef());
						DropGuard(drop_guard_time, false);
					}
					else {
						LowerGuard();
						//Utility::DebugNotification("stop", IsPlayerRef());
					}
				}
				else if (IsStrongGuard() && GetDefendingClass() == WeightClass::kMedium){
					actor->SetGraphVariableBool("Isblocking", true);

				}
			}
			break;
			}
		}

		/*/
		void BeforeGraphInputEvent(EventArgument& argument, std::string& event_name)
		{
			//Things we may want to stop go here, bash start, bash power start, block start

			//BashFail, bashPowerStart, bashStart
			//Legacy eventage.
			//std::string event_name = parameters[0].AsString();


			switch (hash(event_name))
			{
			case "bashStart"_h:
			case "bashPowerStart"_h:
				if (TryBash() == false) {
					std::string test = "BashFail";
					//parameters[0] = test;
					//parameters[0].SetString(test);
					
					event_name = test;
					return;
				} 
				break;
			case "blockStart"_h:
				
				if (TryBlock() == false) {
					//std::string test = "";
					//parameters[0] = test;
					//auto actor = GetActorOwner();
					//actor->actorState2.wantBlocking = true;//Doesn't set the state.
					
					//event_name = "00NextClip";
					argument.control = EventControl::kStop;
					return;
				}
				break;
			}

			//SendOwnerEvent("OnAction");
		}
		
		void OnGraphOutputEvent(std::string event_name)
		{
			//The ones that aren't intended on being stopped go here
			// Bash and block stop

			auto actor = GetActorOwner();

			//Legacy
			//std::string event_name = parameters[0].AsString();
			
			bool medium_guard = false;
			switch (hash(event_name))
			{
			case "bashExit"_h:
			{
				if (GetCurrentState() == GuardState::kStrongGuard && GetDefendingClass() == WeightClass::kMedium) {
					medium_guard = true;
						
					GoToState(GuardState::kGuard);
						
					Utility::DebugMessageBox("unguard", IsPlayerRef());
				}
					

				if (CheckGuarding(actor) == false)
				{
					if (medium_guard)
						actor->SetGraphVariableBool("Isblocking", false);
						
					goto lowering_guard;
				}

				if (_isCounterBash)
					_viableCounterList.Clear();//Either way,

				break;
			}
			case "blockStop"_h:
			{
				if (IsGuardableState() == false)
					return;

				//Get the combo ready to end
				bool result;//We can use attack states for this.
				if (!actor->GetGraphVariableBool("isBashing", result) || !result)
				{
					lowering_guard:

					if (IsWeakGuard() == true) {
						float drop_guard_time = ItemStatHandler::GetStatValue(_blockForm, dropGuardTime);
						Utility::DebugNotification(std::format("drop {}", drop_guard_time), IsPlayerRef());							
						DropGuard(drop_guard_time, false);
					}
					else {
						StopGuard();
						Utility::DebugNotification("stop", IsPlayerRef());
					}
				}
			}
			break;
			}
		}

		//*/

		//*
		void OnAttackHitQuery(QueryContext& context, HitInfo& hit_info, StrikeResult& restrict_flags, CombatData* defender)
		{
			//  BashRestricts& Counter&
			bool isBash = context.GetBool(queryBash);
			
			if (!isBash)
				return;

			bool is_timed = context.GetBool(queryTimed);
			bool is_power = context.GetBool(queryPowerAction);
			bool is_counter = IsCounterBash();


			if (is_timed) {
				restrict_flags |= StrikeResult::allDefend;
			}
			if (!is_power)
				restrict_flags |= StrikeResult::majorAttack;

			if (is_counter) {
				//This is pushback btw, to be a constant or item state at a later point.
				NULL_CONDITION_TMP(hit_info.GetPhysicalHitData())->reflectedDamage = 1000;
				context.SetBool(queryCounter, is_counter);
			}

			//I'm thinking, should a block be neutral, I want it to do a minor stagger and forcibly push someone back.
			// dunno, it feels weird seeing it do no damage.


		}

		void BeforeHit(QueryContext& context, HitInfo& hit_info, StrikeResult& restrict_flags, CombatValueOwner* attacker)
		{
			if (IsWeakGuard() == true)
				restrict_flags |= StrikeResult::allDefend;

			if (attacker && context.GetBool(queryBlocked) == true)
			{
				//I would like this to be handled in the attack version, but it's fucking worthless.
				auto* guard_control = attacker->TryBehaviour<GuardStatController>();

				NULL_CONDITION(guard_control)->_ignoreRecoilFlag = ShouldRecoilIdle();//currently doesnt get undone, but fuck it for now.
			}

			//I won't be handling this here for right now.
			//_ignoreRecoilFlag =
		}

		void BeforeAttackHit(QueryContext& context, HitInfo& hit_info, StrikeResult& restrict_flags, CombatData* defender)
		{
			//better way to do this at this point.
			if (context.GetBool(queryCounter) == false)
				return;
			
			//I can just get it from defender, that would be better, or get it from hit info.
			//auto actor = context.GetForm<RE::Actor>(queryDefender);
			float damage = GetBashDamage(hit_info.target);
			NULL_CONDITION_TMP(hit_info.GetPhysicalHitData())->totalDamage += damage;

			if (damage > 0) {
				PromoteResolveType(ResolveType::kPositiveBonus);
				Utility::DebugMessageBox("Should promote", IsPlayerRef());
			}
		}

		void OnHit(QueryContext& context, HitInfo& hit_info, CombatValueOwner* attacker)
		{
			// : AddCounterHit
			//if (hit_info.GetResult() != StrikeResult::none)
			if ((*hit_info.result & StrikeResult::allAttack) != StrikeResult::none)
				return;
			else if (context.GetBool(queryBlocked) && (*hit_info.result & StrikeResult::majorDefend) != StrikeResult::none)
				PromoteResolveType(ResolveType::kPositiveBonus);

			auto* hit_data = hit_info.GetPhysicalHitData();
			//This is supposed to query blocked
			if (!hit_data || IsWeakGuard() == true || context.GetBool(queryBlocked) == false)
				return;
			
			auto actor = context.GetForm<RE::Actor>(queryDefender);


			using func_t = float(*)(RE::HitData*);
			REL::Relocation<func_t> get_block_stamina{ REL::ID(25864) };
			float block_stamina = get_block_stamina(hit_data);

			float blocked_damage = hit_data->totalDamage * hit_data->percentBlocked;
			float damage = hit_data->totalDamage - blocked_damage;

			AddCounterHit(actor, damage, blocked_damage, block_stamina);
		}

		void AfterHit(QueryContext& context, HitInfo& hit_info, CombatValueOwner* attacker)
		{
			if (context.GetBool(queryBlocked) == false)//Doesn't feel needed at this point
				return;


			//SendStamina
			StrikeResult result = hit_info.result;
			//Should be checking if this is in any kind of state before it does this likely
			//Feels like this should work differently, like it should make minor the exception, in which it would reduce on attack,
			// because minor drains the stamina anyways.
			if ((result & StrikeResult::exceptMajorAttack) == StrikeResult::none) {
				//float mod = result == StrikeResult::majorAttack ? -1 : 1;
				//float mod = result != StrikeResult::minorAttack;
				// I'm thinking up a scenerio where the perm stamina gets injuried.
				SendTempStamina(result == StrikeResult::minorAttack ? -1 : 1);
			}
		}
		//*/




		void OnTimerFinish(Timer* timer_source, float total_time)
		{
			auto current_state = GetCurrentState();


			//For now, I would like to set time based on something like this, but a major note is at a later

			if (timer_source == &_strongTimer)
			{
				logger::info("s. guard restrict lifted");
				_stateTimer.Resume();
					
				if (GetDefendingClass() == WeightClass::kHeavy) {
					TryStrongGuard();
				}
			}
			else
			{
				float time = 0;

				switch (current_state)
				{

				case GuardState::kGuard:
				case GuardState::kStrongGuard:
					_stage++;
					OnBlockStageProgress(true);//This should handle timing
					break;

				case GuardState::kWeakGuard:
					Utility::DebugNotification("Your guard tights", IsPlayerRef());
					GoToState(GuardState::kGuard);
					time = ItemStatHandler::GetStatValue(_blockForm, upkeepTime);
					break;

				case GuardState::kGuardDrop:
				case GuardState::kGuardBreak:
					Utility::DebugNotification("You can guard again", IsPlayerRef());
					GoToState(GuardState::kNone);
					break;
				}

				if (time)
					StartPreviousTimer(time);
			}
		}

		//for passives
		bool CanForceCancel(RE::Actor* actor = nullptr)
		{
			if (!actor) {
				actor = GetActorOwner();

				if (!actor) return true;
			}

			bool result;
			return !actor->GetGraphVariableBool("isBashing", result) || !result;

		}


		bool ShouldBlockHitIdle()
		{
			switch (GetDefendingClass())
			{
			case WeightClass::kHeavy:
				return !IsStrongGuard();
			default:
				return true;
			}
		}

		bool ShouldRecoilIdle()
		{
			switch (GetDefendingClass())
			{
			case WeightClass::kHeavy:
				return true;
				
			case WeightClass::kMedium:
				return IsStrongGuard();
			default:
				return false;
			}
		}

		//The lightest types can situationally evade.
		bool ShouldAllowEvade()
		{
			return true;
		}


		float _blockWant = -1;

		void OnUpdate(RE::Actor* self, float delta) override
		{
			if (1!=1 && self->IsPlayerRef() == false)
			{ 
				auto* base = self->GetActorBase();

				if (base && base->combatStyle)
				{
					float& defensiveness = base->combatStyle->generalData.defensiveMult;

					if (_blockWant == -1 && self->GetActorValue(RE::ActorValue::kStamina) <= 10)
					{
						_blockWant = defensiveness;

						defensiveness = 0;
					}
					else if (_blockWant != -1)
					{
						_blockWant = defensiveness;

						_blockWant = -1;
					}
				}
			}
			
			if (IsStrongGuard() == true)
				TryGuardPoint(GetDefendingClass());

			if (IsGuardableState() == false)
				return;

			_viableCounterList.Update(delta);
			
			//If bashing we won't be updating the stamina lost. If I can swing it, I'd like to pause it too.
			if (_stage == Stage0 ||  self->GetAttackState() == uATTACK_ENUM::kBash)
				return;

			


			float weight = 5;

			if (_blockForm)
				weight = fmin(_blockForm->GetWeight(), weight);


			StatBase* block_stat = ItemStatHandler::GetStatBase(_blockForm);


			float upkeep = 0;

			float upkeepDelta = weight * delta;

			float stamina = self->GetActorValue(RE::ActorValue::kStamina);

			float take;

			switch (_stage)
			{
			case Stage2:
				upkeep = block_stat->GetValue(majorUpkeepMult) * upkeepDelta;
				take = fmin(upkeep, stamina);
				//self->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, -take);

			case Stage1:
				//For this bit, only this amount is refundable.
				upkeep = block_stat->GetValue(minorUpkeepMult) * upkeepDelta;
				take += fmin(upkeep, stamina);
				self->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, -take);
				ModStamina(upkeep, true);
				stamina -= take;

				if (CanForceCancel(self) == true && stamina <= 0)
				{
					float exhaust_time = block_stat->GetValue(exhaustGuardTime);
					DropGuard(exhaust_time, true);
				}
			}

		}


		bool IsGuardableState(GuardState state)
		{
			switch (state)
			{
			case GuardState::kGuard:
			case GuardState::kWeakGuard:
			case GuardState::kStrongGuard:
				return true;
			default:
				return false;
			}
		}


		bool IsGuardableState()
		{
			auto state = GetCurrentState();

			return IsGuardableState(state);
		}

		bool IsUnguardableState(GuardState state)
		{
			switch (state)
			{
			case GuardState::kGuardDrop:
			case GuardState::kGuardBreak:
				return true;
			default:
				return false;
			}
		}


		bool IsUnguardableState()
		{
			auto state = GetCurrentState();

			return IsUnguardableState(state);
		}


		bool IsRegularGuard() { return GetCurrentState() == GuardState::kGuard; }
		bool IsWeakGuard() { return GetCurrentState() == GuardState::kWeakGuard; }
		bool IsStrongGuard() { return GetCurrentState() == GuardState::kStrongGuard; }

		bool IsStrongGuardConsumed() { return _strongTimer.isActive(); }

	protected:

		void Initialize() override
		{
			//Set the events thanks k
				
			_stateTimer.AddCallback(this);
			_strongTimer.AddCallback(this);

			//CreateMemberEvent("OnCombatIdle", &GuardStatController::OnCombatIdle);
			CreateMemberEvent("OnQueryIdle_blockStart", &GuardStatController::AllowIdle);


			//CreateMemberEvent("BeforeGraphInputEvent_bashStart", &GuardStatController::BeforeGraphInputEvent);
			//CreateMemberEvent("BeforeGraphInputEvent_bashPowerStart", &GuardStatController::BeforeGraphInputEvent);
			//CreateMemberEvent("BeforeGraphInputEvent_blockStart", &GuardStatController::BeforeGraphInputEvent);
			////CreateMemberEvent("OnGraphInputEvent", &GuardStatController::OnGraphInputEvent);
		
			//CreateMemberEvent("AfterGraphInputEvent", &GuardStatController::AfterGraphInputEvent);
			CreateMemberEvent("BeforeGraphInputEvent", &GuardStatController::BeforeGraphInputEvent);

			//CreateMemberEvent("OnCombatIdle_bashStart", &GuardStatController::OnCombatIdle);
			//CreateMemberEvent("OnCombatIdle_bashPowerStart", &GuardStatController::OnCombatIdle);

			//CreateMemberEvent("OnGraphOutputEvent_attackStop", &GuardStatController::OnGraphOutputEvent);
			//CreateMemberEvent("OnGraphOutputEvent_bashExit", &GuardStatController::OnGraphOutputEvent);
			//CreateMemberEvent("OnGraphOutputEvent_blockStop", &GuardStatController::OnGraphOutputEvent);
			CreateMemberEvent("OnGraphOutputEvent", &GuardStatController::OnGraphOutputEvent);
			
			CreateMemberEvent("OnAttackHitQuery", &GuardStatController::OnAttackHitQuery);
			CreateMemberEvent("BeforeHit", &GuardStatController::BeforeHit);
			CreateMemberEvent("BeforeAttackHit", &GuardStatController::BeforeAttackHit);
			CreateMemberEvent("OnHit", &GuardStatController::OnHit);
			CreateMemberEvent("AfterHit", &GuardStatController::AfterHit);
		}



		void SerializeData(SerialArgument& serializer, bool& success) override
		{
			serializer.Serialize(_viableCounterList);

			serializer.Serialize(_isCounterBash);
			serializer.Serialize(_blockForm);
			serializer.Serialize(_strongTimer);
			serializer.Serialize(_stateTimer);
			serializer.Serialize(_prevTime);
			serializer.Serialize(blockCombo);
			serializer.Serialize(_stage);
		}

		void OnStateFinish() override
		{
			auto going_state = GetGoingToState();

			if (going_state == GuardState::kGuard)
				TryStrongGuard();
				

			switch (GetCurrentState())
			{
				//For both of these I want to stop the timer.
			case GuardState::kStrongGuard:
			{
				ActionPointController& action_control = GetSibling<ActionPointController>();//temp_GetActionPointController();
				WeightClass block_class = GetDefendingClass();

				bool unguard_point = false;

				switch (block_class)
				{
				case WeightClass::kHeavy:
					unguard_point = action_control.StopAction(ActionPoint::heavyStrongGuard);
					break;

				case WeightClass::kMedium:
					unguard_point = action_control.StopAction(ActionPoint::lightStrongGuard);
					break;
				}

				//Utility::DebugMessageBox("undo guard point", IsPlayerRef() && unguard_point);


				auto state = GetGoingToState();//is optional
				if (going_state == GuardState::kWeakGuard) {
						
					float recovery = ItemStatHandler::GetStatValue(_blockForm, strongGuardRecovery);
					//This is dumb and bad depending on how I do it, because there's no way to control this based on
					// external influences like who attacked. I think what I'll do is, keep this, and later it will be 
					// block tool based.
					//So to reiterate, CHANGE THIS SHIT DUMMY.
					if (recovery != -1) {
						SetGoingState(GuardState::kGuard);
						
						ConsumeStrongGuard(recovery);
					}

				}
				break;
			}

			case GuardState::kWeakGuard:
				Utility::DebugNotification("The guard strengthens...");
				break;

			case GuardState::kGuardBreak:
				Utility::DebugNotification("You feel you can guard again...");
				break;
			}
		}
		void OnStateBegin() override
		{
			switch (GetCurrentState())
			{
			case GuardState::kNone:
			case GuardState::kGuardBreak:
			case GuardState::kGuardDrop:
				Clear();
				break;
			}
		}
			

		void Clear()
		{
			_blockForm = nullptr;
			_stage = Stage0;
			_stateTimer.Stop();
			_prevTime = 0;
			_isCounterBash = false;
			_viableCounterList.Clear();
		}

	};
}