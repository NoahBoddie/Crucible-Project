#pragma once

#include "Combo.h"
#include "CombatData.h"
#include "RecoveryController.h"
#include "MagicStatController.h"

#include "Utility.h"
#include "ItemStatHandler.h"

namespace Crucible
{

	void  MeleeCombo::OnComboIncrease(unsigned int& inc) 
	{
		_confirmedHit = false;

		if (_comboCount >= 1) {
			//owner->resist -= negInc * inc;
			//owner->evade -= negInc * inc;
			//I don't think I'll need no cross.
			CombatValueInput input = CombatValueInput('MLE', ValueInputType::Modify, negInc * inc, true);
			owner->resist -= input;
			owner->evade -= input;
			//Utility::DebugMessageBox(std::format("{}\n{}\n{}", _comboCount, owner->resist.print(), owner->evade.print(), owner->evade.print()), IsPlayerRef());
		
			//if (_confirmedHit)
			//	throw nullptr;
		}
		//else { SetRegenSubscription(RE::ActorValue::kStamina, true); }
		//_confirmedHit = false;

	}


	void  MeleeCombo::OnSuccessIncrease(unsigned int& inc) 
	{
		if (_confirmedHit) {
			inc = 0;
			return;
		}
		//Feels like this isn't supposed to be here, maybe this is meant to be in start
		// or something.
		//owner->recoverControl.ClearRegenPool(static_cast<std::int32_t>(resource), RecoveryType::ActorValue);
		//owner->stagger += posInc * inc;
		owner->stagger += CombatValueInput('MLE', ValueInputType::Modify, posInc * inc, true);
		_confirmedHit = true;
	}


	void MeleeCombo::OnInterrupt()
	{
		TryFailure(Utility::GetAttackingWeapon(GetActorOwner()));
		__super::OnInterrupt();
	}


	void  MeleeCombo::HandleResourceReturn(ResolveType resolve)
	{
		RE::Actor* actorOwner = GetActorOwner();
		auto resource = RE::ActorValue::kStamina;

		float temp_LimitConst = 0.45f;
		float temp_staminaLossHitMult = 0.5f;

		float limit = _usedResource * temp_LimitConst;

		float loss = _staminaLostRate * (_comboCount - (_successCount * temp_staminaLossHitMult));

		float returnAmount = _usedResource - loss;

		returnAmount = returnAmount < limit ? limit : returnAmount;


		float avBase = actorOwner->GetBaseActorValue(resource);
		float avTemp = actorOwner->GetActorValueModifier(AVModifier::kTemporary, resource);
		float avPerm = actorOwner->GetActorValueModifier(AVModifier::kPermanent, resource);

		float max = avBase + avTemp + avPerm;

		float curr = max + actorOwner->GetActorValueModifier(AVModifier::kDamage, resource);

		float temp_penaltyThreshold = 0.35f;
		float temp_restoreBuffer = 0.05f;
		float temp_InstRestoreLimit = temp_penaltyThreshold + temp_restoreBuffer;


		float instRestoreAmount = 0;


		if ((curr / max) < temp_InstRestoreLimit)
		{
			//Should this be accounting for if the restore amount is too large?
			instRestoreAmount = (temp_InstRestoreLimit * max) - curr;

			if (instRestoreAmount > returnAmount)
				instRestoreAmount = returnAmount;
			//Utility::DebugMessageBox(std::format("B: {}, {}, {}", max, curr, temp_InstRestoreLimit));

			returnAmount -= instRestoreAmount;

			actorOwner->RestoreActorValue(AVModifier::kDamage, resource, instRestoreAmount);
			//curr = max + actorOwner->GetActorValueModifier(AVModifier::kDamage, av);

			if ((curr + instRestoreAmount) / max < temp_penaltyThreshold) {
				Utility::DebugMessageBox(std::format("Big penalty for {}, return {}", actorOwner->GetName(), instRestoreAmount));
			}
		}
		if (returnAmount > 0)
			owner->recoverControl.SetRegenPool(static_cast<std::int32_t>(resource), RecoveryType::ActorValue, returnAmount);

		Utility::DebugNotification(std::format("{}({}/{}) stamina returned to {}", returnAmount + instRestoreAmount,
			instRestoreAmount, returnAmount, actorOwner->GetName()).c_str());

	}


