#pragma once

#include "CombatValue.h"
#include "GlobalEffectHolder.h"

#include "GlobalEffects.h"
//#include "GlobalResistEffect.h"
//#include "GlobalStaggerEffect.h"

namespace Crucible
{
	
	void GlobalEffectHolder::Initialize()
	{
		AddFunctionToGlobal(CombatValue::kStagger, &GlobalEffect::PowerAction_Stagger);
		AddFunctionToGlobal(CombatValue::kStagger, &GlobalEffect::CounterBash_Stagger);
		AddFunctionToGlobal(CombatValue::kResist, &GlobalEffect::Blocking_Resist);
		AddFunctionToGlobal(CombatValue::kResist, &GlobalEffect::TimedBash_Resist);
	}

	//should make a new list, don't want to make any changes from here.
	std::vector<EffectFunction*>& const GlobalEffectHolder::GetFunctionList(const CombatValue value)
	{
		return _effectContainer[value];
	}

	bool GlobalEffectHolder::AddFunctionToGlobal(const CombatValue listToAddTo, EffectFunction* newFunction)
	{
		if (!newFunction)
			return false;

		_effectContainer[listToAddTo].push_back(newFunction);

		return true;
	}

	//How many lists, does one fucking have on these?
	// There's always the global, then theres the shit added to npcs by effects and what not
	// then there's the shit added by the OTHER person in calculation. So from this, it should produce
	// an array that is the combinate of 3 lists.

}