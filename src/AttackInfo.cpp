#pragma once

#include "AttackInfo.h"
#include "CombatData.h"
#include "CombatValueOwner.h"

//*
namespace Crucible
{
	/*
	DodgeInfo DodgeInfo::CreateGeneric(bool success, CombatData* def, CombatValueOwner* atk)
	{
		DodgeInfo info;
		info.success = success;
		info.SetTargets(def, atk);
		info.SetResults();
		return info;

	}

	DodgeInfo DodgeInfo::CreateGeneric(bool success, RE::Actor* def, RE::TESObjectREFR* atk)
	{
		CombatData* __def = def ? CombatData::GetDataRecord(def->GetFormID()) : nullptr;
		CombatValueOwner* __atk = atk ? CombatData::GetValueOwner(atk->GetFormID()) : nullptr;

		return CreateGeneric(success, __def, __atk);
	}

	CombatData* IAttackInfo::GetAggressorCombatData() { return aggressor ? aggressor->AsCombatData() : nullptr; }
	//*/
}

//*/