	void MeleeCombo::OnComboFinish(ResolveType resolve, bool resolved)
	{
		//This bit about penalty gets removed once the other method comes in.
		RE::Actor* actorOwner = GetActorOwner();

		if (!resolved)
		{
			auto resource = RE::ActorValue::kStamina;

			float avBase = actorOwner->GetBaseActorValue(resource);
			float avTemp = actorOwner->GetActorValueModifier(AVModifier::kTemporary, resource);
			float avPerm = actorOwner->GetActorValueModifier(AVModifier::kPermanent, resource);

			float max = avBase + avTemp + avPerm;

			float curr = max + actorOwner->GetActorValueModifier(AVModifier::kDamage, resource);

			float temp_penaltyThreshold = 0.35f;
			float temp_restoreBuffer = 0.05f;
			float temp_InstRestoreLimit = temp_penaltyThreshold + temp_restoreBuffer;

			if ((curr / max) < temp_InstRestoreLimit)
				Utility::DebugMessageBox(std::format("Big penalty for {}", actorOwner->GetName()), IsPlayerRef());

		}
		
		//owner->stagger += -posInc * _successCount;
		//owner->resist -= -negInc * (_comboCount - 1);
		//owner->evade -= -negInc * (_comboCount - 1);
		
		CombatValueInput delete_buildup = CombatValueInput('MLE', ValueInputType::Delete);
		owner->stagger += delete_buildup;
		owner->resist -= delete_buildup;
		owner->evade -= delete_buildup;

		Unregister();

	}


	void  MeleeCombo::OnComboResolve(ResolveType resolve)
	{
		HandleResourceReturn(resolve);

		_confirmedHit = false;

		auto cooldownEffect = Utility::GetEffectFromSetting(GetActorOwner(), Resources::paCooldown);

		//do this bit only if the reduction is not zero.

		if (cooldownEffect)
		{
			if (cooldownEffect->elapsedSeconds + _powerAttackReduction > cooldownEffect->duration) {
				//Utility::DebugMessageBox(std::format("old: {}, power reduct: {}", cooldownEffect->elapsedSeconds, powerAttackReduction));
				cooldownEffect->Dispel(true);
			}
			else {
				//float old = cooldownEffect->elapsedSeconds;
				cooldownEffect->elapsedSeconds += _powerAttackReduction;
				//Utility::DebugMessageBox(std::format("old: {}, new: {}, power reduct: {}", old, cooldownEffect->elapsedSeconds, powerAttackReduction));
			}
		}

		//Regen is paused while a combo is ongoing, but resumes after finishing.
		//SetRegenSubscription(RE::ActorValue::kStamina, false);
	}
		
