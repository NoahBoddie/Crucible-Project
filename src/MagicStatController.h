#pragma once
#include "Combo.h"
#include "Utility.h"
#include "Resources.h"
#include "StatedObject.h"
#include "CombatDataBehaviour.h"

namespace Crucible
{
	/*
	class MagicCombo;

	//All instances of casting share one state. So this doesn't get updated automatically, instead it gets updated
	// via hooks.
	//<!> Use smart pointers for these please.
	class MagicCasterData 
	{
	public:
		uMagicCastState previousState = uMagicCastState::kNone;
		ItemType spellType = ItemType::kNone;
		MagicSkill skillLevel = MagicSkill::kZero;
		float resourceStore = 0;
		float castTime = 0;
		



		bool growthFlag = false;
		bool dualCast = false;
		bool usedFlag = false;	//This flag prevents the send or manipulation of this toward the notion of going to none.
								// If casting gets canceled this will be ticked.
		//other things to remember in this shit
		// dual remember if this thing was dual casted (information is ignored in places if so
		// remember how long it was cast for, mainly for concentration registration. Gotta give it
		//   A chance to input for power attack reduction.
	};

	enum class MagicState 
	{ 
		kNone, 
		kCharging, 
		//kDualCharging, 
		kRitualCharging, 
		kCasting, 
		//kDualCasting, 
		//kRitualCasting, 
		kFocusing 
	};

	class MagicStatController :
		public StatedObject<MagicState>,
		public CombatDataBehaviour
	{
	public:
		MagicCombo magicCombo;
	
	protected:

		uMagicCastState storedEnum;

		//Stores the value of what's currently being cast, as well as the item type and the resources spent doing it.
		//This does not and should not be storing pointers for as simple as this little thing is.
		std::map<uCastingSource, MagicCasterData*> _castingMap;

	public:
		bool EvaluateState(RE::ActorMagicCaster* caster, float delta);

		
		bool HasCasterData(uCastingSource source)
		{
			return _castingMap.contains(source);
		}

		bool HasCasterData(RE::ActorMagicCaster* caster)
		{
			return caster ? HasCasterData(caster->GetCastingSource()) : false;
		}

		
		MagicCasterData* GetCasterData(uCastingSource source)
		{
			auto pair = _castingMap.find(source);

			if (_castingMap.end() != pair)
				return pair->second;

			return nullptr;
		}


		MagicCasterData* GetCasterData(RE::ActorMagicCaster* caster)
		{
			return caster ? GetCasterData(caster->GetCastingSource()) : nullptr;
		}


		//Creates and gets. There's never a reason to override so we just use this. Actually, get is useless. Probably.
		MagicCasterData* GetCreateCasterData(uCastingSource source)
		{
			auto pair = _castingMap.find(source);

			if (_castingMap.end() != pair)
				return pair->second;

			MagicCasterData* castData = new MagicCasterData();

			_castingMap[source] = castData;

			return castData;
		}


		MagicCasterData* GetCreateCasterData(RE::ActorMagicCaster* caster)
		{
			return GetCreateCasterData(caster->GetCastingSource());
		}

		float temp_restore;
		float temp_spent;
		float temp_power;

		void SendCastData()//This will be used when a combo is ended but someone is still casting.
		{
			RE::Actor* actor = GetActorOwner();

			for (auto const& [castType, castData] : _castingMap)
			{
				if (!castData)
					continue;

				//RE::MagicCaster* magicCaster = actor->GetMagicCaster(castType);
				//if (!magicCaster)
				//	continue;
				//conc is the only one that should go through, all others haven't fired yet.
				switch (castData->previousState)
				{
				case uMagicCastState::kNone:
					continue;

				case uMagicCastState::kConcentrating:
					magicCombo.IncrementCombo(castData);
				}

				castData->usedFlag = true;
			}
		}

	protected:
		void OnStateBegin() override;
		//void OnStateFinish() override;

		void Initialize() override;

	private:
		void HandleChange(MagicState _state, bool assign);
	};
	//*/
}