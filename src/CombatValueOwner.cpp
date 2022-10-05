#pragma once

#include "CombatValueOwner.h"
#include "ItemStats.h"
#include "ItemStatHandler.h"
#include "CombatData.h"

namespace Crucible
{
	float CombatValueOwner::GetStagger(RE::TESForm* a_form)
	{
		QueryContext context{};
		if (auto* data = AsCombatData())
			context.SetForm(queryAggressor, data->GetOwnerID());
		
		context.SetForm(querySource, a_form);
		context.SetBool(queryRaw, true);
		//Probably set as non-confrontation
		return GetStagger(context, nullptr);
	}

	float CombatValueOwner::GetPrecision(RE::TESForm* a_form)
	{
		QueryContext context{};
		context.SetForm(querySource, a_form);
		
		if (auto* data = AsCombatData())
			context.SetForm(queryAggressor, data->GetOwnerID());
		
		
		context.SetBool(queryRaw, true);
		
		return GetPrecision(context, nullptr);
	}

	float CombatValueOwner::GetResist()
	{
		QueryContext context{};
		
		if (auto* data = AsCombatData())
			context.SetForm(queryDefender, data->GetOwnerID());

		context.SetBool(queryRaw, true);
		//and this too
		return GetResist(context, nullptr);
	}

	float CombatValueOwner::GetEvade()
	{
		QueryContext context{};
		if (auto* data = AsCombatData())
			context.SetForm(queryDefender, data->GetOwnerID());
		context.SetBool(queryRaw, true);
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