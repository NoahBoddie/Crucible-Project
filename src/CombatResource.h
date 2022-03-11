#pragma once

#include "ReadableValue.h"
#include "FormulaHandler.h"

namespace Crucible
{
	enum class CombatResource
	{
		Poise,				//Used to determine poise state
		PoiseRecover,		//Used to determine how much poise should be recovered per second.
		StaminaRecover, 
		MagickaRecover,
		SpeedRecover	//Likely won't be a resource, I don't really want this to slosh around much.
	};

	//enum class ResourceModifier
	
	struct SimpleCombatResource
	{
	public:
		//using BaseDelegate = float(RE::Actor*);
		//100 is the default base, for variables
		//float (*GetBase)(RE::Actor*) = [](RE::Actor* actor) { return 100.f; };
		//BaseDelegate GetBase = [](RE::Actor* actor) { return 100.f; };

		float damageValue{0};
		float modifierValue{0};

		//maybe hide these? Or friend class it?
		ReadableValue baseValue;
		FormulaType formular = FormulaType::NoFormular;

		SimpleCombatResource& operator+=(const std::float_t& a_rhs) {
			//Increments the modifier
			modifierValue += a_rhs;
			return *this;
		}
		SimpleCombatResource& operator-=(const std::float_t& a_rhs) {
			//Increments the damageValue
			damageValue -= a_rhs;
			return *this;
		}
	};

	using CombatResourceContainer = std::map<const CombatResource, SimpleCombatResource>;

}