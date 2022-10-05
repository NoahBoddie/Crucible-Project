#pragma once
#include "WeaponSwing.h"
#include "CombatData.h"
#include "Utility.h"
#include "ItemStatHandler.h"

namespace Crucible
{
	bool WeaponSwing::OnResult(bool success, RE::TESObjectWEAP* weap)
	{
#ifdef turning_off_old_weaponswing

		//A big note, later this will use weapon weight for the effort values. What I want to do
		// with fists is used some notion of weight in order to effect what that is. Weight on race
		// nor character is enough, I need to figure out something better. No worries atm, npcs wont need that.

		auto c_data = GetOwner();

		if (!c_data)
			return false;

		RE::Actor* actorOwner = GetActorOwner();

		if (!actorOwner)
			return false;

		RE::TESForm* form = CheckAttackForm(weap);

		StatBase* weaponStats = ItemStatHandler::GetStatBase(form);

		float weaponWeight = !weap ? 1 : weap->GetWeight();

		bool firstBlow = c_data->meleeCombo.IsInFirstBlow();
		//May need - 1 on count
		//The first blow should always be temperate.  Doesn't matter if you hit or not so it can be used as a poke
		
		float temp_StaminaRateClimb = 15;
		float& staminaLostRate = c_data->meleeCombo.staminaLostRate;
		float k_LerpPerc = 0.25f;
		//Dis lerp, put in utility.
		//staminaLostRate += (weaponStats->GetValue(staminaLossRateClimb) - staminaLostRate) * k_LerpPerc + staminaLostRate;
		staminaLostRate += Utility::Lerp(weaponStats->GetValue(staminaLossRateClimb) - staminaLostRate, staminaLostRate,  k_LerpPerc);
		//Experimenting with specific drop times now.
		
		
		float stamina = actorOwner->GetActorValue(RE::ActorValue::kStamina);

		if (Utility::IsPowerAttacking(actorOwner) == false)
		{
			float reduce = firstBlow ?
				weaponStats->GetValue(firstBlowPowerRestore) : success ?
				weaponStats->GetValue(hitPowerRestore) : weaponStats->GetValue(swingPowerRestore);


			c_data->meleeCombo.powerAttackReduction += reduce;

			//16
			//4
			//successmult 1



			float weightFactor = sqrt(weaponWeight) - 1;
			weightFactor = fmax(weightFactor, 1);

			//This is the functionality that I want to use for this, for now.
			float successMult = !success || firstBlow ? 1 : weaponStats->GetValue(staminaHitCostMult);
			float headValue = weaponStats->GetValue(firstBlow ? firstBlowCost : baseStaminaCost) * successMult * weightFactor;
			float accumulatingValue = c_data->meleeCombo.climbingValue + weaponStats->GetValue(staminaCostClimb) * (firstBlow ? 0 : successMult);

			//If the accumulating value is too static, I'll add the weight factor to it. Heavier weapons should be harder to handle.

			//Utility::DebugNotification(std::format("head {}, accum {}, stamina climb {}, climb value {}", headValue, accumulatingValue,
			//	weaponStats->GetValue(staminaCostClimb), c_data->meleeCombo.climbingValue).c_str());

			float cost = headValue + accumulatingValue;
			c_data->meleeCombo.climbingValue = accumulatingValue;
			

			//The stamina cost only happens if it isn't a power attack.
			//float cost = firstBlow ?
			//	weaponStats->GetValue(firstBlowCost) :
			//	(weaponStats->GetValue(baseStaminaCost) + c_data->meleeCombo.count * weaponStats->GetValue(staminaCostClimb)) * (!success ? 1 : weaponStats->GetValue(staminaHitCostMult));


			//There are 2 notions in the above style, there is accumulated drawback, where regardless of how many hits you do, if you miss
			// for every attack that you've made you suffer, but hits are incredibly valuable,
			//AND then theres you're gonna carry that weight, which makes it so for each single swing it will store the value used
			// so if you keep being successful 1 dropped hit wont matter. But many dropped hits can't be fixed by suddenly hitting.
			cost = fmin(stamina, cost);

			c_data->meleeCombo.usedResource += fmax(0.f, cost);

			actorOwner->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, -cost);
			stamina -= cost;
		}
		else
		{
			//If it's successful, set to reduce time. unless it's the first blow.
			int cooldownTime = firstBlow ?
				weaponStats->GetValue(firstBlowPowerCooldown) : success ?
				weaponStats->GetValue(successPowerCooldown) : weaponStats->GetValue(failurePowerCooldown);
			
			//Note, this will not accept zero as a viable answer (at the moment, later I can use an error value as an idea)
			Utility::SetMagicItemValues(Resources::paCooldownSpell, EffectValue::Duration, cooldownTime);
			//Resources::paCooldownSpell->effects[0]->effectItem.duration = cooldownTime;

			actorOwner->DoCombatSpellApply(Resources::paCooldownSpell, actorOwner);
		}
		
		//For now, specific values, I couldn't give a fuck to be honest.
		auto isPenalty = stamina <= 10;
		
		auto statValue = isPenalty ?
			penaltyComboTime : firstBlow ?
			firstComboTime : followComboTime;

		float currentDropTime = c_data->meleeCombo.GetComboDropTime();
		float weaponDropTime = weaponStats->GetValue(statValue);

		//If the time is non-existent or this is the follow up attack, override. If not lerp.
		if (isPenalty || currentDropTime == 0 || c_data->meleeCombo.GetCount() <= 2)
		{
			c_data->meleeCombo.SetComboDropTime(weaponDropTime);
		}
		else
		{
			//This thing is supposed to change more drastically. Experiment with the numbers.
			currentDropTime = Utility::Lerp(currentDropTime, weaponDropTime, 0.75f);
			c_data->meleeCombo.SetComboDropTime(currentDropTime);
		}

		
		//want to make stamina consumption tied to the stamina mult in the attack data at some point.
		
		
#endif
		return false;
		//If the attack data
	}

}