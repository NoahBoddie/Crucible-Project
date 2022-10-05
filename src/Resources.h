#pragma once

namespace Crucible
{
	class TaskTimerHandler;

	enum class WeaponType 
	{ 
		kHandToHandMelee, 
		kOneHandSword, 
		kOneHandDagger, 
		kOneHandAxe, 
		kOneHandMace,
		kTwoHandSword,
		kTwoHandAxe,
		kBow,
		kStaff,
		kCrossbow,
		kTwoHandMace
	};
	//NA specifically is for instances were the magicka is 0 rather than the effect not existing.
	enum class MagicSkill
	{
		kNone,
		kZero,
		kNovice,
		kApprentice,
		kAdept,
		kExpert,
		kMaster,
		kRitual
	};
	

	enum class WeightClass
	{
		kNone,
		kLight,
		kMedium,
		kHeavy
	};

	//Relies the relevant item type of the hands. Especially because the position doesn't really imply what it really is.
	enum class ItemType
	{
		kNone,
		kMelee,
		kShield,
		kRangedBow,
		kCrossBow,
		kSpell,
		kScroll, 
		kStaff
	};

	//enum class SizeType { };

	//gSecondsSinceLastFrame_WorldTime_142F6B948 consider this too.
	static float* g_deltaTime = (float*)REL::ID(523660).address();
	static float* g_runTime = (float*)REL::ID(523662).address();
	//need to make an additional value for timer serialization here, it's the runtime since load game, incrementing every main
	// loop.



	//Would like to use these to carry important effects if they have relevant magic effects.
	using SpellPack = std::pair<RE::MagicItem*, RE::EffectSetting*>;

	class Resources
	{
	public:

		static bool Initialize()
		{
			//Now, if this fails to load properly, it will submit an error, the same type that happens when there is no
			// crucible. I think I will have this entire thing in a throw catch situation, and when something throws
			// it returns false.
			//weapTypeWarhammer

			auto dataHandler = RE::TESDataHandler::GetSingleton();

			if (!dataHandler)
				return false;

			weapTypeWarhammer = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("WeapTypeWarhammer");
			 
			paCooldownSpell = dataHandler->LookupForm<RE::SpellItem>(0x12D1, "Crucible.esp");
			paCooldown = dataHandler->LookupForm<RE::EffectSetting>(0x12D2, "Crucible.esp");
	
			dcCooldownSpell = dataHandler->LookupForm<RE::SpellItem>(0xB4D6, "Crucible.esp");
			dcCooldown = dataHandler->LookupForm<RE::EffectSetting>(0xB4D5, "Crucible.esp");



			if (!paCooldown || !paCooldownSpell)
				return false;
			
			equipBothHand = dataHandler->LookupForm<RE::BGSEquipSlot>(0x13F45, "Skyrim.esm");


			//equipRightHand = RE::TESForm::LookupByEditorID<RE::BGSEquipSlot>("RightHand");
			//equipLeftHand = RE::TESForm::LookupByEditorID<RE::BGSEquipSlot>("LeftHand");
			//equipEitherHand = RE::TESForm::LookupByEditorID<RE::BGSEquipSlot>("EitherHand");
			//equipVoice = RE::TESForm::LookupByEditorID<RE::BGSEquipSlot>("Voice");
			//equipShield = RE::TESForm::LookupByEditorID<RE::BGSEquipSlot>("SHIELD");
			
			return true;

		}


		//The stuff I want to store will go here, as will the functions that manage them.
		//Mainly stuff like boundobjects, magic effects and such. Nothing volital or prone to change.
		
		//These are the movement type effects. For still, walking running, and sprinting. These signal when they
		// happen. Hopefully. For the player, I can use a different method for tracking and triggering these states.
		
		static inline RE::BGSKeyword* weapTypeWarhammer;//Specially exists to differenciate between types.
		
		static inline RE::EffectSetting* paCooldown;
		static inline RE::SpellItem* paCooldownSpell;


		static inline RE::EffectSetting* dcCooldown;
		static inline RE::SpellItem* dcCooldownSpell;

		static inline RE::BGSEquipSlot* equipBothHand;

		//A value that refreshes each time a new save is load. It's attached to the time scale for processing as well.
		//Or should be.

		//static inline RE::BGSEquipSlot* equipRightHand;
		//static inline RE::BGSEquipSlot* equipLeftHand;
		//static inline RE::BGSEquipSlot* equipEitherHand;
		//static inline RE::BGSEquipSlot* equipVoice;
		//static inline RE::BGSEquipSlot* equipShield;

		
		//if you want armorType, use get armor type.

		

	private:
		

		//static inline float sessionRuntime;

		///friend class TaskTimerHandler;
		//friend class TimeManager;
	};
}