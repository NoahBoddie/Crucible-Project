#pragma once
#include "CombatResource.h"
#include "ActorCombatValueOwner.h"

#include "ReadableValue.h"
#include "FormulaHandler.h"//might have to remove

#include "SerialComponent.h"
#include "SerializableMap.h"

namespace Crucible
{
	using CombatResourceContainer = SerializableMap<CombatResource, SimpleCombatResource>;


	class CombatResourceOwner :
		public ActorCombatValueOwner,
		public SerialComponent<CombatResourceContainer>
	{
		//using CombatResourceContainer = std::map<const CombatResource, SimpleCombatResource>;
	public:

		CombatResourceContainer& resourceContainer = data;


		float GetResource(CombatResource resource);
		float GetResourceUnclamped(CombatResource resource);
		float GetResourceBase(CombatResource resource);
		float GetResourceMax(CombatResource resource);
		float GetResourcePercent(CombatResource resource);
		float GetResourceModifier(CombatResource resource);
		float GetResourceDamage(CombatResource resource);


		void DamageResource(CombatResource resource, float damage);
		void RestoreResource(CombatResource resource, float restore);
		void ModResource(CombatResource resource, float mod);

	protected:
		void Init()
		{
			//This needs to change, there needs to be a initializing set up like behaviour does.

			auto& poiseNode = resourceContainer[CombatResource::Poise];
			poiseNode = SimpleCombatResource(RE::ActorValue::kHealth, IncludeFlags::kMaxNoTemporary, FormulaType::PoiseCalculation);
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
			//poiseNode.baseValue.SetValue(RE::ActorValue::kHealth, IncludeFlags::kMax ^ IncludeFlags::kTemporary);
			//poiseNode.formular = FormulaType::PoiseCalculation;
		}
	};
}