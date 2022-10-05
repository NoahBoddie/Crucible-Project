#pragma once

#include "EquipUtilities.h"
#include "CombatData.h"
#include "Utility.h"

namespace Crucible
{


	//*
	namespace EquipUtilities
	{
		//The deprecated handles will be moved to some place else. Utility maybe?

		void HandleSpellStats(CombatData* cData, RE::MagicItem* magicItem, bool equipping, bool right)
		{
			//scrolls and staves (ie enchants) shouldn't comprimise focus.
			int8_t mod = equipping ? 1 : -1;

			cData->resist %= CombatValueInput('_SPL', ValueInputType::ModifyNoCross, -0.15f, equipping);//-0.15f * mod;
		}

		//Don't need to account for left right for now, but when it expands I will.
		void HandleWeaponStats(CombatData* cData, WeaponType w_Type, bool right)
		{
			

			/*
			switch (w_Type)
			{
			case WeaponType::kHandToHandMelee:
			case WeaponType::kOneHandDagger:
				cData->stagger = 6;//3
				break;
			case WeaponType::kOneHandSword:
				cData->stagger = 8;//4
				break;

			case WeaponType::kOneHandAxe:
				cData->stagger = 10;//5
				break;

			case WeaponType::kOneHandMace:
				cData->stagger = 12;//6
				break;

			case WeaponType::kTwoHandSword:
				cData->stagger = 12;//6
				break;

			case WeaponType::kTwoHandAxe:
				cData->stagger = 14;//7
				break;

			case WeaponType::kBow:
				cData->stagger = 20;//10
				break;

			case WeaponType::kStaff:
				cData->stagger = 0;//Staves are weird, cause they're magics. SO you know. Direct to spell handle.
				break;

			case WeaponType::kCrossbow:
				cData->stagger = 24;//12
				break;

			case WeaponType::kTwoHandMace:
				cData->stagger = 16;//8
				break;
			}
			//*/
		}

		//YO seriously move this shit into a source please smile
		void HandleArmorStats(CombatData* cData, ArmorType armType, bool equipping)
		{
			int8_t mod = equipping ? 1 : -1;

			//The mults are going away for now, they will likely be reworked into being a global effect that applies the mults
			// based on skills in armoring, making clothes useful in you need no skill for it.

			switch (armType)
			{
			case ArmorType::kClothing:
				//cData->stagger *= 1 * mod;// I might make these raise by a bit for each piece of clothing, seeing
											// as it gets removed from ranged anyways.
				//Gen 1
				//cData->resist *= 0.25f * mod;
				//cData->evade *= 0.25f * mod;
				
				cData->resist &= CombatValueInput('ARMO', ValueInputType::ModifyNoCross, 0.5f, equipping);//0.5f * mod;//Total of 2

				//Increase of 0.5
				cData->resist *= CombatValueInput('ARMO', ValueInputType::ModifyNoCross, 0.125f, equipping);//0.125f * mod;
				cData->evade *= CombatValueInput('ARMO', ValueInputType::ModifyNoCross, 0.125f, equipping);//0.125f * mod;

				//Debuffs to both are less effective. total -0.25
				cData->resist /= CombatValueInput('ARMO', ValueInputType::ModifyNoCross, -0.0625f, equipping);//-0.0625f * mod;
				cData->evade /= CombatValueInput('ARMO', ValueInputType::ModifyNoCross, -0.0625f, equipping);//-0.0625f * mod;
				
				break;

			case ArmorType::kLightArmor:
				//Gen 1
				//cData->resist &= 1 * mod;
				//cData->evade &= 1 * mod;

				cData->resist &= CombatValueInput('ARMO', ValueInputType::ModifyNoCross, 1, equipping);//1 * mod;
				cData->evade &= CombatValueInput('ARMO', ValueInputType::ModifyNoCross, 0.75f, equipping);//0.75f * mod;

				//cData->evade *= 0.0875f * mod;//0.35 total

				break;

			case ArmorType::kHeavyArmor:
				//Gen 1
				//cData->resist &= 2 * mod;
				//cData->evade &= -0.5f * mod;
				
				cData->resist &= CombatValueInput('ARMO', ValueInputType::ModifyNoCross, 1.75f, equipping);//1.75f * mod;
				cData->evade &= CombatValueInput('ARMO', ValueInputType::ModifyNoCross, -0.25f, equipping);//-0.25f * mod;//perhaps it would be better to make positives climb less instead?

				//cData->resist *= 0.0625f * mod;//0.25 total

				
				break;

			}
		}




		void HandleSpellEquip(CombatData* cData, RE::MagicItem* magicItem, bool equipping, bool handleEquip)
		{
			//If this is a staff called from weapons it will not handle it's own equip in the CombatData
			if (!magicItem)
				return;

			switch (magicItem->GetFormType())
			{
			case RE::FormType::Spell:
			case RE::FormType::Scroll:
				EquipUtilities::HandleSpellStats(cData, magicItem, equipping, true);
			}
		}

