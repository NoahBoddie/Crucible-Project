#pragma once
#include "CombatValue.h"
#include "ActorCombatValueOwner.h"
#include "Combo.h"
#include "WeaponSwing.h"
#include "AttackStatController.h"
#include "MagicStatController.h"
#include "BlockStatController.h"
#include "MotionStatController.h"
#include "ActionPointController.h"
#include "SpecialStrikeController.h"
#include "Timer.h"
#include "TimeManager.h"

#include "Utility.h"
#include "ItemStatHandler.h"


#include "CombatResourceOwner.h"

//"Doesn't need include just needs declare" gang
#include "PoiseController.h"
#include "RecoveryController.h"


#include "QueryContext.h"
#include <cmath>

#include "EquipUtilities.h"


#include "EventObject.h"
#include "EventSystem.h"




namespace Crucible
{
	



	class WeaponSwing;
	class MotionStatController;
	

	class SerializingObject;





	/// <summary>
	/// Keeps track of crucible data and some worn data, as well as handles processing on functions like combos.
	/// </summary>
	class CombatData : 
		public CombatResourceOwner,
		public EventHandler
	{

	protected: 
		bool _initialized = false;
	public:
		//Curious if a hash map would be better.
		using CombatMap = std::map<RE::FormID, CombatData*>;
		//This object is due for some reuse. For example
		// instead of having the bulk of functionality put into objects this object holds onto. Stuff like
		// is character should still be stored here, but the jist of it would be think reverse of how
		// extra data is, a central data unit, and multiple variable function units.


		RE::TESObjectREFR* PlaceAtMe(RE::TESObjectREFR* self, RE::TESForm* a_form, std::uint32_t count, bool forcePersist, bool initiallyDisabled)
		{
			using func_t = RE::TESObjectREFR* (RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::TESObjectREFR*, RE::TESForm*, std::uint32_t, bool, bool);
			RE::VMStackID frame = 0;//Not important, probably.
			REL::Relocation<func_t> func{ REL::ID(55672)/*9951F0*/ };
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			
			return func(vm, frame, self, a_form, count, forcePersist, initiallyDisabled);
		}


		//SerializableList<float> s_list;


		CombatData(RE::Actor* a_actor)
		{
			//s_list->push_back(1);
			//s_list->push_back(2);
			//s_list->push_back(3);
			//s_list->push_back(4);
			//s_list->push_back(5);

			//SerializingObject obj;

			//s_list.HandleConstruct(obj);

			_ownerID = a_actor->GetFormID();

			Init();
			//I would like to revise how this works, instead getting an id instead of a pointer.
			//
			swingEvent.SetOwner(this);
			attackControl.SetOwner(this);
			magicControl.SetOwner(this);
			blockControl.SetOwner(this);
			motionControl.SetOwner(this);
			poiseControl.SetOwner(this);
			actionPointControl.SetOwner(this);
			meleeCombo.SetOwner(this);
			recoverControl.SetOwner(this);
			strikeControl.SetOwner(this);

			ownerRace = a_actor->race;

			HandleRaceChange(ownerRace, true);

			head = a_actor->GetWornArmor(BipedObjectSlot::kCirclet);//Head is literally head, this works better
			torso = a_actor->GetWornArmor(BipedObjectSlot::kBody);
			hands = a_actor->GetWornArmor(BipedObjectSlot::kHands);
			feet = a_actor->GetWornArmor(BipedObjectSlot::kFeet);
			shield = a_actor->GetWornArmor(BipedObjectSlot::kShield);

			rightHand = a_actor->GetEquippedObject(false);
			leftHand = a_actor->GetEquippedObject(true);
			
			//*
			EquipUtilities::HandleWornInit(this, head);
			EquipUtilities::HandleWornInit(this, torso);
			EquipUtilities::HandleWornInit(this, hands);
			EquipUtilities::HandleWornInit(this, feet);


			EquipUtilities::HandleHandInit(this, rightHand, true);
			EquipUtilities::HandleHandInit(this, leftHand, false);
			//*/
			
			std::string display = std::format("[init {} ]: stagger({}), resist({}), evade: ({}), poise({})",
				a_actor->GetName(), GetStagger(rightHand), GetResist(), GetEvade(), GetResourceMax(CombatResource::Poise));
			//needs an initializer part
			//It will need to search the left hand, the right hand
			RE::DebugNotification(display.c_str());

			//if (rightHand && PlaceAtMe(a_actor, rightHand, 1, false, false) != nullptr)
			//	RE::DebugNotification("This guy dropped a weapon");
		}

		//SHOULD be protected but whatever.
		void HandleRaceChange(RE::TESRace* race, bool _to)
		{
			int mod = _to ? 1 : -1;

			auto raceStat = ItemStatHandler::GetRaceStatBase(ownerRace);

			resist &= raceStat->GetValue(baseResist) * mod;
			evade &= raceStat->GetValue(baseEvade)* mod;
		}
		
		void ChangeRace(RE::TESRace* race)
		{
			if (!race)
				return;

			HandleRaceChange(ownerRace, false);
			HandleRaceChange(race, true);

			ownerRace = race;

			//scream so I can know this happened later.
		}


		static CombatData* AddDataRecord(RE::Actor* _actor);
		static CombatData* AddDataRecord(RE::FormID _id);
		static bool RemoveDataRecord(RE::FormID _id);
		static bool HasDataRecord(RE::FormID _id);
		static int GetCount() { return _characterMap.size(); }
		static CombatData* GetDataRecord(RE::Actor* _actor);
		static CombatData* GetDataRecord(RE::FormID _id);

		static void RemoveDataRecords();

		//Functions I want and/or need
		// Make combo manipulation functions here, the combos are structs because combos don't work independent of each other
		//  nor to they do their own thing.
		// I would like this class to hold the functions for triggering weaponswing. Maybe might just make that a class
		// in of itself this derives from.


		/// <summary>
		/// This is the class that holds all the serializable combat data for the individual.
		/// </summary>
		
		
		//struct SerializableCombatData
		//{
			//Not sure what's serializable anymore so I'm just gonna slap this here.
			//Ideally it should be used for stuff that needs to be remembered in some way shape or form, something
			//that may be a pain to reapply. I was gonna same magic effects but no, those need a reapply as well.
		//};


		//Enables and checks for higher functionality such as equipping different weapons and spells and such.
		bool isCharacter();
		void setCharacter(bool a_value);

		CombatData* AsCombatData() override { return this; }

		RE::TESForm* GetHand(bool rightHand)
		{
			auto actor = GetOwner();

			if (!actor)
				return nullptr;

			auto form = actor->GetEquippedObject(!rightHand);

			return CheckAttackForm(form);
		}
		
		RE::TESForm* GetBlockingForm(bool check = true)
		{
			if (shield)
				return shield;

			//If check included, null sends will use race data instead.
			auto actor = GetOwner();

			if (!actor) {
				return nullptr;
			}

			RE::TESForm* focus = leftHand ? leftHand : rightHand;



			if (!focus)
				return check ? actor->race : nullptr;

			auto formType = focus->GetFormType();
			
			switch (formType)
			{
			case RE::FormType::Armor:
			case RE::FormType::Weapon:
			case RE::FormType::Light:
				return focus;
			default:
				return check ? actor->race : nullptr;
			}
		}

		template<class To>
		To* GetHand(bool rightHand)
		{
			auto hand = GetHand(rightHand);

			return hand->As<To>();
		}

		float GetReach()
		{
			auto actor = GetOwner();

			auto weapon = Utility::GetAttackingWeapon(actor);

			

			if (weapon)
				return weapon->weaponData.reach;
			else
				return actor->race->data.unarmedReach;
		}

		//float recordHealth;//Override the damage function for burst damage checking instead.
		
		//probably serializable. Also when the player loads in game, I would like this to be set to the gametime on load.
		//I would very much not like to load in and be stunned.
		float lastStaggerTime;//I believe gametime starts over when this goes so maybe store it as a negative?
									 // I'm partially thinking of having having these register to a class that basically just updates
									 // an internal timer. That, or I can just use a magic effect to measure cooldown and dispel it on attack.

		std::uint8_t burstStagger;	//When you take burst damage or cross over a threshold of health you trigger one of these.
		bool thresholdStagger;      // for now, this just takes threshold. These values control the stagger information over time.
									// best to probably put them into a struct with record health perhaps. Dunno.
	




		//Move this to the ranged controller
		//When the crossbow isn't loaded, attack will instead be translated to reload, which can only go through
		// Also, would like to have this handle the bolt showing.
		bool crossbowLoaded = true;


		WeaponSwing swingEvent;		//Manages the hit or miss stamina recovery system.
		AttackStatController attackControl;
		MagicStatController magicControl;
		BlockStatController blockControl;
		MotionStatController motionControl;

		MeleeCombo meleeCombo;
		
		SpecialStrikeController strikeControl;
		ActionPointController actionPointControl;
		PoiseController poiseControl;
		RecoveryController recoverControl;
		//This object is the thing that should handle updates, so each time the actor thingy is called, do it from here instead.
		// Might make it hold functions, but also that sounds like it's extra space and everythings in close proximity so.
		void Update(float updateDelta, RE::Actor* owner = nullptr)
		{

			//Actor being here saves the look up. But not really required.
			if (!owner || owner->GetFormID() != _ownerID)
				owner = GetOwner();

			//Individual character update deltas are broken, I don't think I'm hooking the right part.

			TimeManager::Update(_ownerID, updateDelta);

			//Would like to have this be an event system
			attackControl.EvaluateState(owner->GetAttackState());
			blockControl.EvaluateState(updateDelta, owner);
			motionControl.EvaluateState();
			recoverControl.RecoveryUpdate(updateDelta);
			
		}

		int GetFormRegisters(RE::TESForm* a_form)
		{
			
			int result = 0;
		
			switch (a_form->GetFormType())
			{
			case RE::FormType::Weapon:
			case RE::FormType::Spell:
			case RE::FormType::Scroll:
				result += rightHand == a_form ? 1 : 0;
				result += leftHand == a_form ? 2 : 0;
				break;
				
			case RE::FormType::Armor:
				if (!a_form)
					return 0;

				
				result += head == a_form ? 1 : 0;
				result += torso == a_form ? 2 : 0;
				result += hands == a_form ? 4 : 0;
				result += feet == a_form ? 8 : 0;
				result += shield == a_form ? 16 : 0;

				break;
			}

			return result;
		}

		
		WeightClass GetBestAttackClass()
		{
			//this at a later point should be current attack class, depending on if they are doing left
			// right, and if dual its the better of both. This makes it so dual wielding will have a point of balance.
			// In terms of running out of stamina, it will likely use the worst of either, or some weird inbetween ruling.

			//This also comes with a host of problems. Like, if your bashing it needs to handle that as well. You know?

			auto weapon = Utility::GetAttackingWeapon(GetOwner());


			return Utility::GetAttackingClass(weapon);//leftHand->As<RE::TESObjectWEAP>());

			//This might have some issues in the future. This function should probably check if this is
			// a dual attack, and if it is, it should use the below.

			//Would like to use this, or repurpose this, not right now though.

			auto _l = Utility::GetAttackingClass(leftHand->As<RE::TESObjectWEAP>());
			auto _r = Utility::GetAttackingClass(rightHand->As<RE::TESObjectWEAP>());

			return _l > _r ? _l : _r;
		}

		//You could close these off and make then apply to 
		RE::TESObjectARMO* head;
		RE::TESObjectARMO* torso;
		RE::TESObjectARMO* hands;
		RE::TESObjectARMO* feet;
		RE::TESObjectARMO* shield;

		//Nothing in this slot means it's fists. Merge these into a 
		RE::TESForm* rightHand;
		RE::TESForm* leftHand;//Does not include shield.
		//ItemType rightType;
		//ItemType leftType;

		bool ignoreRecoil = false;

		std::uint16_t currentSpecialID = 0;
		//GetRace()
	protected:

		
		
		RE::TESRace* ownerRace;


		inline static CombatMap _characterMap{ };

		

		//RE::TESRace*& actorRace;
	};
	


	
}