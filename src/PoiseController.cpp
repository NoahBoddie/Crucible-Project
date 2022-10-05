#pragma once
#include "Utility.h"
#include "CombatData.h"
#include "PoiseController.h"
#include "RecoveryController.h"
#include "EventSystem.h"
//#include "CombatResource.h"
//#include "CombatDataBehaviour.h"

namespace Crucible
{
	//This function should include WHO is doing the breaking, like handle health damage does.
	void PoiseController::DamagePoise(float value, float& regenDamage, bool allowBreak)
	{
		//Regen damage is used to return how much regen damage was actually done.
		auto state = GetCurrentState();
		
		if (state == PoiseState::Broken)
			return;//Poise broken, head empty, no damagio
		else if (state != PoiseState::Recovering)
		{
			float poiseValue = owner->GetResource(CombatResource::Poise);
			/*
			if (poiseValue <= 0)
			{
				if (allowBreak)
				{
					//do the break poise ting
					if (owner->IsPlayerRef() == false)
						Utility::DebugMessageBox("Oh no, poise broke ;_;");
					
					return;
				}
				else
				{

				}
			}
			else
			//*/
			//We want to do damage, if the person is above 0. If the new number is zero, and allow break is enabled
			// should they be exposed, we break. If not, we expose.
			//If they are already below or at 0, should they be exposed, we break, if they are not exposed we expose.

			if (poiseValue > 0)
			{
				owner->DamageResource(CombatResource::Poise, value);
				
				poiseValue -= value;
				poiseValue = __max(poiseValue, 0);

				//if (owner->IsPlayerRef() == false) {
					//poiseValue = owner->GetResource(CombatResource::Poise);
					//Utility::DebugNotification(std::format("({}), {} poise left", value, poiseValue).c_str(), IsPlayerRef());
				//}
			}

			if (poiseValue <= 0)
			{
				if (GetCurrentState() == PoiseState::Exposed)
				{
					if (allowBreak)
					{
						//GoToState(PoiseState::Broken);
						BreakPoise();
						RE::DebugMessageBox(std::format("{}'s poise broke", NULL_CONDITION_RET(GetActorOwner(), "Unknown")->GetName()));
					}
				}
				else
				{
					GoToState(PoiseState::Exposed);
					//if (owner->IsPlayerRef() == false)
					RE::DebugMessageBox(std::format("{}'s poise is exposed...", NULL_CONDITION_RET(GetActorOwner(), "Unknown")->GetName()));
					//Utility::DebugNotification("EXPOSED....", true);
				}
			}

			
		}
		else
		{
			//I'm capping this for now, poise calculation is so fucking busted it's not really funny.
			regenDamage = fmin(value, 20);
			if (owner->IsPlayerRef() == false)
				Utility::DebugNotification(std::format("recovery set back to {} seconds", regenDamage).c_str());
		}

		auto curDelay = owner->recoverControl.GetRegenDelay(0, RecoveryType::CombatResource);

		//IF exposed perhaps the regen on this is less frightening for heavy weapons. Because, as you know
		// that shit can be like 8 seconds.

		if (curDelay < regenDamage)
			owner->recoverControl.SetRegenDelay(0, RecoveryType::CombatResource, regenDamage);
		else
			regenDamage = 0;
	}

	void PoiseController::BreakPoise() 
	{
		float k_Temp_PoiseBrokeResource = 15;//No idea how long poise gets broken for, at least for now.
		owner->recoverControl.SetRegenDelay(0, RecoveryType::CombatResource, k_Temp_PoiseBrokeResource);
		auto actor = GetActorOwner();

		static constexpr float k_temp_poiseBreakMagnitude = 1.f;

		//STAGGER_SWAP

		//NULL_CONDITION(actor)->SetGraphVariableFloat("staggerMagnitude", k_temp_poiseBreakMagnitude);
		//NULL_CONDITION(actor)->NotifyAnimationGraph("staggerStart");
		GoToState(PoiseState::Broken);
	}

	void PoiseController::OnRecoverStart(int value)
	{
		//Legacy setup
		//int value;
		//if (!params[0].AsInt(value) || (sign_bit | (int)CombatResource::Poise) != value)
		//	return;

		if ((sign_bit | (int)CombatResource::Poise) != value)
			return;

		//If the state is exposed, we go to normal. If the state is BROKEN we go to recovering. Else nothing.

		auto currState = GetCurrentState();

		Utility::DebugNotification("poise regen: start", IsPlayerRef());

		timestamp = TimeManager::GetRuntime();
		switch (currState)
		{
		//case PoiseState::Exposed:
		//	GoToState(PoiseState::Normal);
		//	return;
		case PoiseState::Broken:
			GoToState(PoiseState::Recovering);
			
		default:
			return;
		}
	}

	void PoiseController::OnRecoverFinish(int value, RecoveryFinishReason reason)
	{
		//int value;
		//RecoveryFinishReason reason;

		//if (!params[0].AsInt(value) || (sign_bit | (int)CombatResource::Poise) != value)
		//	return;
		//last part is temp
		//if (!params[1].AsEnum(reason) || (reason != RecoveryFinishReason::maxValue && reason != RecoveryFinishReason::exhaustPool))
		//	return;

		if ((reason != RecoveryFinishReason::maxValue && reason != RecoveryFinishReason::exhaustPool) || (sign_bit | (int)CombatResource::Poise) != value)
			return;

		auto currState = GetCurrentState();
		//if (owner->IsPlayerRef() == false)
		Utility::DebugNotification(std::format("poise regen: finish @ {}", TimeManager::GetRuntime() - timestamp), IsPlayerRef());

		switch (currState)
		{
		case PoiseState::Exposed:
		//case PoiseState::Broken:
		case PoiseState::Recovering:
			GoToState(PoiseState::Normal);

		default:
			return;
		}
	}


	void PoiseController::Initialize()
	{
		CreateMemberEvent("OnRecoveryStart", &PoiseController::OnRecoverStart);
		CreateMemberEvent("OnRecoveryFinish", &PoiseController::OnRecoverFinish);
	}

	void PoiseController::HandleChange(bool apply)
	{
		int mod = apply ?  1 : -1;

		owner->resist ^= CombatValueInput('PZE', ValueInputType::Set, -0.5f, apply);//-0.5f * mod;
		owner->evade %= CombatValueInput('PZE', ValueInputType::Set, -0.25f, apply);//-0.25f * mod;
	}

	void PoiseController::OnStateBegin()
	{
		auto cState = GetCurrentState();
		auto pState = GetPreviousState();
		//Don't like this implementation
		if (cState == PoiseState::Broken)
		{
			HandleChange(true);
		}
			
		else if (cState == PoiseState::Normal && pState != PoiseState::Exposed)
		{
			HandleChange(false);
		}
	}

}