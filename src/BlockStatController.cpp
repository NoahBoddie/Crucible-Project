#pragma once

#include "CombatData.h"
#include "RecoveryController.h"
#include "BlockStatController.h"

namespace Crucible
{


	void BlockStatController::OnStateFinish()
	{


		switch (GetCurrentState())
		{
		//For both of these I want to stop the timer.
		case GuardState::kStrongGuard:
		{
			auto state = GetGoingToState();//is optional
			if (state == GuardState::kWeakGuard) {
				SetGoingState(GuardState::kStrongGuard);
			}
			break;
		}
			
		
		case GuardState::kWeakGuard:

			RE::DebugNotification("The guard strengthens...");
			staminaTimer.Resume();
			break;
		case GuardState::kGuardBreak:
			RE::DebugNotification("You feel you can guard again...");
			malusTimer.Stop();
			break;
		}
	}
	void BlockStatController::OnStateBegin()
	{
		switch (GetCurrentState())
		{
			//For both of these I want to stop the stamina timer.
		case GuardState::kWeakGuard:
			//This might get missed if its starting out. recitify issue.
			staminaTimer.Pause();
			break;

		case GuardState::kNone:
		case GuardState::kGuardBreak:
			ClearStaminaUpkeep();
			break;
		}
	}

	void BlockStatController::ReimburseAction()
	{
		RE::Actor* actorOwner = GetActorOwner();

		if (!actorOwner)
			return;

		float immediateReturn = fmin(raiseStamina * 0.5f, actionStamina);

		actionStamina -= immediateReturn;

		actorOwner->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, immediateReturn);

		if (actionStamina > 0)
			owner->recoverControl.SetRegenPool(static_cast<std::int32_t>(RE::ActorValue::kStamina), RecoveryType::ActorValue, actionStamina);
	}


	void BlockStatController::OnBlockStageIncrement()
	{
		auto actor = GetActorOwner();

		if (!actor) {
			return;
		}

		auto b_form = owner->GetBlockingForm(false);

		if (b_form && Utility::GetDefendingClass(b_form->As<RE::TESObjectARMO>()) == WeightClass::kHeavy) {
			GoToState(GuardState::kStrongGuard);
		}
	}

	void BlockStatController::GoToGuardStart(RE::TESForm* b_form, bool metStaminaReq)
	{
		//If you're a heavy shield, I will not stack the time for penalty and for start up
		// but the fact remains one is a penalty, so what I'll do is we go with whatever is greater
		// conversely, I could make it so heavy shield has a "buff" in that the weakness lasts a certain
		// amount of time always.

		if (!metStaminaReq)
			RE::DebugMessageBox("Didn't have enough stams didjya");

		float weakGuardTime = metStaminaReq ? 0 : 5;//Will be dynamic later on.

		if (b_form && Utility::GetDefendingClass(b_form->As<RE::TESObjectARMO>()) == WeightClass::kHeavy) {
			weakGuardTime = fmax(4, weakGuardTime);
		}

		if (weakGuardTime > 0) {
			WeakenGuard(weakGuardTime);
		}
		else
			GoToState(GuardState::kGuard);
	}

	bool BlockStatController::TryRaise()
	{
		//if we have stamina, and aren't guard crushed, raise shield. Expend stamina.
		// If we can't expend all, go to weak guard, set time off by how far behind we are.
		auto state = GetCurrentState();

		switch (state)
		{
		default:
			return false;
		case GuardState::kNone:
			break;
		}

		auto actor = GetActorOwner();

		if (!actor) {
			return false;
		}

		float actorStamina = actor->GetActorValue(RE::ActorValue::kStamina);

		auto defendForm = owner->GetBlockingForm(true);//will be needed later.

		float takeValue = k_temp_RaiseReq;

		bool metReq = true;

		if (actorStamina < takeValue)
		{
			takeValue = actorStamina;
			metReq = false;
		}

		GoToGuardStart(defendForm, metReq);


		actor->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, -takeValue);
		raiseStamina = takeValue;

		return true;
	}


	void BlockStatController::TryBashingBlock(RE::Actor* actor)
	{
		if (!actor) {
			actor = GetActorOwner();

			if (!actor) return;
		}

		auto b_form = owner->GetBlockingForm(false);//nullptr is fine, it'll go quicker that way.

		if (b_form && Utility::GetDefendingClass(b_form->As<RE::TESObjectARMO>()) == WeightClass::kMedium) {
			RE::DebugNotification("Parry Activated");

			actor->SetGraphVariableBool("Isblocking", true);
		}
	}

	void BlockStatController::UpdateStamina(float updateDelta, RE::Actor* actor)
	{
		if (InGuardableState() == false)
			return;

		auto oldStage = stage;

		if (staminaTimer.isEnabled() == false) {
			staminaTimer.Start(k_temp_UpkeepTimer);
		}
		else if (staminaTimer.isFinished() == true) {
			stage++;

			if (oldStage != stage) {
				staminaTimer.Start(k_temp_UpkeepTimer);
				RE::DebugMessageBox(std::format("{} stage", (int)stage));

			}
		}

		if (!actor)
			actor = GetActorOwner();

		auto defendForm = owner->GetBlockingForm(false);

		float weight = 5;

		if (defendForm)
			weight = fmin(defendForm->GetWeight(), weight);

		float upkeepDelta = weight * updateDelta;

		switch (stage)
		{
		case Stage0:
			//Does nothing.
			break;
		case Stage2:
			actor->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, -k_temp_UpkeepCost_second * upkeepDelta);
		case Stage1:
			//For this bit, only this amount is refundable.
			actor->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, -k_temp_UpkeepCost_first * upkeepDelta);
			IncrementActionStamina(k_temp_UpkeepCost_first * upkeepDelta);
			//This needs to make an important distiction in only taking the amount ACTUALLY spent.
			if (CanForceCancel(actor) == true && actor->GetActorValue(RE::ActorValue::kStamina) <= 0)
			{
				DropGuard();
			}
		}

	}


}