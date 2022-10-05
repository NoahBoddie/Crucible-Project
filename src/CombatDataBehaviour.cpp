#pragma once
#include "CombatData.h"
#include "CombatDataBehaviour.h"
namespace Crucible
{
	void CombatDataBehaviour::SetOwner(CombatData* newOwner, bool isBuilding) 
	{ 
		static bool once = true;
		//owner += 16;
		//Utility::DebugMessageBox(std::format("distance {}", offsetof(CombatDataBehaviour, CombatDataBehaviour::owner)), once);
		once = false;

		if (_init || !newOwner)
			return;

		owner = !owner ? newOwner : owner; 
		Initialize_INTERNAL();

		if (isBuilding){
			Build();
		}
		else {
			//Nothing for now
		}
		_init = true;
	}
	bool CombatDataBehaviour::IsPlayerRef() { return owner->IsPlayerRef(); }

	CombatData* CombatDataBehaviour::GetOwner() { return owner; }
	EventHandler* CombatDataBehaviour::GetHandler() { return GetOwner(); }
	RE::FormID	CombatDataBehaviour::GetID() { return owner->GetOwnerID(); }
	RE::Actor* CombatDataBehaviour::GetActorOwner() { return owner ? owner->GetOwner() : nullptr; }

	RE::TESForm* CombatDataBehaviour::CheckAttackForm(RE::TESForm* form) { return owner ? owner->CheckAttackForm(form) : nullptr; }

	RE::TESForm* CombatDataBehaviour::GetBlockingForm(bool check) { return owner ? owner->GetBlockingForm(check) : nullptr; }

	template<class... Args> requires(sizeof...(Args) != 0)
	void CombatDataBehaviour::SendOwnerEvent(std::string eventName, Args&&... args) { owner->SendEvent(eventName, args...); }

	//Easy short hand for an empty one.
	void CombatDataBehaviour::SendOwnerEvent(std::string eventName){ owner->SendEvent(eventName); }



	//CombatDataBehaviour* CombatDataBehaviour::GetSiblingBehaviour(std::string name){ owner->GetBehaviourChild(name); }

	//ActionPointController& CombatDataBehaviour::temp_GetActionPointController(){return owner->actionPointControl;}

	void* CombatDataBehaviour::TrySibling(const type_info* a_type) { return owner->GetBehaviourChild(a_type); }

	//template<std::derived_from<CombatDataBehaviour> DerivedBehaviour>
	//DerivedBehaviour* CombatDataBehaviour::GetSiblingBehaviour(std::string name) { owner->GetBehaviourChild<DerivedBehaviour>(name); }
}