#pragma once
//Move owner to source
#include "Resources.h"
#include "CombatData.h"
#include "CombatDataBehaviour.h"
#include "CombatResource.h"
#include "CombatResourceOwner.h"
#include "RecoveryController.h"

#include "ModSettingHandler.h"

namespace Crucible
{
	RecoveryType RecoveryController::GetValueType(std::int32_t& value)
	{
		//Compares the sign bit to see should it be actor value
		// or combat resource respective to true or false.
		RecoveryType result = value & sign_bit ? RecoveryType::ActorValue : RecoveryType::CombatResource;
		
		//Also cleans the flag out for av usage.
		 value &= ~sign_bit;

		return result;
	}

	//Both functions need to recieve more. Like the event thats supposed to be sent. for now, we only have 1 
	// so it's alright.
	void RecoveryController::EndRecovery(RecoveryNode& node, std::int32_t valueType, RecoveryFinishReason reason)
	{
		if (node.isRecovering)
		{
			node.isRecovering = false;


			if (reason == RecoveryFinishReason::noComment)
				return;
			//This is triggered when it isn't supposed to be, a pooled regen 
			//Utility::DebugNotification("End Rejuv");
			
			//Legacy version
			//EventObject params;
			//params.PushInt(valueType);
			//params.PushEnum(reason);

			owner->SendEvent("OnRecoveryFinish", valueType, reason);
		}
	}

	void RecoveryController::StartRecovery(RecoveryNode& node, std::int32_t valueType)
	{
		if (!node.isRecovering)
		{
			node.isRecovering = true;
			//Utility::DebugMessageBox("Recovery Start");

			//The pool needs to be cleared upon finishing here.
			//Utility::DebugNotification(std::format("Start Rejuv {}", node.GetRegenPool()).c_str());

			//Legacy
			//EventObject params;
			//params.PushString("Poise");
			//params.PushInt(valueType);

			owner->SendEvent("OnRecoveryStart", valueType);
		}
	}
	

