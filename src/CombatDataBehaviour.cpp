#pragma once
#include "CombatData.h"
#include "CombatDataBehaviour.h"
namespace Crucible
{
	void CombatDataBehaviour::SetOwner(CombatData* newOwner) 
	{ 
		if (_init || !newOwner)
			return;

		owner = !owner ? newOwner : owner; 
		Initialize();
		_init = true;
	}
	CombatData* CombatDataBehaviour::GetOwner() { return owner; }
	RE::Actor* CombatDataBehaviour::GetActorOwner() { return owner ? owner->GetOwner() : nullptr; }

	RE::TESForm* CombatDataBehaviour::CheckAttackForm(RE::TESForm* form) { return owner ? owner->CheckAttackForm(form) : nullptr; }
}