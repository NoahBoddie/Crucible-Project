#pragma once

#include "AttackStatController.h"
#include "CombatData.h"
#include "Utility.h"
#include <cmath>
#include "CollisionIgnoreHandler.h"

namespace Crucible
{
	bool AttackStatController::EvaluateState(uATTACK_ENUM attackState)
	{//Can be override, otherwise does nothing.
		if (storedAttackEnum == attackState)
			return false;

		storedAttackEnum = attackState;

		auto cState = GetCurrentState();
		
		//For multi hit attacks, swing is done multiple times with the hit following up. Take major note of this
		// to prevent it from having issues with hit accounting.

		//For the motion state control stuff, make that sort of stuff an event



		switch (attackState)
		{
		case uATTACK_ENUM::kNone:
			if (owner->meleeCombo.count)
				owner->meleeCombo.StartEndComboWindow();
			//Once registered, this should check for failure. getting hit out of your stuff mid swing
			// should result in failure. Maybe? Cause if it hasn't gone to hit yet you know
			//owner->swingEvent.TryFailure(Utility::GetAttackingWeapon(GetActorOwner()));
			CollisionIgnoreHandler::ClearIgnoreList(GetActorOwner());
			
			owner->motionControl.SetLockState(false);

		case uATTACK_ENUM::kBash:
			return cState != GoToState(AttackState::kNone);
			
		case uATTACK_ENUM::kDraw:
			owner->meleeCombo.IncrementCombo();
			owner->motionControl.SetLockState(true);
			CollisionIgnoreHandler::ClearIgnoreList(GetActorOwner());
			return cState != GoToState(AttackState::kPrehitCalc);

		case uATTACK_ENUM::kSwing:
			owner->swingEvent.Register();
			return true;

		case uATTACK_ENUM::kHit:
			owner->swingEvent.TryFailure(Utility::GetAttackingWeapon(GetActorOwner()));
			return cState != GoToState(AttackState::kHitCalc);

		case uATTACK_ENUM::kNextAttack:
		case uATTACK_ENUM::kFollowThrough:
			//I want a section here, or in hit for can increment. This should be the thing that prevents
			// a character from incrementing a combo before actually going to the hit state.
			owner->motionControl.SetLockState(false);
			return cState != GoToState(AttackState::kPosthitCalc);

		}
		
		

		return false;
	}


	void AttackStatController::OnStateBegin()
	{

		HandleChange(GetPreviousState(), false);
		HandleChange(GetCurrentState(), true);

		//float stagger = owner->stagger.DeleteLater();
		//float resist = owner->resist.DeleteLater();
		//float evade = owner->evade.DeleteLater();
		//std::string printString = std::format("[ {} ]: resist({}), stagger({}), evade: ({}), state ({})",
		//	GetActorOwner()->GetName(), resist, stagger, evade, (int)GetCurrentState());

		//RE::DebugNotification(printString.c_str());
		//print if you can.
	}

	//Currently, this cannot handle the concept of power attacks, or individuals
	// who are currently using completely different weapons.
	void AttackStatController::HandleChange(AttackState _state, bool assign)
	{
		if (_state == AttackState::kNone)//and you aren't powerattacking
			return;

		std::int8_t mod = assign ? 1 : -1;
		//I am currently unsure if I will need some for of ValueEffect for this,
		// because it was that I wanted to handle the reductions differently.
		float apply = 0.f;
		
		//This should use get attacking weapon instead of this. That can get shields as well.
		// And instead of using it's own stats here, it should be using the itemStats.

		if (assign)
		{
			switch (owner->GetBestAttackClass())
			{
			case WeightClass::kLight:
				apply = 1;
				break;
			case WeightClass::kMedium:
				apply = 2;
				break;
			case WeightClass::kHeavy:
				apply = 3;
				break;
			}
			
			switch (_state)

			{
			case AttackState::kHitCalc:
			case AttackState::kPosthitCalc:
				if (Utility::IsPowerAttacking(GetActorOwner()) == true)
				apply /= 2;
			}
			
		}
		else
		{
			apply = resistTaken;
		}
		

		owner->resist -= apply * mod;
		resistTaken += apply * mod;

		
	}

}