		void HandleWeaponEquip(CombatData* cData, RE::TESObjectWEAP* a_weap, bool equipping)
		{
			//These will still get used, mainly just to tell if you're dual wielding or not, sword and board, fists
			// stuff like that. So this is gonna change, and get a shit tun more complex.
			if (!a_weap || !cData)
				return;

			auto actor = cData->GetOwner();

			if (!actor)
				return;

			auto type = Utility::GetWeaponType(a_weap);

			int hands = cData->GetFormRegisters(a_weap);
			//Note, for weapons when unequipping we don't reduce, rather we'll just set it to melee and if there's
			// something equipping it can set it back.
			//Utility::DebugMessageBox(std::format("{}, {} & {}, e {}", hands, !equipping == (hands & 1 == 1), (actor->GetEquippedObject(false) == a_weap) == equipping, equipping).c_str());
			if (!equipping == (hands & 1 == 1) && (actor->GetEquippedObject(false) == a_weap) == equipping)
			{
				if (equipping && type == WeaponType::kStaff)
					EquipUtilities::HandleSpellStats(cData, a_weap->formEnchanting, equipping, true);
				else
					EquipUtilities::HandleWeaponStats(cData, equipping ? type : WeaponType::kHandToHandMelee, true);

				cData->rightHand = equipping ? a_weap : nullptr;
			}


			if (!equipping == (hands & 2 == 2) && (actor->GetEquippedObject(true) == a_weap) == equipping)
			{

				if (equipping && type == WeaponType::kStaff)
					EquipUtilities::HandleSpellStats(cData, a_weap->formEnchanting, equipping, false);
				else
					EquipUtilities::HandleWeaponStats(cData, equipping ? type : WeaponType::kHandToHandMelee, false);

				cData->leftHand = equipping ? a_weap : nullptr;
			}


		}

		//YO seriously move this shit into a source please smile
		void HandleArmorEquip(CombatData* cData, RE::TESObjectARMO* a_armor, bool equipping)
		{
			if (!a_armor)
				return;

			auto slotMask = a_armor->GetSlotMask();



			//post handle, the armor slot should be added or removed from where they are supposed to
			// in combat data.
			//For the direct equip refresh situation, I would like 
			if ((slotMask & BipedObjectSlot::kCirclet) == BipedObjectSlot::kCirclet && (!equipping || cData->head != a_armor)) {
				HandleArmorStats(cData, a_armor->GetArmorType(), equipping);
				cData->head = equipping ? a_armor : nullptr;
			}
			if ((slotMask & BipedObjectSlot::kBody) == BipedObjectSlot::kBody && (!equipping || cData->torso != a_armor)) {
				HandleArmorStats(cData, a_armor->GetArmorType(), equipping);
				cData->torso = equipping ? a_armor : nullptr;
			}
			if ((slotMask & BipedObjectSlot::kHands) == BipedObjectSlot::kHands && (!equipping || cData->hands != a_armor)) {
				HandleArmorStats(cData, a_armor->GetArmorType(), equipping);
				cData->hands = equipping ? a_armor : nullptr;
			}
			if ((slotMask & BipedObjectSlot::kFeet) == BipedObjectSlot::kFeet && (!equipping || cData->feet != a_armor)) {
				HandleArmorStats(cData, a_armor->GetArmorType(), equipping);
				cData->feet = equipping ? a_armor : nullptr;
			}
			if ((slotMask & BipedObjectSlot::kShield) == BipedObjectSlot::kShield) {
				cData->shield = equipping ? a_armor : nullptr;//Shields yield no inheritent bonuses
			}
		}


		void HandleHandInit(CombatData* cData, RE::TESForm* a_form, bool right)
		{
			//purely handles stats when you know what the value is in. Mainly for constructors.
			// doesn't set anything.
			if (!a_form) {
				EquipUtilities::HandleWeaponStats(cData, WeaponType::kHandToHandMelee, right);
				return;
			}


			switch (a_form->GetFormType())
			{
			case RE::FormType::Weapon:
			{
				auto a_weap = a_form->As<RE::TESObjectWEAP>();
				auto type = Utility::GetWeaponType(a_weap);

				if (type == WeaponType::kStaff)
					EquipUtilities::HandleSpellStats(cData, a_weap->formEnchanting, right);
				else
					EquipUtilities::HandleWeaponStats(cData, type, right);
			}
			break;

			case RE::FormType::Scroll://Not real but you get the jist, I don't give a shit.
			case RE::FormType::Spell://Not real but you get the jist, I don't give a shit.
				EquipUtilities::HandleSpellStats(cData, a_form->As<RE::MagicItem>(), true, right);
				break;
			}
		}

		void HandleWornInit(CombatData* cData, RE::TESObjectARMO* a_armor)
		{
			//Also constructor only, this loads the stats.

			if (!a_armor || !cData)
				return;

			HandleArmorStats(cData, a_armor->GetArmorType(), true);
		}
	}
	//*/
}