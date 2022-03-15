#pragma once

#include "Resources.h"
#include <cmath>
#include <stdarg.h>
# define pi				3.14159265358979323846  /* pi */
# define sign_bit		0x80000000



using string_hash = uint32_t;
constexpr string_hash hash(const char* data, size_t const size) noexcept
{
	string_hash hash = 5381;

	for (const char* c = data; c < data + size; ++c) {
		hash = ((hash << 5) + hash) + (unsigned char)*c;
	}

	return hash;
}


constexpr string_hash operator"" _h(const char* str, size_t size) noexcept
{
	//function and operator courtesy of Ershin's TDM
	return hash(str, size);
}


namespace Crucible
{
	//I haven't taken to it yet here, but in the places I use "using" I would like to put uName.
	// That way I know where to go for the definition.

	using HitEvent = RE::TESHitEvent;
	using CombatEvent = RE::TESCombatEvent;
	using REFRLoadEvent = RE::TESObjectLoadedEvent;
	using EffectEvent = RE::TESActiveEffectApplyRemoveEvent;
	using AnimEvent = RE::BSAnimationGraphEvent;
	using EquipEvent = RE::TESEquipEvent;

	using AnimSource = RE::BSTEventSource<AnimEvent>;
	using HitSource = RE::BSTEventSource<RE::TESHitEvent>;
	using CombatSource = RE::BSTEventSource<RE::TESCombatEvent>;
	using REFRLoadSource = RE::BSTEventSource<RE::TESObjectLoadedEvent>;
	using EffectSource = RE::BSTEventSource<RE::TESActiveEffectApplyRemoveEvent>;
	using EquipSource = RE::BSTEventSource<RE::TESEquipEvent>;

	using SkyrimVM = RE::BSScript::IVirtualMachine;



	using uATTACK_ENUM = RE::ATTACK_STATE_ENUM;
	using BipedObjectSlot = RE::BIPED_MODEL::BipedObjectSlot;
	using ArmorType = RE::BIPED_MODEL::ArmorType;
	using EventResult = RE::BSEventNotifyControl;
	using AVModifier = RE::ACTOR_VALUE_MODIFIER;
	using AttackDataFlags = RE::AttackData::AttackFlag;
	using HitFlag = RE::HitData::Flag;
	using HitEventFlag = RE::TESHitEvent::Flag;
	using uCastingSource = RE::MagicSystem::CastingSource;
	using uMagicCastState = RE::MagicCaster::MagicCastingState;
	using uWardState = RE::MagicSystem::WardState;
	//enum class GraphValueType { kBool, kFloat, kInt };
	
	using ProjectileType = RE::BGSProjectileData::Type;


	enum class EffectValue {Magnitude, Area, Duration };


	constexpr BipedObjectSlot operator &(BipedObjectSlot lhs, BipedObjectSlot rhs) noexcept
	{
		return static_cast<BipedObjectSlot> (
			static_cast<std::underlying_type<BipedObjectSlot>::type>(lhs) &
			static_cast<std::underlying_type<BipedObjectSlot>::type>(rhs)
			);
	}

	constexpr HitFlag operator &(HitFlag lhs, HitFlag rhs) noexcept
	{
		return static_cast<HitFlag> (
			static_cast<std::underlying_type<HitFlag>::type>(lhs) &
			static_cast<std::underlying_type<HitFlag>::type>(rhs)
			);
	}



