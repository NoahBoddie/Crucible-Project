#pragma once
#include "CombatResource.h"
#include "ActorCombatValueOwner.h"

#include "ReadableValue.h"
#include "FormulaHandler.h"//might have to remove

namespace Crucible
{
	class CombatResourceOwner :
		public ActorCombatValueOwner
	{
	public:

		CombatResourceContainer resourceContainer;


		float GetResource(CombatResource resource);
		float GetResourceBase(CombatResource resource);
		float GetResourceMax(CombatResource resource);
		float GetResourceModifier(CombatResource resource);
		float GetResourceDamage(CombatResource resource);


		void DamageResource(CombatResource resource, float damage);
		void RestoreResource(CombatResource resource, float restore);
		void ModResource(CombatResource resource, float mod);

	protected:
		void Init()
		{
			auto& poiseNode = resourceContainer[CombatResource::Poise];
			/*
			poiseNode.GetBase = [](RE::Actor* actor)->float
			{ 
				if (!actor)
					return 0;

				StatBase* raceStat = ItemStatHandler::GetStatBase(actor);

				float healthBase = actor->GetBaseActorValue(RE::ActorValue::kHealth);
				float healthTemp = actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kHealth);
				float healthPerm = actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kHealth);

				float maxHP = healthBase + healthTemp + healthPerm;

				float result = raceStat->GetValue(basePoise) + maxHP * raceStat->GetValue(poiseHealthMult);

				return result;
			};
			//*/
			poiseNode.baseValue.SetValue(RE::ActorValue::kHealth, IncludeFlags::kNoTemporary);
			poiseNode.formular = FormulaType::PoiseCalculation;
		}


	};
}