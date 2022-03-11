#pragma once

#include "Timer.h"
#include "Utility.h"
#include "StatedObject.h"
#include "CombatDataBehaviour.h"

namespace Crucible
{
	enum class GuardState 
	{
		kNone, 
		kGuard,
		kWeakGuard,
		kStrongGuard, 
		kGuardBreak 
	};

	enum BlockStage { Stage0, Stage1, Stage2, StageTotal };

	
	inline BlockStage& operator++(BlockStage& stage, int)
	{
		switch (stage) {
		case Stage0: 
			return stage = Stage1;
		case Stage1:
			return stage = Stage2;
		}

		return stage;

		/*
		int inc = 1;

		while (inc-- >= 0)
		{
			switch (stage) {
			case Stage0:
				stage = Stage1;
				break;
			case Stage1:
				stage = Stage2;
				break;
			}

			break;
		}
		return stage;
		//*/
	}


	class BlockStatController :
		public StatedObject<GuardState, float, RE::Actor*>,
		public CombatDataBehaviour
	{
	private:
		void GoToGuardStart(RE::TESForm* b_form, bool metStaminaReq);
		
		bool CheckGuarding(RE::Actor* actor = nullptr)
		{
			//As a precaution, if not already in a guarding state this should likely not go through.

			if (!actor) {
				actor = GetActorOwner();

				if (!actor) return false;
			}

			bool result;
			actor->GetGraphVariableBool("isBashing", result);

			if (result)
				return true;


			int i_result = 0;
			actor->GetGraphVariableInt("iState", i_result);

			int blockStand = 0;
			int blockCharge = 0;
			
			actor->GetGraphVariableInt("iState_NPCBlocking", blockStand);

			if (i_result == blockStand)
				return true;
			
			actor->GetGraphVariableInt("iState_NPCBlockingShieldCharge", blockCharge);

			if (i_result == blockCharge)
				return true;

			return false;

			//There are a few ways to measure this, if ward power exists, or if the warding state is absorbed.
		//Additionally, get blocking item should use wards if I can use it.
			bool warding = false;

			if (!actor || !actor->currentProcess || !actor->currentProcess->middleHigh)
				warding = actor->currentProcess->middleHigh->wardState == uWardState::kAbsorb;

			return warding;
		}

	public:
		
		//This is used for passive cancellations, not stuff like guard breaking.
		bool CanForceCancel(RE::Actor* actor = nullptr)
		{
			if (!actor) {
				actor = GetActorOwner();

				if (!actor) return true;
			}

			bool result;
			return !actor->GetGraphVariableBool("isBashing", result) || !result;

		}


		bool EvaluateState(float updateDelta, RE::Actor* actor) override
		{
			auto state = GetCurrentState();

			switch (state)
			{
			case GuardState::kNone:
				return true;
			
			case GuardState::kWeakGuard:
				if (malusTimer.isFinished() == true)
					//Debating this sending you to whatever guard you need to be at.
					GoToState(GuardState::kGuard);
				break;

			case GuardState::kGuardBreak:
				if (malusTimer.isFinished() == true)
					GoToState(GuardState::kNone);
				break;

			default:
				//break;
				if (CheckGuarding(actor) == false) {
					LowerGuard();
					RE::DebugNotification("Out state detected");
				}
				else {
					UpdateStamina(updateDelta, actor);
				}
				break;
			}

			//COND: if character is in blocking state or warding state continue.
			//		 if not, invoke drop guard, no animation.

			//UpdateStamina(updateDelta, actor);
			//If this is the is the update that has to come for block, it will fire drop guard.
			// For this, make a toggle that will optionally send the event.

			return true;
		}

		void TryBashingBlock(RE::Actor* actor = nullptr);


		bool TryRaise();

		void OnBlockStageIncrement();

		void LowerGuard()
		{
			if (InGuardableState() == false)
				return;

			auto actor = GetActorOwner();

			if (!actor)
				return;

			//Really try doing this in a safer manner, this could maybe cause some issues elsewhere.
			actor->SetGraphVariableBool("Isblocking", false);

			//Used when the actor lowers their guard, seemingly voluntarily.
			ReimburseAction();
			ReimburseRaise();

			GoToState(GuardState::kNone);
		}

		void WeakenGuard(float time)
		{
			if (GoToState(GuardState::kWeakGuard) != GuardState::kWeakGuard)
				return;

			RE::DebugNotification("Your guard weakens...");

			malusTimer.Start(time);
		}

		void BreakGuard(float time)
		{
			if (GoToState(GuardState::kGuardBreak) != GuardState::kGuardBreak)
				return;

			malusTimer.Start(time);
		}

		//The effects of drop guard should be used in just about all other cases where someone 
		// drops their shield invuluntarily.
		void DropGuard(bool blockStop = true)
		{
			auto actor = GetActorOwner();

			if (!actor)
				return;

			ReimburseRaise();
			GoToState(GuardState::kNone);
			//Make this a constant somewhere maybe, somewhere in resources?
			if (blockStop)
				actor->NotifyAnimationGraph("blockStop");
		}
		void IncrementActionStamina(float value)
		{			
			if (InGuardableState() == false)
				return;

			actionStamina += value;
		}

		bool InGuardableState()
		{
			auto state = GetCurrentState();

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

	protected:

		const float k_temp_UpkeepTimer = 4.f;//I won't be making this unique but it might become a setting.

		//These are mults, they will be mults when we get them implemented. Based on weight or something.
		const float k_temp_UpkeepCost_first = 1.f;
		const float k_temp_UpkeepCost_second = 1.5f;//is additive.

		const float k_temp_RaiseReq = 30.f;


		Timer malusTimer;//In charge of weak guard state and guard crush state.
		Timer staminaTimer;

		float raiseStamina;
		float actionStamina;//Magicka will be managed elsewhere.

		BlockStage stage;

		void OnStateBegin() override;
		void OnStateFinish() override;

		//Use this first, recovers some value instantly based on raise.
		void ReimburseAction();

		void ReimburseRaise()
		{

			RE::Actor* actorOwner = GetActorOwner();

			if (!actorOwner)
				return;

			actorOwner->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, raiseStamina);
		}

		void ClearStaminaUpkeep()
		{
			stage = Stage0;
			raiseStamina = 0;
			actionStamina = 0;
			staminaTimer.Stop();
		}
		
		void UpdateStamina(float updateDelta, RE::Actor* actor = nullptr);
	};

}