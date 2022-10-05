#pragma once
#include "CombatValue.h"
#include "ActorCombatValueOwner.h"
#include "Combo.h"
#include "WeaponSwing.h"
#include "AttackStatController.h"
//#include "MagicStatController.h"
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

#include "IBehaviour.h"

#include "SerialArgument.h"

#include "SerializationTypePlayground.h"

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
		
		//This object is due for some reuse. For example
		// instead of having the bulk of functionality put into objects this object holds onto. Stuff like
		// is character should still be stored here, but the jist of it would be think reverse of how
		// extra data is, a central data unit, and multiple variable function units.
		HeadBehaviour header;

		RE::TESObjectREFR* PlaceAtMe(RE::TESObjectREFR* self, RE::TESForm* a_form, std::uint32_t count, bool forcePersist, bool initiallyDisabled)
		{
			using func_t = RE::TESObjectREFR* (RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::TESObjectREFR*, RE::TESForm*, std::uint32_t, bool, bool);
			RE::VMStackID frame = 0;//Not important, probably.
			REL::Relocation<func_t> func{ REL::ID(55672)/*9951F0*/ };
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			
			return func(vm, frame, self, a_form, count, forcePersist, initiallyDisabled);
		}


		//SerializableList<float> s_list;
		//CombatData* test = this;//Guess I can do this
		

		CombatData(RE::Actor* a_actor, bool is_building = true)
		{
			//s_list->push_back(1);
			//s_list->push_back(2);
			//s_list->push_back(3);
			//s_list->push_back(4);
			//s_list->push_back(5);

			//SerializingObject obj;

			//s_list.HandleConstruct(obj);

			_ownerID = a_actor->GetFormID();
			
			if (is_building)
				Init();

			//I would like to revise how this works, instead getting an id instead of a pointer.
			//
			
			header.SetOwner(this, is_building);//False if it is  deserializing

			//RE::DebugMessageBox(std::format("First: {}", (std::uintptr_t)&swingEvent));
#ifndef automatic_registry_enabled

			attackControl.SetOwner(this);
			//magicControl.SetOwner(this);
			guardControl.SetOwner(this);
			motionControl.SetOwner(this);
			poiseControl.SetOwner(this);
			actionPointControl.SetOwner(this);
			meleeCombo.SetOwner(this);
			recoverControl.SetOwner(this);
			strikeControl.SetOwner(this);
#endif

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
			
			//if (a_actor && a_actor->currentProcess && a_actor->currentProcess->high) {
			//	a_actor->currentProcess->high->healthRegenDelay = 0;
			//	a_actor->currentProcess->high->magickaRegenDelay = 0;
			//	a_actor->currentProcess->high->staminaRegenDelay = 0;
			//}
			
			//std::string display = std::format("[init {} ]: stagger({}), resist({}), evade({}), poise({})",
			//	a_actor->GetName(), GetStagger(rightHand), GetResist(), GetEvade(), GetResourceMax(CombatResource::Poise));
			//needs an initializer part
			//It will need to search the left hand, the right hand
			//Utility::DebugNotification(display);
			if (is_building)
				logger::info("[init {} ]: stagger({}), resist({}), evade({}), poise({})",
					a_actor->GetName(), GetStagger(rightHand), GetResist(), GetEvade(), GetResourceMax(CombatResource::Poise));
			//if (rightHand && PlaceAtMe(a_actor, rightHand, 1, false, false) != nullptr)
			//	Utility::DebugNotification("This guy dropped a weapon");
			_initialized = true;
		}
		~CombatData()
		{
			logger::info("CombatData {:08X} has been deleted.", _ownerID);
		}

		//SHOULD be protected but whatever.
		void HandleRaceChange(RE::TESRace* race, bool _to)
		{
			//int mod = _to ? 1 : -1;

			auto raceStat = ItemStatHandler::GetRaceStatBase(ownerRace);

			//resist &= raceStat->GetValue(baseResist) * mod;
			//evade &= raceStat->GetValue(baseEvade)* mod;

			resist = raceStat->GetValue(baseResist);
			evade = raceStat->GetValue(baseEvade);
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

		private:
		static inline CombatData* AddDataRecord_Internal(RE::Actor* _actor);

		public:
		
		static CombatData*			AddDataRecord(RE::Actor* _actor);
		static CombatData*			AddDataRecord(RE::FormID _id);
		static bool					InsertDataRecord(CombatData* data);
		



		static bool RemoveDataRecord(RE::FormID _id);
		static bool HasDataRecord(RE::FormID _id);
		static int GetCount() { return _characterMap->size(); }
		static CombatData* GetDataRecord(RE::Actor* _actor, bool force = false);
		static CombatData* GetDataRecord(RE::FormID _id, bool force = false);
		static CombatData* GetPlayerRecord() { return GetDataRecord(0x00000014, false); }
		static CombatValueOwner* GetValueOwner(RE::TESObjectREFR* _actor);
		static CombatValueOwner* GetValueOwner(RE::FormID _id);


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





		//Move this to the ranged controller
		//When the crossbow isn't loaded, attack will instead be translated to reload, which can only go through
		// Also, would like to have this handle the bolt showing.
		bool crossbowLoaded = true;


	public:
	
		/*
		template<class ReturnType>
		constexpr void* GetBehaviourChild()
		{
			//This is mostly temporary, one should be the one to reinterpret this back into the other form.
#define TestBehaviourType(mc_return_name) \
if constexpr (std::is_same_v<decltype(mc_return_name), ReturnType>) return &##mc_return_name//if constexpr (std::is_same_v<decltype(mc_return_name), ReturnType>) return &##mc_return_name
			

			TestBehaviourType(attackControl);
			else TestBehaviourType(guardControl);
			else TestBehaviourType(motionControl);
			else TestBehaviourType(meleeCombo);
			else TestBehaviourType(strikeControl);
			else TestBehaviourType(actionPointControl);
			else TestBehaviourType(poiseControl);
			else TestBehaviourType(recoverControl);

			throw nullptr;
		}
		//*/
		
		
		void* GetBehaviourChild(const type_info* a_type) override
		{
			//This is mostly temporary, one should be the one to reinterpret this back into the other form.
#define TestBehaviourType(mc_return_name) \
if (&typeid(decltype(mc_return_name)) == a_type) return &##mc_return_name//if constexpr (std::is_same_v<decltype(mc_return_name), ReturnType>) return &##mc_return_name


			TestBehaviourType(attackControl);
			else TestBehaviourType(guardControl);
			else TestBehaviourType(motionControl);
			else TestBehaviourType(meleeCombo);
			else TestBehaviourType(strikeControl);
			else TestBehaviourType(actionPointControl);
			else TestBehaviourType(poiseControl);
			else TestBehaviourType(recoverControl);

			throw nullptr;
		}


		AttackStatController attackControl;
		//MagicStatController magicControl;
		GuardStatController guardControl;
		MotionStatController motionControl;

		MeleeCombo meleeCombo;

		SpecialStrikeController strikeControl;
		ActionPointController actionPointControl;
		PoiseController poiseControl;
		RecoveryController recoverControl;
		//This object is the thing that should handle updates, so each time the actor thingy is called, do it from here instead.
		// Might make it hold functions, but also that sounds like it's extra space and everythings in close proximity so.
		void Update(DisallowReference<float> updateDelta, DisallowReference<RE::Actor*> owner = nullptr)
		{
			//if (!_initialized)//Will not run if not initialized
			//	return;

			//Actually, it's probably best to make this whole thing an event.
			// Have the base intercept it, and then have the an interpreted function convey it.
			//Actor being here saves the look up. But not really required.
			if (!owner || owner->GetFormID() != _ownerID)
				owner = GetOwner();

			if (owner->IsDead() == true) {
				//This won't update if they are dead, updates should cease function.
				// Ideally, what we want to do so it works with revives, resets, and ressurects
				// is that it sends one final message to absolute everything.
				// last element, if this thing is dead, this data is marked as trival.
				// no serialization.
				//A major note, though, might as well just delete thyself upo death right? Then bring
				// it back on the revive? If there's anything I wanna remember, I'll put it in here.
				// XXX can't work, combat effects would go away. This has to stick around.
				// So instead, make trival serialization a default for the behaviours, but not for 
				// CD's serializable data.
				//When you do the above, make it an event to make it less of a pain.
				return;
			}
			//Individual character update deltas are broken, I don't think I'm hooking the right part.


			TimeManager::UpdateTimer(_ownerID, updateDelta);

			//I would like to disallow references in this, without the use of a const value.
			SendEvent("OnUpdate", owner, updateDelta);
			
			return;
			//Testing perks, ver fun stuff
			if (!_addedPerk && IsPlayerRef() == false && owner->currentProcess && owner->currentProcess->middleHigh)
			{
				auto* perk = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSPerk>(0x800, "Test1HitKill.esp");

				for (auto& perkEntry : perk->perkEntries) {
					if (perkEntry) {
						perkEntry->ApplyPerkEntry(owner);
					}
				}
				owner->OnArmorActorValueChanged();
				//auto invChanges = owner->GetInventoryChanges();
				//if (invChanges) {
					//invChanges->armorWeight = invChanges->totalWeight;
					//invChanges->totalWeight = -1.0f;
					//a_form->equippedWeight = -1.0f;
				//}
				_addedPerk = true;

				/*
				using func_t = bool(RE::Actor*, RE::BGSPerk*, std::uint8_t);

				REL::Relocation<func_t> func{ REL::ID(39677) };

				func(owner, perk, 0);

				_addedPerk = true;
				//*/
			}
		}

		bool _addedPerk = false;

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
		bool temp_IgnoreHitStrike = false;
		std::uint16_t currentSpecialID = 0;
		//GetRace()
	protected:


		template<DerivedSerialWrapper<SerialFormID> WrapFormID, DerivedSerialWrapper<CombatData*> WrapCombatData>
		static void HandleCombatMapEntry(Pair<WrapFormID, WrapCombatData>& entry, SerialArgument& serializer, bool& success)
		{
			success = serializer.Serialize(entry.first);//Needs to be a particular type of object, serializable formID


			if (success && serializer.IsDeserializing() == true) {
				RE::Actor* actor = nullptr;
				if (success) {
					actor = RE::TESForm::LookupByID<RE::Actor>(entry.first);
				}
				else
				{
					logger::error("Serial Failure");
				}

				
				if (!actor) {
					logger::error("Actor FormID {:08X} invalid, dumping.", static_cast<RE::FormID>(entry.first));
					success = false;
				}
				else {
					logger::info("Actor {}(FormID:{:08X}) successful, creating and deserializing.", actor->GetName(), static_cast<RE::FormID>(entry.first));
					entry.second = new CombatData(actor, false);//, serializer);
				}
			}

			//If the pointer is null or the success is false, it will dump the data, and return unsuccessful.
			success = serializer.DumpIfFailure(entry.second, success);

			if (success)
				logger::info("serialized: {:08X} at {:08X}", static_cast<RE::FormID>(entry.first), (uint64_t)(CombatData*)entry.second);
			else
				logger::error("failed to de/serialize");
		}

		//using CombatDataMap = std::map<RE::FormID, CombatData*>;
		using CombatDataMap = SerializableMap<SerialFormID, CombatData*, nullptr, nullptr, HandleCombatMapEntry>;

		template<class A, class B>
		using Pair = std::pair<const A, B>;



		RE::TESRace* ownerRace;
		
		static CombatDataMap _characterMap;
		
		void SerializeData(SerialArgument& serializer, bool& success) override 
		{
			serializer.Serialize(attackControl);
			serializer.Serialize(guardControl);
			serializer.Serialize(motionControl);
			serializer.Serialize(meleeCombo);
			serializer.Serialize(strikeControl);
			serializer.Serialize(actionPointControl);
			serializer.Serialize(poiseControl);
			serializer.Serialize(recoverControl);

			logger::info("size test 1: {}", resourceContainer->size());

			AutoSerialize(serializer);

			logger::info("size test 2: {}", resourceContainer->size());


			logger::info("[cereal {} ]: stagger({}), resist({}), evade({}), poise({})",
				NULL_CONDITION_RET(GetOwner())->GetName(), GetStagger(rightHand), GetResist(), GetEvade(), GetResourceMax(CombatResource::Poise));
		}
		//RE::TESRace*& actorRace;
	};

	inline CombatData::CombatDataMap CombatData::_characterMap = Initializer(_characterMap, HandlePrimarySerializer, PrimaryRecordType::CombatDataMap);
}