#pragma once

#include "Timer.h"
#include "ActionStat.h"
#include "SerialComponent.h"
#include "SerializableMap.h"
#include "SerializableList.h"
#include "ActionStatHandler.h"
#include "CombatDataBehaviour.h"

namespace Crucible
{
	class CombatValueOwner;
	class QueryContext;
	
	struct DodgeInfo;
	struct HitInfo;
	enum class StrikeResult;



	enum class ActionPointFailureReason
	{
		NA,
		OnCooldown,
		UnderRequirement,
		InActionPoint,
		ErrorState,
		ErrorSettings,
		FatalError//Generic, but it means some core basic part is missing. Associated with owner
	};

	struct ActionPointData
	{
	private:
		ActionState currentState = ActionState::Ready;

		//If the stat pulled is generic or the action registered is 0, this will be generic
		// A value of 0x00FFFFFF is likely gonna mean something like, no cooldown incase this uses spells
		// A value of 0x00FFFFFE when submitted via script will mean successCount irrelevant. Which is rare but an option.
		// Storage wise, all loaded setting here should fit into a single uint theoretically speaking. but with the floats
		// they'll have to be rounded, which I don't mind, makes storage easier.
		std::uint32_t cooldownLocation;
		//I want the above to work differently, it should store the pure value, and then have a bool that forces it to be read as zero
		// This way, I don't need the cooldown location, and the action point id. It will come in handy later.

		bool genericType = false;

		std::uint8_t successCount = 0;
		std::uint8_t successRequirement = 0;

		float storedValue = 0;

		float cooldownTime = 0;

		friend class ActionPointController;
	};



	//NOTICE, IsOnCooldown, it should be if it returns true, it will remove that entry.
	// Second, when reloading this should reject serializing timers that are finished, this is a good time to use the serial wrapper stuff.
	class ActionPointController : 
		public CombatDataBehaviour,
		public ITimerCallback, 
		public SerialComponent<ActionPointData>
	{
	protected:
		static const int k_genericLength = 3;
		
		Timer activeTimer = Timer::Local();

		

		ActionState& currentState = data.currentState;

		//If the stat pulled is generic or the action registered is 0, this will be generic
		// A value of 0x00FFFFFF is likely gonna mean something like, no cooldown incase this uses spells
		// A value of 0x00FFFFFE when submitted via script will mean successCount irrelevant. Which is rare but an option.
		// Storage wise, all loaded setting here should fit into a single uint theoretically speaking. but with the floats
		// they'll have to be rounded, which I don't mind, makes storage easier.
		std::uint32_t& cooldownLocation = data.cooldownLocation;
		//I want the above to work differently, it should store the pure value, and then have a bool that forces it to be read as zero
		// This way, I don't need the cooldown location, and the action point id. It will come in handy later.

		bool& genericType = data.genericType;

		std::uint8_t& successCount = data.successCount;
		std::uint8_t& successRequirement = data.successRequirement;

		float& storedValue = data.storedValue;

		float& cooldownTime = data.cooldownTime;

		//Timer genericCooldowns[k_genericLength];
		SerialVector<Timer> genericCooldowns = SerialVector<Timer>(k_genericLength);

		//std::map<std::uint32_t, Timer> actionCooldowns;
		SerializableMap<std::uint32_t, Timer> actionCooldowns;

	public:
		
		//ActionPointController() {}
		
		//Make a merge of dodge and guard, just with more settings. But keep these definitions.
		ActionPointFailureReason	StartGuardWithReason(ActionPoint action, float evalValue = 0);

		ActionPointFailureReason	StartDodgeWithReason(ActionPoint action, float evalValue = 0);

		bool StartGuard(ActionPoint action, float evalValue = 0) { return StartGuardWithReason(action, evalValue) == ActionPointFailureReason::NA; }
		bool StartDodge(ActionPoint action, float evalValue = 0) { return StartDodgeWithReason(action, evalValue) == ActionPointFailureReason::NA; }


		void StopAction(bool forceFailure = false);
		
		//Need a version of this function that will effectively only stop 1 type of action
		//
		bool	StopAction(std::uint32_t number, bool forceFailure = false);
		bool	StopAction(ActionPoint ap, bool forceFailure = false);



		void	InvokeCooldown(std::uint32_t cd_loc, float cd_time);
		int		GetGeneralCooldown();
		bool	IsOnCooldown(std::uint32_t cd_loc);
		bool	IsOnCooldown(ActionPoint ap) { return IsOnCooldown(static_cast<std::uint32_t>(ap)); }

		bool IsGuarding() { return currentState == ActionState::Guard; }
		bool IsDodging() { return currentState == ActionState::Dodge; }
		bool IsInAction() { return currentState != ActionState::Ready; }


		void FlagSuccess(unsigned int worth = 1, ActionState state = ActionState::Ready)
		{
			//If the type is ready, it will only do current. Use a specific type if you don't know if it's active or not.
			worth = __max(worth, 1);
			if (state == ActionState::Ready || state == currentState){//currentState != ActionState::Ready && (state == ActionState::Ready || state == currentState) ){
				//Utility::DebugMessageBox(std::format("success {} += {}", successCount, worth), IsPlayerRef());
				successCount += worth;
			}
		}

		float GetStoredValue() { return storedValue; }

		void OnTimerFinish(Timer* timer_source, float total_time) override;

		//Will return generic if it's invalid.
		ActionPoint GetActionPoint();

		std::uint32_t GetTrueActionPoint();


	protected:
		void Clear();


		void Initialize() override;

		void SerializeData(SerialArgument& serializer, bool& success) override
		{
			serializer.Serialize(activeTimer);
			serializer.Serialize(genericCooldowns);
			serializer.Serialize(actionCooldowns);
		}

		void OnInterrupt()
		{
			StopAction(true);
		}

		void OnContactQuery(QueryContext& context, StrikeResult& restrictFlags, CombatValueOwner* attacker);

		void HandleStrikeResult(StrikeResult result, float value, bool is_dodge);

		void OnContact(QueryContext& context, DodgeInfo& result_info, CombatValueOwner* attacker);
		void OnHit(QueryContext& context, HitInfo& result_info, CombatValueOwner* attacker);

		ActionPointFailureReason ActionSetup(ActionPoint action, ActionState state, float evalValue);

		//Internal thing. Also used for scripted dodges. Comes with context so it works for special actions. Don't know about that
		// for now though.
		ActionPointFailureReason StartAction(ActionState newState, bool isGeneric, float evalValue, std::uint32_t cd_loc, float activeTime, float cd_time, float req = 0, float bonus = 0, std::uint8_t successReq = 1);
	};
}