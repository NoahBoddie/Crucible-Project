#pragma once
#include "CombatValue.h"
#include "ActorCombatValueOwner.h"
#include "ItemStatHandler.h"
#include "ItemStats.h"
#include <cmath>

namespace Crucible
{
	float ActorCombatValueOwner::GetStagger(RE::TESForm* a_form)
	{
		return __super::GetStagger(a_form);
	}

	float ActorCombatValueOwner::GetPrecision(RE::TESForm* a_form)
	{
		return __super::GetPrecision(a_form);
	}
	
	float ActorCombatValueOwner::GetResist()
	{
		return __super::GetResist();
	}

	float ActorCombatValueOwner::GetEvade()
	{
		return __super::GetEvade();
	}


	//I want to use min on all the base values to prevent them from being negative via base.
	// Mults are to be reverse clamped to be above 0.15, and below -0.15, or at zero.
	// The total mult is not allowed to go into negatives. 
	//   To our second line, I'm considering it being rClamped to -0.15 - 0 or allowing just 5%.

	//These lists should be vectors.
	float ActorCombatValueOwner::GetStagger(QueryContext& context, std::list<CombatValueEffect*>* applyValueList)
	{
		auto a_form = CheckSource(context);

		CacheCombatValue baseValue;// = stagger;
		baseValue = stagger;//If above could work, that would be sick.
		baseValue.generalBase = ItemStatHandler::GetStatValue(a_form, context.GetBool(queryBash) ? bashStagger : baseStagger);

		ProcessEffects(CombatValue::kStagger, baseValue, context, applyValueList);

		float result = baseValue.totalMod * (baseValue.generalBase * baseValue.generalMod) +
			(baseValue.positiveBase * baseValue.positiveMod - baseValue.negativeBase * baseValue.negativeMod);
		//make sure to treat the result for mins and maxes
		//return fmax(result, 0);
		return Utility::Clamp(result, 1, 30);
	}

	float ActorCombatValueOwner::GetPrecision(QueryContext& context, std::list<CombatValueEffect*>* applyValueList)
	{
		auto a_form = CheckSource(context);

		CacheCombatValue baseValue;// = precision;
		
		baseValue = precision;
		baseValue.generalBase = ItemStatHandler::GetStatValue(a_form, context.GetBool(queryBash) ? bashPrecision : basePrecision);

		ProcessEffects(CombatValue::kStagger, baseValue, context, applyValueList);

		float result = baseValue.totalMod * (baseValue.generalBase * baseValue.generalMod) +
			(baseValue.positiveBase * baseValue.positiveMod - baseValue.negativeBase * baseValue.negativeMod);
		//make sure to treat the result for mins and maxes
		//return fmax(result, 1);
		return Utility::Clamp(result, 1, 30);
	}


	float ActorCombatValueOwner::GetResist(QueryContext& context, std::list<CombatValueEffect*>* applyValueList)
	{
		CacheCombatValue baseValue = resist;
		
		ProcessEffects(CombatValue::kResist, baseValue, context, applyValueList);

		float result = baseValue.totalMod * ((baseValue.generalBase * baseValue.generalMod) +
			(baseValue.positiveBase * baseValue.positiveMod - baseValue.negativeBase * baseValue.negativeMod));
		//make sure to treat the result for mins and maxes
		
		//return fmax(result, 1);
		return Utility::Clamp(result, 0, 25);
		//return resist();
	}

	float ActorCombatValueOwner::GetEvade(QueryContext& context, std::list<CombatValueEffect*>* applyValueList)
	{
		CacheCombatValue baseValue = evade;

		ProcessEffects(CombatValue::kEvade, baseValue, context, applyValueList);

		float result = baseValue.totalMod * (baseValue.generalBase * baseValue.generalMod) +
			(baseValue.positiveBase * baseValue.positiveMod - baseValue.negativeBase * baseValue.negativeMod);
		//make sure to treat the result for mins and maxes
		return fmax(result, 0);
		return Utility::Clamp(result, 0, 25);
		
		//return evade();
	}
}