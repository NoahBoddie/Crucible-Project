#pragma once

#include "FormulaHandler.h"

#include "CombatData.h"
#include "CombatResource.h"//maybe?

#include "ItemStatHandler.h"

namespace Crucible
{
	float PoiseCalculation(float maxHP, RE::Actor* actor)
	{
		if (!actor)
			return -1;

		StatBase* raceStat = ItemStatHandler::GetStatBase(actor);

		//we do this because I want temporary health to factor larger in this.
		// NOTE, because of this, temporary value should not be included in the formula.
		float bonusHP = actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kHealth);

		float result = raceStat->GetValue(basePoise) + 
			maxHP * raceStat->GetValue(poiseHealthMult) +
			bonusHP * raceStat->GetValue(poiseBonusHealthMult);

		return result;
	};


	float FormulaHandler::UseFormular(FormulaType type, float value, RE::Actor* actor)
	{
		if (!actor) {
			//scream error
			return -1;
		}

		switch (type) {
		case FormulaType::NoFormular:
			return value;

		case FormulaType::PoiseCalculation:
			return PoiseCalculation(value, actor);

		default:
			//Scream error
			return -1;
		}

	}
}