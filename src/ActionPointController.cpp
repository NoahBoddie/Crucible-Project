#pragma once

#include "ActionPointController.h"
#include "ModSettingHandler.h"
#include "CombatData.h"
#include "AttackInfo.h"

namespace Crucible
{
	ActionPointFailureReason ActionPointController::StartGuardWithReason(ActionPoint action, float evalValue)
	{
		if (currentState != ActionState::Ready)
			return ActionPointFailureReason::InActionPoint;

		return ActionSetup(action, ActionState::Guard, evalValue);
	}

	ActionPointFailureReason ActionPointController::StartDodgeWithReason(ActionPoint action, float evalValue)
	{
		if (currentState != ActionState::Ready) {
			RE::DebugNotification("not ready");
			return ActionPointFailureReason::InActionPoint;
		}
		return ActionSetup(action, ActionState::Dodge, evalValue);
	}

	void ActionPointController::StopAction(bool forceFailure)
	{
		if (currentState == ActionState::Ready)
			return;

		if (!successRequirement || successCount < successRequirement || forceFailure) {
			InvokeCooldown(genericType ? 0 : cooldownLocation, cooldownTime);
		
		}
		else{
			Utility::DebugNotification("Action Point Successful", IsPlayerRef());
		}

		Clear();
	}

	bool ActionPointController::StopAction(std::uint32_t number, bool forceFailure)
	{ 
		if (number && GetTrueActionPoint() == number) {
			StopAction(forceFailure);
			return true;
		}

		return false;
	}
	
	bool ActionPointController::StopAction(ActionPoint ap, bool forceFailure)
	{ 
		return StopAction(static_cast<std::uint32_t>(ap), forceFailure); 
	}


	void ActionPointController::Initialize()
	{
		activeTimer.AddCallback(this);
		//CreateMemberEvent("OnInterrupt", &ActionPointController::OnInterrupt);//currently, interrupt is being by OnContact/OnHit
		CreateMemberEvent("OnContact", &ActionPointController::OnContact);
		CreateMemberEvent("OnHit", &ActionPointController::OnHit);
		CreateMemberEvent("OnContactQuery", &ActionPointController::OnContactQuery);
	}

	void ActionPointController::OnTimerFinish(Timer* timer_source, float total_time)
	{
		StopAction();
	}

	void ActionPointController::OnContactQuery(QueryContext& context, StrikeResult& restrictFlags, CombatValueOwner* attacker)
	{
		if (!attacker || IsInAction() == true || IsOnCooldown(ActionPoint::autoDodge) == true)
			return;

		//Fuck it, if there is no actor let it die.
		auto actor = GetActorOwner();


		if (actor->IsPlayerRef() == false && actor->IsWeaponDrawn() == false)
			return;

		//Considering edge cases.
		if (actor->GetAttackState() != uATTACK_ENUM::kNone || Utility::IsCasting(actor) == true)
			return;

		bool boolean = false;

		if (actor->GetGraphVariableBool("IsBlocking", boolean) && boolean) {
			return;
		}


		float evade = owner->GetEvade(context, attacker->GetPortList());

		context.SetFloat(queryPostEvade, evade);


		if (StartDodge(ActionPoint::autoDodge, evade) == false)
			return;
		else if (context.GetBool(queryPowerAction) == true)
			restrictFlags |= StrikeResult::allAttack;
		else
			restrictFlags |= StrikeResult::exceptMinorDefend;

		//Utility::DebugNotification("*auto dodge", IsPlayerRef());
	}

	static constexpr inline unsigned short int k_temp_minor_success = 1;
	static constexpr inline unsigned short int k_temp_major_success = 5;
	static constexpr inline float k_temp_minor_bonus = 2;
	static constexpr inline float k_temp_major_bonus = 2;
	//Include one more value that makes it for every value above, it will do that bit more.
	//So percent for minor works up to upto 2x as much, difference would be like an extra 2 for each difference for major.



	void ActionPointController::HandleStrikeResult(StrikeResult result, float value, bool is_dodge)
	{
		int addon = 0;

		bool force_failure = false;

		switch (result)
		{
		case StrikeResult::majorAttack:
			force_failure = true;
		case StrikeResult::minorAttack:
			if (is_dodge ? IsDodging() : IsGuarding()) {
				//Utility::DebugMessageBox("Knocked out of dodge", IsPlayerRef());
				StopAction(force_failure);
			}
			break;

		case StrikeResult::minorDefend:
			addon = floor(value * k_temp_minor_bonus);
			FlagSuccess(k_temp_minor_success + addon, is_dodge ? ActionState::Dodge : ActionState::Guard);
			break;

		case StrikeResult::majorDefend:
			addon = floor(value * k_temp_major_bonus);
			FlagSuccess(k_temp_major_success + addon, is_dodge ? ActionState::Dodge : ActionState::Guard);
			break;
		}
	}


	void ActionPointController::OnContact(QueryContext& context, DodgeInfo& result_info, CombatValueOwner* attacker)
	{
		if (StopAction(ActionPoint::autoDodge, true) == true)
			return;


		StrikeResult result = result_info.result;
		
		float value = result_info.value;


		return HandleStrikeResult(result, value, true);

		bool force_failure = false;

		switch (result)
		{
		case StrikeResult::majorAttack:
			force_failure = true;
		case StrikeResult::minorAttack:
			if (IsDodging() == true) {
				//Utility::DebugMessageBox("Knocked out of dodge", IsPlayerRef());
				StopAction(force_failure);
			}
			break;

		case StrikeResult::minorDefend:
			FlagSuccess(k_temp_minor_success, ActionState::Dodge);
			break;

		case StrikeResult::majorDefend:
			FlagSuccess(k_temp_major_success, ActionState::Dodge);
			break;
		}
	}




