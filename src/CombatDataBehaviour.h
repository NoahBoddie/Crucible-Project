#pragma once

#include "EventSystem.h"

namespace Crucible
{
	class CombatData;
	
	class CombatDataBehaviour : public EventMember
	{
	public:
		void			SetOwner(CombatData* newOwner);
		CombatData*		GetOwner();
		RE::Actor*		GetActorOwner();
		RE::TESForm*	CheckAttackForm(RE::TESForm* form);//Gives parameter if exists and isn't a fist, else returns race.
	protected:
		virtual void Initialize() {}
		CombatData* owner;
	private:
		bool _init = false;
	};
}