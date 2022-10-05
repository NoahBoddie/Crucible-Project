#pragma once

#include "Utility.h"



namespace Crucible
{
	class CombatData;


	namespace EquipUtilities
	{
		//Depreciated
		void HandleSpellStats(CombatData* cData, RE::MagicItem* magicItem, bool equipping, bool right = true);
		//Depreciated
		void HandleWeaponStats(CombatData* cData, WeaponType w_Type, bool right = true);

		void HandleArmorStats(CombatData* cData, ArmorType armType, bool equipping);





		//Depreciated
		void HandleSpellEquip(CombatData* cData, RE::MagicItem* magicItem, bool equipping, bool handleEquip = true);
		//Depreciated
		void HandleWeaponEquip(CombatData* cData, RE::TESObjectWEAP* a_weap, bool equipping);

		//YO seriously move this shit into a source please smile
		void HandleArmorEquip(CombatData* cData, RE::TESObjectARMO* a_armor, bool equipping);

		//Depreciated
		void HandleHandInit(CombatData* cData, RE::TESForm* a_form, bool right);

		void HandleWornInit(CombatData* cData, RE::TESObjectARMO* a_armor);

	};


}