	void ActionPointController::OnHit(QueryContext& context, HitInfo& result_info, CombatValueOwner* attacker)
	{
		StrikeResult result = result_info.result;
		
		float value = result_info.value;

		return HandleStrikeResult(result, value, false);

		bool force_failure = false;

		switch (result)
		{
		case StrikeResult::majorAttack:
			force_failure = true;

		case StrikeResult::minorAttack:
			//Should guard points only be able to stop minor actions or major ones too? Major is always a reward, it'd be a shame to have it swallowed in one of these.
			//If this is resist decrement. If that reaches it's end, stop the action.
			//For now, we just straight up stop it.
			if (IsGuarding() == true) {
				//Utility::DebugMessageBox("Knocked out of guarding", IsPlayerRef());
				StopAction(force_failure);
			}
			break;

		case StrikeResult::minorDefend:
			FlagSuccess(k_temp_minor_success, ActionState::Guard);
			break;

		case StrikeResult::majorDefend:
			FlagSuccess(k_temp_major_success, ActionState::Guard);
			break;
		}
	}


	void ActionPointController::InvokeCooldown(std::uint32_t cd_loc, float cd_time)
	{
		if (cd_loc == 0)
		{
			//genericRegistry += 1;
			//dummy action for right now. Should have a lambda as well.
			// 
			//Timer* cdTimer = new Timer();

			for (int i = 0; i < k_genericLength; i++)
			{
				if (genericCooldowns[i].isActive() == false)
				{
					genericCooldowns[i].Start(cd_time);
					break;
				}
			}

			//*do generic set up.
		}
		else
		{
			actionCooldowns[cd_loc].Start(cd_time);

			//Do some other shit maybe?
		}
	}

	
	int ActionPointController::GetGeneralCooldown()
	{
		int result = 0;

		for (int i = 0; i < k_genericLength; i++)
		{
			if (genericCooldowns[i].isActive() == true)
				result++;
		}

		return result;
	}

	bool ActionPointController::IsOnCooldown(std::uint32_t cd_loc)
	{//returns an int to return the number of generic on cooldown.
		if (cd_loc == 0)
		{
			return GetGeneralCooldown() >= k_genericLength;
		}
		
		return actionCooldowns->contains(cd_loc) && actionCooldowns[cd_loc].isActive();
	}

	ActionPoint ActionPointController::GetActionPoint()
	{
		if (cooldownLocation <= 0 || cooldownLocation < (int)ActionPoint::customAction)
			return ActionPoint::customAction;

		return static_cast<ActionPoint>(cooldownLocation);
	}

	std::uint32_t ActionPointController::GetTrueActionPoint()
	{
		return cooldownLocation;
	}

	void ActionPointController::Clear()
	{
		activeTimer.Stop();

		currentState = ActionState::Ready;

		genericType = false;

		cooldownLocation = 0;

		successCount = 0;

		successRequirement = 0;

		storedValue = 0;

		cooldownTime = 0;
	}

	ActionPointFailureReason ActionPointController::ActionSetup(ActionPoint action, ActionState state, float evalValue)
	{
		//ActionStat* stat = ActionStatHandler::GetActionStat(action);
		ActionStat* stat = ModSettingHandler::GetSetting<ActionStat>(action);

		if (!stat) {
			return ActionPointFailureReason::ErrorSettings;
		}
		else if (stat->actionType != state) {
			RE::DebugNotification("no state or wrong type");
			return ActionPointFailureReason::ErrorState;
		}

		return StartAction(state, stat->isGeneric, evalValue, (std::uint32_t)action, stat->uptime, stat->cooldownTime, stat->requirement, stat->bonus, stat->successRequirement);
	}

	ActionPointFailureReason ActionPointController::StartAction(ActionState newState, bool isGeneric, float evalValue, std::uint32_t cd_loc, float activeTime, float cd_time, float req, float bonus, std::uint8_t successReq)//, QueryContext& context = QueryContext::empty())
	{
		if (IsOnCooldown(isGeneric ? 0 : cd_loc) == true) {
			RE::DebugNotification("cooldown hit");
			return ActionPointFailureReason::OnCooldown;
		}

		if (!owner)
			return ActionPointFailureReason::FatalError;

		if (evalValue == 0)
		{
			//Never do this, it comes with no context.
			switch (newState)
			{
			case ActionState::Dodge:
				evalValue = owner->GetEvade();//context);
				break;
			case ActionState::Guard:
				evalValue = owner->GetResist();//context);
				break;
			}
		}
		if (req > 0 && evalValue < req)
			return ActionPointFailureReason::UnderRequirement;

		activeTimer.Start(activeTime);

		currentState = newState;
		cooldownLocation = cd_loc;
		genericType = isGeneric;
		successCount = 0;
		successRequirement = successReq;
		storedValue = fmax(evalValue + bonus, 0);

		cooldownTime = cd_time;

		if (cd_loc != (int)ActionPoint::autoDodge)
			SendOwnerEvent("OnAction");

		return ActionPointFailureReason::NA;
	}

}