	/*
	float RecoveryController::GetRegenValue(std::int32_t value, RE::Actor* actor)
	{
		//really, the actor thing is just incase you already looked them up.
		actor = actor ? actor : GetActorOwner();

		RecoveryType type = GetValueType(value);

		switch (type)
		{
		case RecoveryType::ActorValue:
			//Utility::DebugNotification(std::format("Value {}, healrate {}, equal? {}, equal enumerated? {}",
			//	value, (int)RE::ActorValue::kHealRate, value == (int)RE::ActorValue::kHealRate, RE::ActorValue(value) == RE::ActorValue::kHealRate).c_str());

			return actor->GetActorValue(static_cast<RE::ActorValue>(value));//RE::ActorValue::kHealRate);//
		case RecoveryType::CombatResource:
			return owner->GetResource(static_cast<CombatResource>(value));
		}
	}

	//Still want to merge these
	void RecoveryController::HandleActorValue(RE::ActorValue av, RecoveryNode& node, float updateDelta)
	{
		RE::Actor* actor = GetActorOwner();

		if (!actor)
			return;

		float damageValue = -actor->GetActorValueModifier(AVModifier::kDamage, av);

		if (damageValue <= 0) {
			EndRecovery(node, (__int32)av, RecoveryFinishReason::maxValue);
			return;
		}

		float avBase = actor->GetBaseActorValue(av);
		float avTemp = actor->GetActorValueModifier(AVModifier::kTemporary, av);
		float avPerm = actor->GetActorValueModifier(AVModifier::kPermanent, av);

		float max = avBase + avTemp + avPerm;

		float freeDiff = damageValue > max ? damageValue - max : 0;


		float recover = GetRegenValue(node.regenValue, actor);
		recover = recover < node.regenMinValue ? node.regenMinValue : recover;
		recover = (recover * node.regenValueMult) / 100.f;
		
		

		recover *= max * updateDelta;

		float pool = node.GetRegenPool();

		if (pool >= 0)
		{
			if (pool < recover) {
				recover = pool;
				pool = 0;
			}
			else {
				pool -= recover;
			}
		}

		//Utility::DebugNotification(std::format("{} update, {} delta, {} max", recover, updateDelta, max).c_str());

		actor->RestoreActorValue(AVModifier::kDamage, av, recover + freeDiff);

		damageValue = -actor->GetActorValueModifier(AVModifier::kDamage, av);

		if (node.isRecovering == false) {
			StartRecovery(node, (__int32)av);
		}

		if (damageValue <= 0) {
			//fully healed, send message.
			node.ClearRegenPool();//All pool types expire after reaching max value.
			EndRecovery(node, (__int32)av, RecoveryFinishReason::maxValue);
		}
		else if (pool >= 0){
			node.SetRegenPool(pool);

			if (pool == 0) {
				EndRecovery(node, (__int32)av, RecoveryFinishReason::exhaustPool);
			}
		}
	}

	void RecoveryController::HandleCombatResource(CombatResource cr, RecoveryNode& node, float updateDelta)
	{
		//My theory is that this value does not translate to being 0, or rather it doesn't become poise.
		float damageValue = owner->GetResourceDamage(cr);

		if (damageValue <= 0) {
			EndRecovery(node, (__int32)cr | sign_bit, RecoveryFinishReason::maxValue);
			return;
		}

		//these can get merged, the only difference is the use functions
		float max = owner->GetResourceMax(cr);

		//CALLER if the damage exceeds the max, make up the difference for free
		float freeDiff = damageValue > max ? damageValue - max : 0;

		float recover = GetRegenValue(node.regenValue);

		recover = recover > node.regenMinValue ? recover : node.regenMinValue;

		recover = (recover * node.regenValueMult) / 100.f;

		recover *= max * updateDelta;

		float pool = node.GetRegenPool();

		if (pool >= 0)
		{
			if (pool < recover) {
				recover = pool;
				pool = 0;
			}
			else {
				pool -= recover;
			}
		}

		//Utility::DebugNotification(std::format("{} / {} / {}", recover, freeDiff, updateDelta).c_str());

		owner->RestoreResource(cr, recover + freeDiff);

		damageValue = owner->GetResourceDamage(cr);

		//This always seems to start AND in within the exact same spot.

		
		StartRecovery(node, (__int32)cr | sign_bit);
		

		if (damageValue <= 0) {
			//fully healed, send message.
			EndRecovery(node, (__int32)cr | sign_bit, RecoveryFinishReason::maxValue);
			node.SetRegenPool(0);//All pool types expire after reaching max value. Non pools won't set.
		}
		else if (pool >= 0) {
			node.SetRegenPool(pool);

			if (pool == 0) {
				EndRecovery(node, (__int32)cr | sign_bit, RecoveryFinishReason::exhaustPool);
			}
		}
	}



	//Should be able to take static focused delegates too.
	void RecoveryController::AddRecoveryNode(std::int32_t restoreValue, std::int32_t regenValue, RecoveryType type, bool poolType, float recoveryMin, float recoveryMult)
	{
		return;// Deprecated rn

		auto originalValue = restoreValue;

		restoreValue = type == RecoveryType::ActorValue ? restoreValue | sign_bit : restoreValue;

		if (_recoveryMap.contains(restoreValue) == true)
			return;

		RecoveryNode& node = _recoveryMap[restoreValue];
		return;

		node._regenPool = poolType ? 0 : -1;


		node.regenValue = regenValue;
		node.regenMinValue = recoveryMin;
		node.regenValueMult = recoveryMult;

		type = GetValueType(regenValue);

		if (type == RecoveryType::ActorValue)
			node.regenValue2.SetValue(static_cast<RE::ActorValue>(regenValue), IncludeFlags::kAll);
		else
			node.regenValue2.SetValue(static_cast<CombatResource>(regenValue), IncludeFlags::kAll);
	}

	//*/


