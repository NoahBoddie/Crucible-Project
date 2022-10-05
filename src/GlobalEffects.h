#pragma once

#include "CombatValue.h"
#include "CombatData.h"
#include "Utility.h"
#include "QueryContext.h"
#include "ItemStatHandler.h"

namespace Crucible
{
	namespace GlobalEffect
	{
		void Blocking_Resist(CombatValue valueType, SimpleCombatValue& value, QueryContext& context)
		{

			//At a later point, a lot of these things are up to the block stat controller.

			RE::Actor* defender = context.GetForm<RE::Actor>(queryDefender);

			CombatData* combatData = CombatData::GetDataRecord(defender);


			if (!defender && !combatData)
				return;

			//Later replace this with a proper check that may account for wards or other states
			//At a later point, this is something that is likely gonna be more linked with the 
			// block stuff
			bool isBlocking = false;//Utility::GetGraphVariableBool(graphManager, "IsSprinting");
			//Ask via guard state instead, because some ward types will block.
			defender->GetGraphVariableBool("IsBlocking", isBlocking);

			if (isBlocking)
			{
				//Note, I don't really need this anymore.
				//auto defendingClass = combatData->guardControl.GetDefendingClass();//Utility::GetDefendingClass(combatData->shield);
				RE::TESForm* defend_form = combatData->guardControl.GetDefendingForm();

				if (!defend_form)
					return;

				bool blocked = context.GetBool(queryBlocked);
				bool raw = context.GetBool(queryRaw);

				if (blocked || raw)
				{
					/*
					switch (defendingClass)
					{
					case WeightClass::kLight:
						value += 1;
						break;

					case WeightClass::kMedium:
						value += 2;
						break;

					case WeightClass::kHeavy:
						value += 3;
						break;
					}
					//*/

					value += ItemStatHandler::GetStatValue(defend_form, blockResist);
					//Utility::DebugNotification(std::format("resist up {}, hit blocked", value.positiveBase), defender->IsPlayerRef());
				}
				else
				{
					
					value -= ItemStatHandler::GetStatValue(defend_form, unguardedPenalty);
					Utility::DebugNotification(std::format("resist down {} to blocking", value.negativeBase), defender->IsPlayerRef());
				}
			}

		}
	

		void PowerAction_Stagger(CombatValue valueType, SimpleCombatValue& value, QueryContext& context)
		{
			bool powerAction = context.GetBool(queryPowerAction);

			if (!powerAction) {
				return;
			}

			bool isBash = context.GetBool(queryBash);

			//Utility::DebugNotification("PowerAction");
			//Either make this a setting, or make this an item stat.
			value += isBash ? 1 : 2;
			value *= isBash ? 0.5f : 0;
		}
		
		void CounterBash_Stagger(CombatValue valueType, SimpleCombatValue& value, QueryContext& context)
		{
			bool counter = context.GetBool(queryCounter);

			if (!counter) {
				return;
			}

			auto* source = context.GetForm(querySource);

			auto* shield_stat = ItemStatHandler::GetStatBase<ShieldStat>(source);

			if (!shield_stat){
				return;
			}

			float counter_bash_power = shield_stat->GetValue(counterBashStagger);

			//Utility::DebugMessageBox("Counter Bash", context.isPlayerInvolved);

			value += counter_bash_power;
		}


		void TimedBash_Resist(CombatValue valueType, SimpleCombatValue& value, QueryContext& context)
		{
			//Currently, this is being retooled for usage on resist.
			bool isTimed = context.GetBool(queryTimed);

			if (!isTimed) {
				return;
			}

			//Utility::DebugMessageBox("Timed Bash", context.isPlayerInvolved);


			value -= 4;
		}
	
	}
}