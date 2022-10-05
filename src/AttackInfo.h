#pragma once
#include "Property.h"
#include "StrikeEnumerations.h"
//#include "SpecialStrikeController.h"

//*
namespace Crucible
{

	
	//Lock the data within via state. After once the result is called, the values are locked.
	// More and more values get locked progressively as it moves on.
	
	//This determines if the source type is something we actually want to handle or not. For now, this gets processed only in hit, because nothing
	// will really have issue with it, but implement in dodge too.
	enum struct AttackType
	{
		Invalid, 
		Melee = 1,					//Melee weapons or raw fist damage generally associated with the attack classes
		Guard = Melee << 1 | Melee, //A subcategory of melee, but specifically related to bashes
		Ranged,						//A category associated with instances of things such as crossbows or bows being used.
		Magic,						//Shouts, staves, scrolls, spells, magic dealies one and all.
		Extra						//Extra are generally damage vectors not pertaining to spells or weapons, like explosions, and likely will not want processing.
	};

	class CombatData;
	class CombatValueOwner;
	class DodgeInfo;
	class HitInfo;

	//What in this seeks finalization?

	//value, result, defender, aggressor, attacker, and target, attack_type cannot be changed off of non-invalid
	//Success once set to a value shouldn't be set off of it, and as such should be finalized. Basically,
	// success is the only value that would want to be able to set itself, AFTER finalization.

	//Of the things that I think I want,
	//Actor* of target/attacker, and CombatData* of defender and CombatValueOwner* being aggressor

#define INFO_PROPERTY(mc_prop_name,...) RESTRICTED_PROPERTY(mc_prop_name, !static_cast<bool>(_temp_) && !_this._finished, __VA_ARGS__)
//*
	struct IAttackInfo
	{
	protected:
		bool _finished = false;
	public:
		StrikeResult	INFO_PROPERTY(result, public) = StrikeResult::none;
		AttackType		INFO_PROPERTY(attackType, public) = AttackType::Invalid;
		float			INFO_PROPERTY(value, public) = 0;
		
		pointer<CombatData>			INFO_PROPERTY(defender, public) = nullptr;
		pointer<CombatValueOwner>	INFO_PROPERTY(aggressor, public) = nullptr;

		pointer<RE::Actor>			INFO_PROPERTY(target, public) = nullptr;
		pointer<RE::Actor>			INFO_PROPERTY(attacker, public) = nullptr;

	public:
		virtual DodgeInfo* AsDodgeInfo() { return nullptr; }
		virtual HitInfo* AsHitInfo() { return nullptr; }


		//MANUALLY CALL THIS PLEASE
		void Finalize() { _finished = true; }
	};
/*/
	struct IAttackInfo
	{
	protected:
		bool _finished = false;

		StrikeResult _result;
		float _value;

		AttackType _attackType = AttackType::Invalid;

		CombatData*			defender{ nullptr };
		CombatValueOwner*	aggressor{ nullptr };
	public:
		RE::Actor* target{ nullptr };
		RE::Actor* attacker{ nullptr };
		void SetTargets(CombatData* def, CombatValueOwner* atk)
		{
			defender = defender ? defender : def;
			aggressor = aggressor ? aggressor : atk;
		}

		//May need these because some functions decide they don't want to fill agressor when they end before
		// they find out who it is.
		void SetDefender(CombatData* def) { defender = defender ? defender : def; }
		void SetAggressor(CombatValueOwner* atk) { aggressor = aggressor ? aggressor : atk; }

		
		//MANUALLY CALL THIS PLEASE
		void Finalize() { _finished = true; }
		void SetResults() { _finished = true; }


		void SetResult(StrikeResult result) { _result = result; }

		void SetValue(float value) { _value = value; }

		void SetResults(StrikeResult result, float value)
		{
			if (_finished)
				return;

			_result = result;
			_value = value;
		}

		void SetAttackType(AttackType at) { _attackType = at; }


		//I think I would like to make auto properties now thank you very much.

		StrikeResult GetResult() { return _result; }
		float GetValue() { return _value; }
		AttackType GetAttackType() { return _attackType; }

		virtual DodgeInfo*	AsDodgeInfo() { return nullptr; }
		virtual HitInfo*	AsHitInfo() { return nullptr; }

		//Make an optional area to set these while getting them.
		CombatValueOwner* GetAggressor() { return aggressor; }
		CombatData* GetAggressorCombatData();
		CombatData* GetDefender() { return defender; }
	};
	//*/


	struct DodgeInfo : public IAttackInfo
	{

		bool success = false;

		StrikeResult hitRestrictions;
		


		bool PROPERTY(allowStrike)
		{ 
			GET(bool) 
			{ 
				return _this.hitRestrictions != StrikeResult::total; 
			} 
		};
		
		bool PROPERTY(isGeneric)
		{
			GET(bool)
			{
				return *_this.attackType != AttackType::Invalid && _this._finished;
			}
		};

		operator bool() { return success; }

		DodgeInfo* AsDodgeInfo() override { return this; }

		void SetSuccess(bool s, StrikeResult restricts)
		{
			success = s;
			hitRestrictions = restricts;
		}

		static DodgeInfo CreateGeneric(bool success)
		{
			DodgeInfo info;
			info.success = success;
			info.Finalize();
			return info;
		}
	};


	struct MagicHitData
	{
		float healthDamage;
	};

	using PhysicalHitData = RE::HitData;

	struct HitInfo : public IAttackInfo
	{
	private:

		DodgeInfo* _dodgeInfo = nullptr;
		PhysicalHitData* _physical = nullptr;
		MagicHitData* _magical = nullptr;
	
		inline void SetDodge(DodgeInfo* dodge)
		{
			if (!dodge)
				return;

			_dodgeInfo = dodge;
			
			if (dodge->isGeneric)
				return;

			_attackType = dodge->attackType;


		}
	public:

		float poiseDamage = 0;//I may make a temporary combat value out of this.
		bool poiseCanBreak = false;//This is here so I can control this from block stuff.
		float PROPERTY(healthDamage)
		{
			GET(float)
			{
				if (_this._physical)
					return _this._physical->totalDamage;
				else
					return _this._magical->healthDamage;
			}
			SET(float)
			{
				if (_this._physical)
					_this._physical->totalDamage = value;
				else
					_this._magical->healthDamage = value;
			}
		};

		bool IsPhysical() { return _physical; }
		bool IsMagic() { return _magical; }

		PhysicalHitData* GetPhysicalHitData() { return _physical; }
		MagicHitData* GetMagicHitData() { return _magical; }
		DodgeInfo* GetDodgeInfo() { return _dodgeInfo; }

		HitInfo* AsHitInfo() override { return this; }



		HitInfo(PhysicalHitData& hit_data, DodgeInfo* last_dodge = nullptr) { _physical = &hit_data; SetDodge(last_dodge); }
		HitInfo(MagicHitData& hit_data, DodgeInfo* last_dodge = nullptr) { _magical = &hit_data; SetDodge(last_dodge); }
	};


}
//*/