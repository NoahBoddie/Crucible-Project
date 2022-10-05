#pragma once
#include "CompetitiveEvent.h"
#include "CombatDataBehaviour.h"

namespace Crucible
{
	class CombatData;

	class WeaponSwing : 
		public CompetitiveEvent<RE::TESObjectWEAP*>,
		public CombatDataBehaviour
	{
	protected:
		
		bool OnResult(bool success, RE::TESObjectWEAP* weap) override;

	};

	

}