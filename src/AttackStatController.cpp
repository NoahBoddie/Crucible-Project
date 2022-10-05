#pragma once

#include "AttackStatController.h"
#include "CombatData.h"
#include "Utility.h"
#include <cmath>
#include "CollisionIgnoreHandler.h"

namespace Crucible
{
	void AttackStatController::BeforeCombatGraphEvent(EventArgument& argument, std::string anim_event, RE::BGSAttackData* attack_data)
	{
		logger::info("{} name, attack data id {}", anim_event, attack_data->data.flags.underlying());

		auto actor = GetActorOwner();

		if (!actor)
			return;

		//Bash is not our problem.
		if (attack_data->data.flags.all(AttackDataFlags::kBashAttack) == true) {
			return;
		}

		if (attack_data->data.flags.all(AttackDataFlags::kPowerAttack) == true)
		{
			if (actor->HasMagicEffect(Resources::paCooldown) == true) {
				argument.control = EventControl::kStop;
				logger::info("power attack robbed");
			}
		}

		else if (owner->meleeCombo.IsActive() && actor->GetActorValue(RE::ActorValue::kStamina) <= 0) {
			argument.control = EventControl::kStop;
			logger::info("no more stamina");
		}
	}

	void AttackStatController::OnUpdate(RE::Actor* self, float delta)
	{
		uATTACK_ENUM attackState = self->GetAttackState();

		if (storedAttackEnum == attackState)
			return;

		storedAttackEnum = attackState;

		auto cState = GetCurrentState();

		//For multi hit attacks, swing is done multiple times with the hit following up. Take major note of this
		// to prevent it from having issues with hit accounting.

		//For the motion state control stuff, make that sort of stuff an event



		switch (attackState)
		{
		case uATTACK_ENUM::kNone:
			if (owner->meleeCombo.GetCount() != 0)
				owner->meleeCombo.ResolveCombo();
			//Once registered, this should check for failure. getting hit out of your stuff mid swing
			// should result in failure. Maybe? Cause if it hasn't gone to hit yet you know
			//owner->swingEvent.TryFailure(Utility::GetAttackingWeapon(GetActorOwner()));
			CollisionIgnoreHandler::ClearIgnoreList(GetActorOwner());

			owner->motionControl.SetLockState(false);

		case uATTACK_ENUM::kBash:
			GoToState(AttackState::kNone);
			return;

		case uATTACK_ENUM::kDraw:
			owner->meleeCombo.ComboIncrease();
			owner->motionControl.SetLockState(true);
			CollisionIgnoreHandler::ClearIgnoreList(GetActorOwner());
			GoToState(AttackState::kPrehitCalc);
			return;

		case uATTACK_ENUM::kSwing:
			owner->meleeCombo.Register();
			return;

		case uATTACK_ENUM::kHit:
			owner->meleeCombo.TryFailure(Utility::GetAttackingWeapon(GetActorOwner()));
			GoToState(AttackState::kHitCalc);
			return;

		case uATTACK_ENUM::kNextAttack:
		case uATTACK_ENUM::kFollowThrough:
			//I want a section here, or in hit for can increment. This should be the thing that prevents
			// a character from incrementing a combo before actually going to the hit state.
			owner->motionControl.SetLockState(false);
			GoToState(AttackState::kPosthitCalc);
			return;

		}
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

		//Utility::DebugNotification(printString.c_str());
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
			//Make this a weapon stat
			switch (owner->GetBestAttackClass())
			{
			case WeightClass::kLight:
				apply = 1;
				break;
			case WeightClass::kMedium:
				apply = 1.5f;
				break;
			case WeightClass::kHeavy:
				apply = 2.0f;
				break;
			}
			
			switch (_state)

			{
			case AttackState::kHitCalc:
			case AttackState::kPosthitCalc:
				if (Utility::IsPowerAttacking(GetActorOwner()) == false)
				apply /= 2.0f;
			}
			
		}
		else
		{
			//apply = resistTaken;
			//apply = 1000.f;//Setting this is no longer needed.
		}
		

		//owner->resist -= apply * mod;
		//resistTaken += apply * mod;
		//I think I will just set this.
		owner->resist -= CombatValueInput('ATK', ValueInputType::Set, apply, assign);
		
	}

}