	void RecoveryController::HandleRecovery(RecoveryType type, std::int32_t value, RecoveryNode& node, float updateDelta)
	{
		//My theory is that this value does not translate to being 0, or rather it doesn't become poise.
		float damageValue = 0;//owner->GetResourceDamage(cr);

		RE::ActorValue av = static_cast<RE::ActorValue>(value);
		CombatResource cr = static_cast<CombatResource>(value);
		RE::Actor* actor = nullptr;

		std::int32_t nodeIdentity = type == RecoveryType::CombatResource ? value | sign_bit : value;

		if (!owner)
			return;
		switch (type)
		{
		case RecoveryType::CombatResource:
			damageValue = owner->GetResourceDamage(cr);
			break;
		case RecoveryType::ActorValue:
			actor = owner->GetOwner();

			if (!actor)
				return;

			damageValue = -actor->GetActorValueModifier(AVModifier::kDamage, av);
			break;
		}

		if (damageValue <= 0) {
			EndRecovery(node, nodeIdentity, RecoveryFinishReason::maxValue);
			return;
		}

		//these can get merged, the only difference is the use functions
		float max = 0;//owner->GetResourceMax(cr);
		
		switch (type)
		{
		case RecoveryType::CombatResource:
			max = owner->GetResourceMax(cr);
			break;
		case RecoveryType::ActorValue:
		{
			float avBase = actor->GetBaseActorValue(av);
			float avTemp = actor->GetActorValueModifier(AVModifier::kTemporary, av);
			float avPerm = actor->GetActorValueModifier(AVModifier::kPermanent, av);

			max = avBase + avTemp + avPerm;
		}
			break;
		}

		//CALLER if the damage exceeds the max, make up the difference for free
		float freeDiff = damageValue > max ? damageValue - max : 0;

		RecoverySetting* setting = node.GetSetting();

		float recover = setting->regenValue.GetValue(owner);//GetRegenValue(node.regenValue);

		
		float ori = recover;
		recover = recover > setting->recoveryMin ? recover : setting->recoveryMin;

		recover = (recover * setting->recoveryRate) / 100.f;


		recover *= max * updateDelta;

		float pool = node.GetRegenPool();

		if (pool >= 0)
		{
			if (pool < recover) {
				recover = pool;
				pool = 0;
			}
			else {
				pool -= recover;
			}
		}

		//A note, this will need a safety for noticably large updates, maybe something like frame hangs or something like that.

		//Utility::DebugNotification(std::format("{} / {} / {}", recover, freeDiff, updateDelta).c_str());

		//Utility::DebugNotification(std::format("{} update, {} delta, {} max, {} min, {} rate, {} ori", recover, updateDelta, max,
		//	setting->recoveryMin, setting->recoveryRate, ori).c_str());

		
		switch (type)
		{
		case RecoveryType::CombatResource:
			owner->RestoreResource(cr, recover + freeDiff);
			break;
		case RecoveryType::ActorValue:
			actor->RestoreActorValue(AVModifier::kDamage, av, recover + freeDiff);
			break;
		}

		switch (type)
		{
		case RecoveryType::CombatResource:
			damageValue = owner->GetResourceDamage(cr);
			break;
		case RecoveryType::ActorValue:
			damageValue = -actor->GetActorValueModifier(AVModifier::kDamage, av);
			break;
		}
		 
		
		//This always seems to start AND in within the exact same spot.


		StartRecovery(node, nodeIdentity);


		if (damageValue <= 0) {
			//fully healed, send message.
			EndRecovery(node, nodeIdentity, RecoveryFinishReason::maxValue);
			node.SetRegenPool(0);//All pool types expire after reaching max value. Non pools won't set.
		}
		else if (pool >= 0) {
			node.SetRegenPool(pool);

			if (pool == 0) {
				EndRecovery(node, nodeIdentity, RecoveryFinishReason::exhaustPool);
			}
		}
	}


