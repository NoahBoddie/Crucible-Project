#pragma once

#include "Combo.h"
#include "CombatData.h"
#include "RecoveryController.h"
#include "MagicStatController.h"

namespace Crucible
{

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

		if (cooldownEffect)
		{
			if (cooldownEffect->elapsedSeconds + powerAttackReduction > cooldownEffect->duration){
				cooldownEffect->Dispel(true);
			}
			else {
				cooldownEffect->elapsedSeconds += powerAttackReduction;
			}
		}

		auto actorOwner = GetActorOwner();

		if (actorOwner && actorOwner->currentProcess && actorOwner->currentProcess->high)
			actorOwner->currentProcess->high->staminaRegenDelay = 0;
			//Regen is paused while a combo is ongoing, but resumes after finishing.


		powerAttackReduction = 0;
		RE::DebugNotification(std::format("{}: combo end", "Dumbass").c_str());
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
		
		//GetAttackingWeapon(RE::Actor * actor)
		//if dual attack, should get both, do later.
		//float dropTime = ItemStatHandler

		//_comboDropTime = _comboDropTime == 0 ? 
	}

	void MeleeCombo::ReleaseResource()
	{
		RE::Actor* actorOwner = GetActorOwner();
		auto resource = GetResourceType();

		float limit = usedResource * 0.6f;

		float loss = staminaLostRate * (count - (hits * 0.5f));

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

		RE::DebugNotification(std::format("total returned to {}: {}", actorOwner->GetName(), returnAmount).c_str());


		if ((curr / max) < temp_InstRestoreLimit)
		{
			//Should this be accounting for if the restore amount is too large?
			float instRestoreAmount = (temp_InstRestoreLimit * max) - curr;

			if (instRestoreAmount > returnAmount)
				instRestoreAmount = returnAmount;
			//RE::DebugMessageBox(std::format("B: {}, {}, {}", max, curr, temp_InstRestoreLimit));

			returnAmount -= instRestoreAmount;

			actorOwner->RestoreActorValue(AVModifier::kDamage, resource, instRestoreAmount);
			//curr = max + actorOwner->GetActorValueModifier(AVModifier::kDamage, av);
			
			if ((curr + instRestoreAmount) / max < temp_penaltyThreshold) {
				RE::DebugMessageBox(std::format("Big penalty for {}, return {}", actorOwner->GetName(), instRestoreAmount));
			}
		}
		if (returnAmount > 0)
			owner->recoverControl.SetRegenPool(static_cast<std::int32_t>(resource), RecoveryType::ActorValue, returnAmount);
	
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
		
		RE::DebugNotification(std::format("cast concTime", concTime).c_str());

		auto actorOwner = GetActorOwner();

		if (actorOwner && actorOwner->currentProcess && actorOwner->currentProcess->high)
			actorOwner->currentProcess->high->magickaRegenDelay = 999999999;

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
		//*
		
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

		

		if (actorOwner && actorOwner->currentProcess && actorOwner->currentProcess->high)
			actorOwner->currentProcess->high->magickaRegenDelay = 0;
		

		magickaSpent = 0;
		dualCastReduction = 0;
		dualCastCooldown = 0;
		
		RE::DebugNotification(std::format("{}: combo end", "VaN").c_str());
		//*/
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
}