#pragma once

#include "CombatValueOwner.h"
#include "ItemStats.h"
#include "ItemStatHandler.h"

namespace Crucible
{
	float CombatValueOwner::GetStagger(RE::TESForm* a_form)
	{
		QueryContext context;
		context.SetForm(querySource, a_form);
		//Probably set as non-confrontation
		return GetStagger(context, nullptr);
	}

	float CombatValueOwner::GetPrecision(RE::TESForm* a_form)
	{
		QueryContext context;
		context.SetForm(querySource, a_form);
		//This too
		return GetPrecision(context, nullptr);
	}

	float CombatValueOwner::GetResist()
	{
		QueryContext context;
		//and this too
		return GetResist(context, nullptr);
	}

	float CombatValueOwner::GetEvade()
	{
		QueryContext context;
		//you get the picture
		return GetEvade(context, nullptr);
	}



	float CombatValueOwner::GetStagger(QueryContext& context, std::list<CombatValueEffect*>* applyValueList)
	{
		auto a_form = context.GetForm(querySource);

		if (!a_form)
			return 0;

		float generalBase = ItemStatHandler::GetStatValue(a_form, context.GetBool(queryBash) ? bashStagger : bashStagger);

		return generalBase;
	}

	float CombatValueOwner::GetPrecision(QueryContext& context, std::list<CombatValueEffect*>* applyValueList)
	{
		auto a_form = context.GetForm(querySource);


		if (!a_form)
			return 0;

		float generalBase = ItemStatHandler::GetStatValue(a_form, context.GetBool(queryBash) ? bashPrecision : basePrecision);

		return generalBase;
	}

	float CombatValueOwner::GetResist(QueryContext& context, std::list<CombatValueEffect*>* applyValueList)
	{
		return 0;
	}

	float CombatValueOwner::GetEvade(QueryContext& context, std::list<CombatValueEffect*>* applyValueList)
	{
		return 0;
	}

}