	template<class T> inline T operator ~ (T a) { return (T)~(int)a; }
	template<class T> inline T operator | (T a, T b) { return (T)((int)a | (int)b); }
	template<class T> inline T operator & (T a, T b) { return (T)((int)a & (int)b); }
	template<class T> inline T operator ^ (T a, T b) { return (T)((int)a ^ (int)b); }
	template<class T> inline T& operator |= (T& a, T b) { return (T&)((int&)a |= (int)b); }
	template<class T> inline T& operator  &= (T& a, T b) { return (T&)((int&)a &= (int)b); }
	template<class T> inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }
	



	class Utility
	{
	private:
		

	public:

		static float Clamp(float value, float a, float b, bool r = false)
		{
			//if r is true, only the formers are considered, if false only the latters are.
			float min = a > b ? (r ? a : b) : (r ? b : a);
			float max = min == a ? b : a;

			if (value < min)
				return min;
			else if (value > max)
				return max;
			else
				return value;
		}

		static float ReverseClamp(float value, float a, float b)
		{
			return Clamp(value, a, b, true);
		}


		static WeaponType GetWeaponType(RE::TESObjectWEAP* a_weap)
		{
			if (a_weap)
			{
				switch (*a_weap->weaponData.animationType)
				{
				case RE::WEAPON_TYPE::kHandToHandMelee:
					return WeaponType::kHandToHandMelee;

				case RE::WEAPON_TYPE::kOneHandDagger:
					return WeaponType::kOneHandDagger;

				case RE::WEAPON_TYPE::kOneHandSword:
					return WeaponType::kOneHandSword;

				case RE::WEAPON_TYPE::kOneHandAxe:
					return WeaponType::kOneHandAxe;

				case RE::WEAPON_TYPE::kOneHandMace:
					return WeaponType::kOneHandMace;

				case RE::WEAPON_TYPE::kTwoHandSword:
					return WeaponType::kTwoHandSword;

				case RE::WEAPON_TYPE::kTwoHandAxe:
					if (Resources::weapTypeWarhammer && a_weap->HasKeyword(Resources::weapTypeWarhammer) == true)
						return WeaponType::kTwoHandMace;
					else
						return WeaponType::kTwoHandAxe;

				case RE::WEAPON_TYPE::kBow:
					return WeaponType::kBow;

				case RE::WEAPON_TYPE::kStaff:
					return WeaponType::kStaff;

				case RE::WEAPON_TYPE::kCrossbow:
					return WeaponType::kCrossbow;
				}
			}
			return WeaponType::kHandToHandMelee;
		}

		static ItemType GetItemType(RE::TESForm* a_form)
		{
			//Mainly gonna be used to handle what to do concerning an item
			if (!a_form)
				return ItemType::kNone;

			switch (a_form->GetFormType())
			{
			case RE::FormType::Weapon:
				switch (a_form->As<RE::TESObjectWEAP>()->GetWeaponType())
				{
				case RE::WEAPON_TYPE::kBow:
					return ItemType::kRangedBow;

				case RE::WEAPON_TYPE::kStaff:
					return ItemType::kStaff;

				case RE::WEAPON_TYPE::kCrossbow:
					return ItemType::kCrossBow;

				default:
					return ItemType::kMelee;
				}

			case RE::FormType::Spell:
				return ItemType::kSpell;

			case RE::FormType::Scroll:
				return ItemType::kScroll;

			case RE::FormType::Race:
				return ItemType::kMelee;

			case RE::FormType::Armor:
				if (a_form->As<RE::TESObjectARMO>()->IsShield() == true)
					return ItemType::kMelee;
			}

			return ItemType::kNone;
		}

		static WeightClass GetAttackingClass(RE::TESObjectWEAP* a_weap)
		{
			if (!a_weap)//Same as below, this being null is ok, that will likely mean it's magic.
				return WeightClass::kLight;

			auto type = GetWeaponType(a_weap);

			switch (type)
			{
			case WeaponType::kHandToHandMelee:
			case WeaponType::kOneHandDagger:
				return WeightClass::kLight;

			case WeaponType::kOneHandSword:
			case WeaponType::kOneHandAxe:
			case WeaponType::kOneHandMace:
			case WeaponType::kStaff:
				return WeightClass::kMedium;

			case WeaponType::kTwoHandSword:
			case WeaponType::kTwoHandAxe:
			case WeaponType::kBow:
			case WeaponType::kCrossbow:
			case WeaponType::kTwoHandMace:
				return WeightClass::kHeavy;
			}
		}

		static WeightClass GetDefendingClass(RE::TESObjectARMO* a_armor)
		{
			if (!a_armor)//Perfectly fine for sending this as null, as they're likely weapons or torches
				return WeightClass::kLight;
			else if (a_armor->IsShield() == false)
				return WeightClass::kNone;
			else if (a_armor->IsHeavyArmor() == true)
				return WeightClass::kHeavy;
			else
				return WeightClass::kMedium;
		}

		static RE::ActiveEffect* GetEffectFromID(RE::MagicTarget* magicTarget, std::uint16_t uniqueID)
		{
			if (!magicTarget)
				return nullptr;

			RE::BSSimpleList<RE::ActiveEffect*>* activeEffects = magicTarget->GetActiveEffectList();

			if (!activeEffects || activeEffects->empty() == true)
				return nullptr;

			for (auto effect : *activeEffects)
			{
				if (effect->usUniqueID == uniqueID)
					return effect;
			}

			return nullptr;
		}

		static RE::ActiveEffect* GetEffectFromSetting(RE::MagicTarget* magicTarget, RE::EffectSetting* setting)
		{
			if (!magicTarget)
				return nullptr;

			RE::BSSimpleList<RE::ActiveEffect*>* activeEffects = magicTarget->GetActiveEffectList();

			if (!activeEffects || activeEffects->empty() == true)
				return nullptr;

			for (auto effect : *activeEffects)
			{
				if (effect->GetBaseObject() == setting)
					return effect;
			}

			return nullptr;
		}

		static std::list<RE::ActiveEffect*> GetAllEffectsFromSetting(RE::MagicTarget* magicTarget, RE::EffectSetting* setting)
		{

			std::list<RE::ActiveEffect*> returnList;

			if (!magicTarget)
				return returnList;

			RE::BSSimpleList<RE::ActiveEffect*>* activeEffects = magicTarget->GetActiveEffectList();

			if (!activeEffects || activeEffects->empty() == true)
				return returnList;

			for (auto effect : *activeEffects)
			{
				if (effect->GetBaseObject() == setting)
					returnList.push_back(effect);
			}

			return returnList;
		}

		static MagicSkill GetMagicSkill(RE::MagicItem* a_magic)
		{
			if (!a_magic)
				return MagicSkill::kZero;

			switch (a_magic->GetFormType())
			{
			case RE::FormType::Scroll:
			case RE::FormType::Spell:
				if (a_magic->As<RE::SpellItem>()->GetEquipSlot() == Resources::equipBothHand)
					return MagicSkill::kRitual;
			}

			auto effect = a_magic->GetCostliestEffectItem();

			if (!effect || !effect->baseEffect)
				MagicSkill::kNone;

			auto minSkill = effect->baseEffect->GetMinimumSkillLevel();

			if (minSkill < 25)
				return MagicSkill::kNovice;
			else if (minSkill < 50)
				return MagicSkill::kApprentice;
			else if (minSkill < 75)
				return MagicSkill::kAdept;
			else if (minSkill < 100)
				return MagicSkill::kExpert;
			else
				return MagicSkill::kMaster;
		}

		static bool GetProjectileType(RE::MagicItem* a_magic, ProjectileType& type)
		{
			if (!a_magic)
				return false;



			auto effect = a_magic->GetCostliestEffectItem();

			if (!effect || !effect->baseEffect)
				return false;

			auto projectile = effect->baseEffect->data.projectileBase;

			if (!projectile)
				return false;

			type = *projectile->data.types;
			return true;
		}

		static RE::TESObjectWEAP* GetAttackingWeapon(RE::Actor* actor)
		{
			if (!actor)
				return nullptr;

			auto atkInvData = actor->GetAttackingWeapon();

			if (!atkInvData)
				return nullptr;

			auto result = atkInvData->GetObject();

			if (!result)
				return nullptr;

			return result->As<RE::TESObjectWEAP>();
		}


		/// <summary>
		/// Use BGSAttackDataMap::GetAttackData_1403E88F0 instead
		/// </summary>
		/// <param name="actor"></param>
		/// <param name="value"></param>
		/// <returns></returns>
		static RE::BGSAttackData* GetAttackData(RE::Actor* actor, const RE::BSFixedString& value)
		{
			if (!actor)
				return nullptr;


			auto race = actor->GetRace();

			if (!race)
				return nullptr;

			auto atkMap = race->attackDataMap.get()->attackDataMap;

			if (atkMap.contains(value) == true) {
				auto iter = atkMap.find(value);
				return iter->second.get();

			}

			return nullptr;
		}

		static RE::BGSAttackData* GetCurrentAttackData(RE::Actor* actor)
		{
			//use Get currentData here
			if (!actor || !actor->currentProcess || !actor->currentProcess->high || !actor->currentProcess->high->attackData)
				return nullptr;

			return actor->currentProcess->high->attackData.get();
		}

		static bool IsPowerAttacking(RE::Actor* actor)
		{
			auto attackData = GetCurrentAttackData(actor);

			if (!attackData)
				return false;

			return attackData->data.flags.all(AttackDataFlags::kPowerAttack);
		}

		


		//template <typename... Ts>
		///struct ToInfoArray
		//{
		//	static constexpr std::array<Info, sizeof...(Ts)> value = { { Ts::info... } };
		//};
		template <typename... Ts>
		static void SetMagicItemValues(RE::MagicItem* magItem, EffectValue setting, Ts... Args)//, ...)
		{
			static_assert(std::is_integral<Ts...>::value);

			if (!magItem)
				return;

			std::array values{ Args... };

			for (int i = 0; i < magItem->effects.size() && i < values.size(); i++)
			{
				RE::Effect* magEffect = magItem->effects[i];

				if (!magEffect)
					continue;

				auto& value = values[i];

				switch (setting)
				{
				case EffectValue::Magnitude:
					magEffect->effectItem.magnitude = static_cast<float>(value);
					break;
				case EffectValue::Area:
					magEffect->effectItem.area = static_cast<std::uint32_t>(value);
					break;
				case EffectValue::Duration:
					magEffect->effectItem.duration = static_cast<std::uint32_t>(value);
					break;
				}
			}
		}
	};
}