	void RecoveryController::OnUpdate(RE::Actor* actor, float updateDelta)
	{
		//This shit doesn't work on regular actors, my hand is forced. :pensive:
		if (updateDelta == 0)
			updateDelta = TimeManager::GetDeltaTime();

		for (auto& entry : *_recoveryMap)
		{
			float tempDelta = updateDelta;

			//Player seems to leave the loop, belethor does not.
			std::int32_t value = entry.first;

			if (entry.second.IsSubscribed() == true)
				continue;//Tis merely paused, carry on.

			if (entry.second.GetRegenPool() == 0) {
				//It's unlikely for this to be the thing to encounter this being zero but perhaps it should
				// send an event.
				EndRecovery(entry.second, value);
				continue;
			}
			else if (entry.second.regenDelay > 0) {

				entry.second.regenDelay -= updateDelta;

				if (entry.second.regenDelay <= 0) {
					tempDelta = -entry.second.regenDelay;
					entry.second.regenDelay = 0;
					//Utility::DebugNotification("br");
				}
				else {
					EndRecovery(entry.second, value);
					//Utility::DebugNotification("be");
					continue;

				}
			}

			//Issue is actor value falls through. Fix it.
			//auto before = value;
			RecoveryType type = GetValueType(value);

			//Utility::DebugNotification(std::format("{} dt, {} before", value, before).c_str());

			HandleRecovery(type, value, entry.second, tempDelta);
		}
		
		if (!_recoverContinueQueue->size() || !actor || !actor->currentProcess || !actor->currentProcess->high)
			return;

		for (auto& [actor_value, set_value] : *_recoverContinueQueue)
		{
			switch (actor_value)
			{
			case RE::ActorValue::kHealth:
				actor->currentProcess->high->healthRegenDelay = set_value;
				Utility::DebugMessageBox("resuming health", IsPlayerRef());
				break;

			case RE::ActorValue::kMagicka:
				actor->currentProcess->high->magickaRegenDelay = set_value;
				Utility::DebugMessageBox("resuming magicka", IsPlayerRef());
				break;

			case RE::ActorValue::kStamina:
				actor->currentProcess->high->staminaRegenDelay = set_value;
				Utility::DebugMessageBox("resuming stamina", IsPlayerRef());
				break;
			}
		}

		/*
		do
		{
			size--;

			auto& restore_pair = _recoverContinueQueue[size];

			RE::ActorValue& actor_value = restore_pair.first;
			float& set_value = restore_pair.second;

			switch (actor_value)
			{
			case RE::ActorValue::kHealth:
				actor->currentProcess->high->healthRegenDelay = set_value;
				break;

			case RE::ActorValue::kMagicka:
				actor->currentProcess->high->magickaRegenDelay = set_value;
				break;

			case RE::ActorValue::kStamina:
				actor->currentProcess->high->staminaRegenDelay = set_value;
				break;
			}
			//_recoverContinueQueue->pop_back();
		} while (size);
		//*/
		_recoverContinueQueue->clear();
	}


	void RecoveryController::AddRecoveryNode(CombatResource restoreValue, RecoverValue recoverType)
	{
		std::int32_t gotoValue = (std::int32_t)restoreValue;

		AddRecoveryNode(gotoValue, recoverType);
	}


	void RecoveryController::AddRecoveryNode(RE::ActorValue restoreValue, RecoverValue recoverType)
	{
		std::int32_t gotoValue = (std::int32_t)restoreValue | sign_bit;

		AddRecoveryNode(gotoValue, recoverType);
	}

	void RecoveryController::AddRecoveryNode(std::int32_t gotoValue, RecoverValue recoverType)
	{
		if (_recoveryMap->contains(gotoValue) == true)
			return;

		RecoveryNode& node = _recoveryMap[gotoValue];
		node.recoverSettings = recoverType;

		//Utility::DebugMessageBox(std::format("{}", gotoValue));
		if (node.IsPooled() == true)
			node._regenPool = 0;
	}

	void RecoveryController::Build()
	{
		//Utility::DebugMessageBox("dd");
		//This will not reload via serialization, instead this happens first, then
		// it's data should be loaded. A specific object may be needed to handle this.
		AddRecoveryNode(RE::ActorValue::kStamina, RecoverValue::kStamina);
		AddRecoveryNode(CombatResource::Poise, RecoverValue::kPoise);		
	}
}