	//Ported from weapon swing
	bool MeleeCombo::OnResult(bool success, RE::TESObjectWEAP* weap)
	{
		//A few notes, this should be the thing that marks success or not. It also should be split into multiple different functions.

		//A big note, later this will use weapon weight for the effort values. What I want to do
		// with fists is used some notion of weight in order to effect what that is. Weight on race
		// nor character is enough, I need to figure out something better. No worries atm, npcs wont need that.

		RE::Actor* actorOwner = GetActorOwner();

		if (!actorOwner)
			return false;

		RE::TESForm* form = CheckAttackForm(weap);

		StatBase* weaponStats = ItemStatHandler::GetStatBase(form);

		float weaponWeight = !weap ? 1 : weap->GetWeight();

		bool firstBlow = IsInFirstBlow();
		//May need - 1 on count
		//The first blow should always be temperate.  Doesn't matter if you hit or not so it can be used as a poke

		float temp_StaminaRateClimb = 15;
		//float& staminaLostRate = _staminaLostRate;
		float k_LerpPerc = 0.25f;
		//Dis lerp, put in utility.
		//staminaLostRate += (weaponStats->GetValue(staminaLossRateClimb) - staminaLostRate) * k_LerpPerc + staminaLostRate;
		_staminaLostRate += Utility::Lerp(weaponStats->GetValue(staminaLossRateClimb) - _staminaLostRate, _staminaLostRate, k_LerpPerc);
		//Experimenting with specific drop times now.


		float stamina = actorOwner->GetActorValue(RE::ActorValue::kStamina);

		if (Utility::IsPowerAttacking(actorOwner) == false)
		{
			float reduce = firstBlow ?
				weaponStats->GetValue(firstBlowPowerRestore) : success ?
				weaponStats->GetValue(hitPowerRestore) : weaponStats->GetValue(swingPowerRestore);


			_powerAttackReduction += reduce;//ModifyValue::CONFUTE



			float weightFactor = sqrt(weaponWeight) - 1;
			weightFactor = fmax(weightFactor, 1);

			//This is the functionality that I want to use for this, for now.
			float successMult = !success || firstBlow ? 1 : weaponStats->GetValue(staminaHitCostMult);
			float headValue = weaponStats->GetValue(firstBlow ? firstBlowCost : baseStaminaCost) * successMult * weightFactor;
			float accumulatingValue = _climbingValue + weaponStats->GetValue(staminaCostClimb) * (firstBlow ? 0 : successMult);

			//If the accumulating value is too static, I'll add the weight factor to it. Heavier weapons should be harder to handle.

			//Utility::DebugMessageBox(std::format("head {}, accum {}, stamina climb {}, climb value {}, first blow {}", headValue, accumulatingValue,
			//	weaponStats->GetValue(staminaCostClimb), _climbingValue, firstBlow), IsPlayerRef());

			float cost = headValue + accumulatingValue;
			_climbingValue = accumulatingValue;//SetValue::CONFUTE


			//The stamina cost only happens if it isn't a power attack.
			//float cost = firstBlow ?
			//	weaponStats->GetValue(firstBlowCost) :
			//	(weaponStats->GetValue(baseStaminaCost) + c_data->meleeCombo.count * weaponStats->GetValue(staminaCostClimb)) * (!success ? 1 : weaponStats->GetValue(staminaHitCostMult));


			//There are 2 notions in the above style, there is accumulated drawback, where regardless of how many hits you do, if you miss
			// for every attack that you've made you suffer, but hits are incredibly valuable,
			//AND then theres you're gonna carry that weight, which makes it so for each single swing it will store the value used
			// so if you keep being successful 1 dropped hit wont matter. But many dropped hits can't be fixed by suddenly hitting.
			cost = fmin(stamina, cost);

			_usedResource += fmax(0.f, cost);//ModifyValue::CONFUTE

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

		float currentDropTime = GetResolveDuration(true);
		float weaponDropTime = weaponStats->GetValue(statValue);

		//If the time is non-existent or this is the follow up attack, override. If not lerp.
		// Was better suited in the below format.
		//if (isPenalty || currentDropTime == 0 || _comboCount <= 2){SetResolveDuration(weaponDropTime);}
		//else{//This thing is supposed to change more drastically. Experiment with the numbers.//SetResolveDuration(currentDropTime);}

		float _k_temp_dropLerpPercent = 0.75f;
			
		currentDropTime = !isPenalty && currentDropTime && _comboCount > 2 ?
			Utility::Lerp(currentDropTime, weaponDropTime, _k_temp_dropLerpPercent) : weaponDropTime;


		SetResolveDuration(currentDropTime);

		//want to make stamina consumption tied to the stamina mult in the attack data at some point.

		if (success)
			SuccessIncrease();

		return false;
		//If the attack data
	}


	void BlockCombo::HandleBlockStamina(RE::Actor* actorOwner)
	{
		if (!actorOwner)
			return;

		float immediateReturn = fmin(_raiseStamina * 0.5f, _actionStamina);

		_actionStamina -= immediateReturn;

		actorOwner->RestoreActorValue(AVModifier::kDamage, RE::ActorValue::kStamina, immediateReturn);

		if (_actionStamina > 0)
			owner->recoverControl.IncRegenPool(static_cast<std::int32_t>(RE::ActorValue::kStamina), RecoveryType::ActorValue, _actionStamina);
	}


	/*
	//Merge this and magic, put in base combo
	void MeleeCombo::StartEndComboWindow()
	{
		if (!active)
			return;

		else if (count <= 1)
			_comboTimer.Start(k_preemptiveTime);//End(ResolveType::kNormal);
		else
			_comboTimer.Start(k_comboDropTime);

	}

	void MeleeCombo::ExtendEndComboWindow()
	{
		//This is effectively used by the other combo window everytime it increments a combo.
		//I actually thing I would like their combo timers to be synced.
		_comboTimer.Restart(k_comboDropTime);
	}

	void MeleeCombo::IncrementHit()
	{
		auto resource = GetResourceType();
		owner->recoverControl.ClearRegenPool(static_cast<std::int32_t>(resource), RecoveryType::ActorValue);

		if (!_confirmedHit) {
			hits++;
			owner->stagger += posInc;
			_confirmedHit = true;
		}
	}

	void MeleeCombo::OnInterrupt(EventObject& params, EventControl& control)
	{
		owner->swingEvent.TryFailure(Utility::GetAttackingWeapon(GetActorOwner()));
		__super::OnInterrupt(params, control);
	}

	void MeleeCombo::OnEnd(ResolveType resolve)
	{
		owner->stagger += -posInc * hits;
		owner->resist -= -negInc * (count - 1);
		owner->evade -= -negInc * (count - 1);
		hits = 0;
		
		_confirmedHit = false;
		_comboTimer.Stop();
		staminaLostRate = 0;
		climbingValue = 0;
		_comboDropTime = 0;

		auto cooldownEffect = Utility::GetEffectFromSetting(GetActorOwner(), Resources::paCooldown);

		//do this bit only if the reduction is not zero.

		if (cooldownEffect)
		{
			if (cooldownEffect->elapsedSeconds + powerAttackReduction > cooldownEffect->duration){
				//Utility::DebugMessageBox(std::format("old: {}, power reduct: {}", cooldownEffect->elapsedSeconds, powerAttackReduction));
				cooldownEffect->Dispel(true);
			}
			else {
				//float old = cooldownEffect->elapsedSeconds;
				cooldownEffect->elapsedSeconds += powerAttackReduction;
				//Utility::DebugMessageBox(std::format("old: {}, new: {}, power reduct: {}", old, cooldownEffect->elapsedSeconds, powerAttackReduction));
			}
		}

		auto actorOwner = GetActorOwner();

		//if (actorOwner && actorOwner->currentProcess && actorOwner->currentProcess->high)
		//	actorOwner->currentProcess->high->staminaRegenDelay = 0;
		//Regen is paused while a combo is ongoing, but resumes after finishing.
		SetRegenSubscription(RE::ActorValue::kStamina, false);

		owner->swingEvent.Unregister();


		powerAttackReduction = 0;
		//Utility::DebugNotification(std::format("{}: combo end", "Dumbass").c_str());
	}
	
	void MeleeCombo::OnComboIncrease()
	{
		_comboTimer.Stop();
		_confirmedHit = false;

		if (count > 1)
		{
			owner->resist -= negInc;
			owner->evade -= negInc;
		}
		else
		{
			auto actorOwner = GetActorOwner();

			//Regen is paused while a combo is ongoing, but resumes after finishing.

			//if (actorOwner->currentProcess && actorOwner->currentProcess->high)
			//	actorOwner->currentProcess->high->staminaRegenDelay = 999999;
			SetRegenSubscription(RE::ActorValue::kStamina, true);
		}
		
		//GetAttackingWeapon(RE::Actor * actor)
		//if dual attack, should get both, do later.
		//float dropTime = ItemStatHandler

		//_comboDropTime = _comboDropTime == 0 ? 
	}

	void MeleeCombo::ReleaseResource()
	{
		RE::Actor* actorOwner = GetActorOwner();
		auto resource = GetResourceType();

		float temp_LimitConst = 0.45f;
		float temp_staminaLossHitMult = 0.5f;

		float limit = usedResource * temp_LimitConst;

		float loss = staminaLostRate * (count - (hits * temp_staminaLossHitMult));

		float returnAmount = usedResource - loss;

		returnAmount = returnAmount < limit ? limit : returnAmount;


		float avBase = actorOwner->GetBaseActorValue(resource);
		float avTemp = actorOwner->GetActorValueModifier(AVModifier::kTemporary, resource);
		float avPerm = actorOwner->GetActorValueModifier(AVModifier::kPermanent, resource);

		float max = avBase + avTemp + avPerm;

		float curr = max + actorOwner->GetActorValueModifier(AVModifier::kDamage, resource);

		float temp_penaltyThreshold = 0.35f;
		float temp_restoreBuffer = 0.05f;
		float temp_InstRestoreLimit = temp_penaltyThreshold + temp_restoreBuffer;

		
		float instRestoreAmount = 0;


		if ((curr / max) < temp_InstRestoreLimit)
		{
			//Should this be accounting for if the restore amount is too large?
			instRestoreAmount = (temp_InstRestoreLimit * max) - curr;

			if (instRestoreAmount > returnAmount)
				instRestoreAmount = returnAmount;
			//Utility::DebugMessageBox(std::format("B: {}, {}, {}", max, curr, temp_InstRestoreLimit));

			returnAmount -= instRestoreAmount;

			actorOwner->RestoreActorValue(AVModifier::kDamage, resource, instRestoreAmount);
			//curr = max + actorOwner->GetActorValueModifier(AVModifier::kDamage, av);
			
			if ((curr + instRestoreAmount) / max < temp_penaltyThreshold) {
				Utility::DebugMessageBox(std::format("Big penalty for {}, return {}", actorOwner->GetName(), instRestoreAmount));
			}
		}
		if (returnAmount > 0)
			owner->recoverControl.SetRegenPool(static_cast<std::int32_t>(resource), RecoveryType::ActorValue, returnAmount);
	
		Utility::DebugNotification(std::format("{}({}/{}) stamina returned to {}", returnAmount + instRestoreAmount,
			instRestoreAmount, returnAmount, actorOwner->GetName()).c_str());

	}





	bool MagicCombo::ShouldComboIncrease(MagicCasterData* castData)
	{
		if (!castData)
			return false;

		return castData->previousState != uMagicCastState::kConcentrating || castData->castTime == 0;
	}

	void MagicCombo::OnComboIncrease(MagicCasterData* castData)
	{
		if (!castData)
			return;

		float concTime = castData->previousState == uMagicCastState::kConcentrating ?
			castData->castTime : 1.f;
		
		Utility::DebugNotification(std::format("cast concTime", concTime).c_str());

		auto actorOwner = GetActorOwner();

		//if (actorOwner && actorOwner->currentProcess && actorOwner->currentProcess->high)
		//	actorOwner->currentProcess->high->magickaRegenDelay = 999999999;
		SetRegenSubscription(RE::ActorValue::kMagicka, true);


		switch (castData->spellType)
		{
		case ItemType::kSpell:
			magickaSpent += castData->resourceStore;
			magickaRestore += castData->resourceStore;
			dualCastReduction += (int)castData->skillLevel * 2.5f * concTime;
			dualCastCooldown += (int)castData->skillLevel * 4.f * concTime;
			break;

		case ItemType::kStaff:
			magickaRestore += 0.5f * castData->resourceStore;//cant remember if it was restore or spent

			break;

		case ItemType::kScroll://This is the reason I'm using skill level. For now, me no care. 
			dualCastReduction += (int)castData->skillLevel * 2.5f;
			break;
		}
	}


	void MagicCombo::OnEnd(ResolveType resolve)
	{
		
		
		auto actorOwner = GetActorOwner();

		int cooldownTime = dualCastCooldown - dualCastReduction;

		if (cooldownTime < 0)
		{
			//check for magick effect, if it's here reduce it.
			auto cooldownEffect = Utility::GetEffectFromSetting(GetActorOwner(), Resources::dcCooldown);

			if (cooldownEffect)
			{
				if (cooldownEffect->elapsedSeconds + -cooldownTime > cooldownEffect->duration) {
					cooldownEffect->Dispel(true);
				}
				else {
					cooldownEffect->elapsedSeconds -= cooldownTime;
				}
			}
		}
		else if (cooldownTime > 0.2f)
		{
			//Utility::SetMagicItemValues(Resources::paCooldownSpell, 2, cooldownTime);
			Resources::dcCooldownSpell->effects[0]->effectItem.duration = cooldownTime;
			actorOwner->DoCombatSpellApply(Resources::dcCooldownSpell, actorOwner);
		}
		
		_comboTimer.Stop();

		

		//if (actorOwner && actorOwner->currentProcess && actorOwner->currentProcess->high)
		//	actorOwner->currentProcess->high->magickaRegenDelay = 0;
		SetRegenSubscription(RE::ActorValue::kMagicka, false);


		magickaSpent = 0;
		dualCastReduction = 0;
		dualCastCooldown = 0;
		
		//Utility::DebugNotification(std::format("{}: combo end", "VaN").c_str());
		
	}

	void MagicCombo::ReleaseResource()
	{
		//return;//Disabled for now, having this fire with nothing causes issues. So no deal for now.

		owner->magicControl.SendCastData();
		__super::ReleaseResource();//At some point I'll swap this out. Now aint the time I will.
		//RE::Actor* actorOwner = GetActorOwner();
		//actorOwner->RestoreActorValue(AVModifier::kDamage, GetResourceType(), usedResource);
	}

	void MagicCombo::StartEndComboWindow()
	{
		if (active)
			return;
		
		_comboTimer.Start(k_Temp_MagicComboTimer);

	}
	
	void MagicCombo::StopEndComboWindow()
	{
		if (!active)
			return;
		
		_comboTimer.Stop();

	}

	void MagicCombo::ExtendEndComboWindow()
	{
		_comboTimer.Restart(k_Temp_MagicComboTimer);
	}

	//*/
}