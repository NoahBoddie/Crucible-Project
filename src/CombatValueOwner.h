#pragma once

#include "QueryContext.h"
#include "CombatValue.h"



namespace Crucible
{
	class CombatData;

	//Query context should ALWAYS come with the target actor of the query. Either as the attacker or defender.

	//One central one?
	class CombatValueOwner
	{
	public:
		virtual float GetStagger(RE::TESForm* a_form);

		virtual float GetPrecision(RE::TESForm* a_form);

		virtual float GetResist();

		virtual float GetEvade();


		//For these apply lists, I think a reference might be a good idea.

		virtual float GetStagger(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr);

		virtual float GetPrecision(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr);

		virtual float GetResist(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr);

		virtual float GetEvade(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr);

		virtual CombatData* AsCombatData() { return nullptr; }
	};


	
}