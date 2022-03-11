#pragma once

#include "ActionPointController.h"
#include "ModSettingHandler.h"
#include "CombatData.h"

namespace Crucible
{
	bool ActionPointController::StartGuard(ActionPoint action, float evalValue)
	{
		if (currentState != ActionState::Ready)
			return false;

		return ActionSetup(action, ActionState::Guard, evalValue);
	}

	bool ActionPointController::StartDodge(ActionPoint action, float evalValue)
	{
		if (currentState != ActionState::Ready) {
			RE::DebugNotification("not ready");
			return false;
		}
		return ActionSetup(action, ActionState::Dodge, evalValue);
	}

	void ActionPointController::StopAction(bool forceFailure)
	{
		if (currentState == ActionState::Ready)
			return;

		if (!success || forceFailure)
			InvokeCooldown(cooldownLocation, cooldownTime);

		Clear();
	}


	void ActionPointController::Initialize()
	{
		activeTimer.AddCallback(this);
		CreateMemberEvent("OnInterrupt", owner, &ActionPointController::OnInterrupt);
	}

	void ActionPointController::OnTimerFinish(float totalTime)
	{
		StopAction();
	}

	void ActionPointController::InvokeCooldown(std::int32_t cd_loc, float cd_time)
	{
		if (cd_loc == 0)
		{
			//genericRegistry += 1;
			//dummy action for right now. Should have a lambda as well.
			// 
			//Timer* cdTimer = new Timer();

			for (int i = 0; i < k_genericLength; i++)
			{
				if (genericCooldowns[i].isEnabled() == false)
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

	bool ActionPointController::IsOnCooldown(std::int32_t cd_loc)
	{//returns an int to return the number of generic on cooldown.
		if (cd_loc == 0)
		{
			return GetGeneralCooldown() >= k_genericLength;
		}
		
		return actionCooldowns.contains(cd_loc) && actionCooldowns[cd_loc].isActive();
	}

	void ActionPointController::Clear()
	{
		activeTimer.Stop();

		currentState = ActionState::Ready;

		cooldownLocation = 0;

		success = false;

		storedValue = 0;

		cooldownTime = 0;
	}

	bool ActionPointController::ActionSetup(ActionPoint action, ActionState state, float evalValue)
	{
		//ActionStat* stat = ActionStatHandler::GetActionStat(action);
		ActionStat* stat = ModSettingHandler::GetSetting<ActionStat>(action);

		if (!stat || stat->actionType != state) {
			RE::DebugNotification("no state or wrong type");
			return false;
		}

		return StartAction(state, evalValue, stat->isGeneric ? 0 : (std::int32_t)action, stat->uptime, stat->cooldownTime, stat->requirement);
	}

	bool ActionPointController::StartAction(ActionState newState, float evalValue, std::int32_t cd_loc, float activeTime, float cd_time, float req)//, QueryContext& context = QueryContext::empty())
	{

		if (IsOnCooldown(cd_loc) == true) {
			RE::DebugNotification("cooldown hit");
			return false;
		}

		if (!owner)
			return false;

		if (evalValue == 0)
		{
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
			return false;

		activeTimer.Start(activeTime);

		currentState = newState;
		cooldownLocation = cd_loc;

		bool success = false;

		storedValue = evalValue;

		cooldownTime = cd_time;


		return true;
	}

}
