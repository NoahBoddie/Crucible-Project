#pragma once
#include "CombatValue.h"
#include "CombatValueOwner.h"
#include "GlobalEffectHolder.h"
#include "ActorOwner.h"


namespace Crucible
{
	class ActorCombatValueOwner : 
		public CombatValueOwner,
		public ActorOwner
	{
	public:
		
		//CombatValueContainer valueContainer;
		
		//Going to make a class that holds these, and other potential values in the future.
		CacheCombatValue evade;
		CacheCombatValue resist;
		SimpleCombatValue stagger;
		SimpleCombatValue precision;

		//CoreValueContainer coreValues;
		//ResourceValueContainer resourceValues;//For now this is the only one that gets used.



		//Big notification
		ValueEffectContainer conditionalEffects;

		//This is used for magic effects that have no additional conditions.
		// this way, I don't have to conditionally apply them every single time.
		std::list<CombatValueEffect*> activeEffects;





		//SimpleCombatValue rangedOffsetStagger;//Instead of a ranged offset, I should use conditional
		// values for the the attack boosts, conditioning that I want the submitted form to not be ranged.
		

		//I may remove the concept of needing a source on these. With Getstagger/precision just being overloaded
		// with one that forces something in source then fires the real one.

		
		//For some reason these convience functions will not compile other wise. FUCKING SHRUG.
		float GetStagger(RE::TESForm* a_form) override;;

		float GetPrecision(RE::TESForm* a_form) override;;

		float GetResist() override;

		float GetEvade() override;
		
		//Once magic effects come in, have these able to ask for raw or WITH conditional processing
		// To that end, it should be able to accept a list of effects
		//Additionally, if there is no form sent, it will use the race. The race will be used to get
		// the knockback. This will be based on size, but an additional factor as well, to set npcs
		// apparent from creatures. That, or npcs will have their own category.


		//For these apply lists, I think a reference might be a good idea.
		//Also, I think for at least a difference in offense and defense there should b a requirement of a certain type
		// of context, offense and defensive. ALSO, use pointers. Null is a valid context.
		


		float GetStagger(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr) override;

		float GetPrecision(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr) override;


		float GetResist(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr) override;

		float GetEvade(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr) override;
	
		float GetPoise(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr);
	
	protected:
		
		
		//might need to get put farther back, I think that combat value owner might want to use this.
		void ProcessEffects(const CombatValue valueType, CacheCombatValue& coreValue, QueryContext& context, std::list<CombatValueEffect*>* applyValueList)
		{
			auto list = GlobalEffectHolder::GetFunctionList(valueType);

			SimpleCombatValue value = SimpleCombatValue::zero();

			for (auto effectFunction : list)
			{
				effectFunction(valueType, value, context);
			}
			//This will not hold. But for now, we are copying the values.
			//But note, this is some awful looking shit.
			auto ceList = conditionalEffects[valueType];
			if (applyValueList)
				ceList.merge(*applyValueList);

			for (auto combatEffect : ceList)
			{
				if (!combatEffect)
					continue;

				combatEffect->TryApplyValue(value, context);
			}

			coreValue += value;
		}
	};

}