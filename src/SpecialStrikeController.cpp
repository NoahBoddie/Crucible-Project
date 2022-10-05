#pragma once

#include "Timer.h"
#include "Utility.h"
#include "SerializablePair.h"
#include "SerializableMap.h"
#include "ComplexSerializable.h"
#include "CombatDataBehaviour.h"


#include "QueryContext.h"
#include "Utility.h"
#include "CombatData.h"
#include "PoiseController.h"

#include "ModSettingHandler.h"
#include "EventObject.h"

#include "AttackInfo.h"

#define __major true
#define __minor false

#define k_temp_success_for_major 5
#define k_temp_success_for_minor 2
#define k_temp_success_for_effort 1

#define k_temp_stagger_time 0//Just a callback.

namespace Crucible
{
	//move all current instances of into 1 function, the prime handle function.
	//Along with that, put flag success into the core switch as well.
	void SpecialStrikeController::IncrementStrikeResistance(StrikeResistType resist, bool force, bool reapply_pause)
	{
		if (!force && owner->poiseControl.IsPoiseBroken() == true)
			return;

		float climb = 0;

		switch (resist)
		{
		case StrikeResistType::kEvadeDefend:
			climb = k_temp_StrikeResistClimb_EvdDef;
			break;
		case StrikeResistType::kResistAttack:
			climb = k_temp_StrikeResistClimb_ResAtk;

			break;
		case StrikeResistType::kEvadeAttack:
			climb = k_temp_StrikeResistClimb_EvdAtk;

			break;
		case StrikeResistType::kResistDefend:
			climb = k_temp_StrikeResistClimb_ResDef;

			break;
		}

		

		resistMap[resist].first += climb;

		auto& resistTimer = resistMap[resist].second;

		bool wasPaused = resistTimer.isPaused();

		resistTimer.Restart(k_temp_ClimbTimer);
		
		if (wasPaused && reapply_pause)
			resistTimer.Pause();

		//Utility::DebugMessageBox(std::format("{} incremented by {}", (int)resist, climb), true);
	}

	bool SpecialStrikeController::IsInStaggerCooldown()
	{
		//currently, will not use poise (yet)

		if (owner->poiseControl.GetCurrentState() == PoiseState::Broken)
			return false;
		else if (!lastStagger)
			return false;

		//Time is allowed to be negative, that's what it looks like when it's been deserialized.

		float time = lastStagger.GetTime();

		float poisePercent = owner->GetResourcePercent(CombatResource::Poise) * k_temp_PoiseMult;

		float cooldownTime = k_temp_StaggerCooldown * (1 - k_temp_PoiseMult);

		cooldownTime += k_temp_StaggerCooldown * poisePercent;

		return time < cooldownTime;
	}

	//temp settings
	constexpr float _temp_minorAttackPercentMin = 0.05f;
	constexpr float _temp_minorAttackDamageMult = 0.5f;
	constexpr float _temp_minorAttackPoiseMult = 0.20f;

	constexpr float _temp_minorDefendPercentMin = 0.15f;
	constexpr float _temp_minorDefendDamageMult = 0.50f;
	//float minorAttackPoiseMult = 0.10f;

	//CONFUTE 
	constexpr float _temp_majorDefendReflectMult = 0.15f;//Per point of difference, it will reflect this amount of damage done.
	constexpr float _temp_majorDefendDamageMult = 0.125f;//Per point of difference, it will reduce damage done, plus the minor damage mult.

	//TO DO notes
	//With some of the minor staggers, I'm thinking of allow precision and evasion to play into them a bit.
	//Also of note. With ALL of the minor actions, make sure that do an arrangement like the major thresh - 1
	// to make them reaching their maximum powers more likely.
	//Make a stagger function, complete with a stagger mult input, should except a float, as well as an option to 
	// use perk entry points. This thing will use perk entries manually, so keep that in mind.
	// There should be another function that just takes a bool as to what level of stagger, disregarding perk entries.
	// Last, there should be the option for recoil or stagger, as for some of these they are interchangable.
	//May lock reflect to being some percent of a person's current hp, so it always does damage, but can never kill.
	
	// where stuff has to happen to the attacker. This will allow me to allow people to do things like 
	// evade traps.


	//Split these 2 fuckers in half, splitting them between attacker and defender functions, so I can leave out some of the parts
	//The only things that need it are defensive action
	//Minor Actions
	void SpecialStrikeController::HandleMinor_Flinch(QueryContext& context, HitInfo& info)
	{
		float percent = info.value;

		auto attacker = context.GetForm<RE::Actor>(queryAggressor);
		auto defender = GetActorOwner();


		float poisePercent = percent;//Damage for poise isn't shared with the general percent.


		//Works similar to glancing?
		if (attacker)
			RE::ApplyPerkEntries(uEntryPoint::kModTargetStagger, attacker, defender, &poisePercent);
		
		RE::ApplyPerkEntries(uEntryPoint::kModIncomingStagger, defender, attacker, &poisePercent);

		percent = Utility::Clamp(percent, _temp_minorAttackPercentMin, 1);
		poisePercent = Utility::Clamp(poisePercent, 0, 1);


		float currPoiseDamage = info.poiseDamage;
		float currHealthDamage = info.healthDamage;
			
		float poise_add = currPoiseDamage * poisePercent * _temp_minorAttackPoiseMult;
		float hp_add = currHealthDamage * percent * _temp_minorAttackDamageMult;

		currPoiseDamage += poise_add;
		currHealthDamage += hp_add;

		//I actually wanted to do reflect damage.

		info.poiseDamage = currPoiseDamage;
		info.healthDamage = currHealthDamage;
		//Note, this is reflect damage.
		NULL_CONDITION_TMP(info.GetPhysicalHitData())->criticalDamageMult = 0;

		//context.SetFloat(queryPostPoiseDamage, currPoiseDamage);
		//context.SetFloat(queryPostHealthDamage, currHealthDamage);
		//context.SetFloat(queryPostReflectDamage, 0);//No reflects on successful offensive actions.


		//This might become annoying but want to handle it through here.
		//A quick note, I absolutely need to buff up the stuff added to poise damage and health, flinch does funk all.
		Utility::DebugNotification(std::format("*flinch, {}, poise {}, hp {}*", percent, poise_add, hp_add), context.isPlayerInvolved);

		//If this was a bash we do this, I'm considering making it recoil only?
		if (1 != 1 && context.GetBool(queryBash) && context.GetBool(queryTimed) == true) {
			//defender->SetGraphVariableFloat("recoilMagnitude", 0.01f);
			//defender->NotifyAnimationGraph("recoilStart");
			
			//StartStagger(attacker, defender, 0, true, true);
			//StartStagger(CrowdControlType::kLargeRecoil, attacker, defender, k_temp_stagger_time);
		}
		else {
			//defender->SetGraphVariableFloat("staggerMagnitude", 0.01f);
			//defender->NotifyAnimationGraph("staggerStart");
			//StartStagger(attacker, defender, 0, false, false);
			StartStagger(CrowdControlType::kStagger, attacker, defender, k_temp_stagger_time, percent * 0.5f);
		}

		StartStaggerResist();
	}

	void SpecialStrikeController::HandleMinor_Dodge(QueryContext& context, DodgeInfo& info)
	{
		//Done in post after dodge.
		//auto defender = GetActorOwner();
	}

	void SpecialStrikeController::HandleMinor_Glancing(QueryContext& context, HitInfo& info)
	{
		float percent = info.value;

		//float currPoiseDamage = context.GetFloat(queryPostPoiseDamage);
		//context.SetFloat(queryPostPoiseDamage, 0);

		//float currPoiseDamage = info.poiseDamage;
		float currHealthDamage = info.healthDamage;

		//I actually wanted to do reflect damage.

		info.poiseDamage = 0;
		
		
		float reduce = currHealthDamage * percent * _temp_minorDefendDamageMult;
		//currPoiseDamage += currPoiseDamage * poisePercent * minorAttackPoiseMult;
		currHealthDamage -= reduce;

		//context.SetFloat(queryPostHealthDamage, currHealthDamage);
		info.healthDamage = currHealthDamage;

		Utility::DebugNotification(std::format("The attacker's rebuffed, {}, reduced by {}", percent, reduce), context.isPlayerInvolved);
	}

	void SpecialStrikeController::HandleMinor_EvadeHit(QueryContext& context, DodgeInfo& info)
	{
		float percent = info.value;

		//Should player stagger, and send small flinching stagger.
		Utility::DebugNotification(std::format("evade denied, {}", percent), context.isPlayerInvolved);

		//For this, if someone is blocking, I want them to stop blocking. Thats the event that I send.
		// either block stop or ward break, or forcibly, we turn off the isBlocking graph variable


		//Will end the action, but won't force it to go on cooldown. Though, this doesn't seek a complete interrupt.
		//It's for this bit's sake, that if a minor aggressive dodge action is used, no restrictions on damage calc.
		//owner->actionPointControl.StopAction();
	}


	void SpecialStrikeController::HandleMinor_Dodge_Aggressor(QueryContext& context, DodgeInfo& info)
	{
		float percent = info.value;

		auto attacker = GetActorOwner();
		auto defender = context.GetForm<RE::Actor>(queryDefender);

		//This should control the poise damage the dodge did. NOTE, dodges *may* do percent
		// damage. This value is minimized to 0.
		RE::ApplyPerkEntries(uEntryPoint::kModTargetStagger, attacker, defender, &percent);
		RE::ApplyPerkEntries(uEntryPoint::kModIncomingStagger, defender, attacker, &percent);

		percent = Utility::Clamp(percent, 0, 1);

		//For this, just calculate the poise damage of the source, get the square root, then double it. Maybe.


		Utility::DebugNotification(std::format("*dodge, {}*", percent), context.isPlayerInvolved);

		//Deal poise damage, trust it forces a hit failure event. This will be unique for ranged and melee
		//CALL_EVENT:OnAttackEvaded
	}

	void SpecialStrikeController::HandleMinor_Glancing_Aggressor(QueryContext& context, HitInfo& info)
	{
		//Deals with doling out damage, to poise to aggressor.

		float percent = info.value;

		auto attacker = GetActorOwner();
		auto defender = context.GetForm<RE::Actor>(queryDefender);

		//In the case of glancing, perhaps I should handle it like a shield block without the recognition of one.

		float poisePercent = percent;//Damage for poise isn't shared with the general percent.

		RE::ApplyPerkEntries(uEntryPoint::kModTargetStagger, attacker, defender, &poisePercent);
		RE::ApplyPerkEntries(uEntryPoint::kModIncomingStagger, defender, attacker, &poisePercent);

		percent = Utility::Clamp(percent, _temp_minorDefendPercentMin, 1);
		poisePercent = Utility::Clamp(poisePercent, 0, 1);


		//percent is used to determine value of damage reduction, while poisePercent
		// determines a mult for poise loss, for the enemy not the striker.


		//The requirement for this should be, the enemy isn't blocking
		if (context.GetBool(queryBash) == false && context.GetBool(queryBlocked) == false) {
			//attacker->SetGraphVariableFloat("recoilMagnitude", 1.01f);
			//attacker->NotifyAnimationGraph("recoilStart");
			if (percent < 0.5f)
				StartStagger(CrowdControlType::kRecoil, defender, attacker, k_temp_stagger_time);
			else
				StartStagger(CrowdControlType::kLargeRecoil, defender, attacker,  k_temp_stagger_time);
		}
		else {
			//attacker->SetGraphVariableFloat("staggerMagnitude", 0.01f);
			//attacker->NotifyAnimationGraph("staggerStart");
			StartStagger(CrowdControlType::kStagger, defender, attacker, k_temp_stagger_time, percent * majorFlinch);
		}

		//Trust the negation will carry through.
	}

	void SpecialStrikeController::HandleMinor_BashPunish(QueryContext& context, IAttackInfo& info)
	{
		//Loses poise, functions much like the block punish.

		//Utility::DebugMessageBox("Minor Bashing Special Strike Handle", context.isPlayerInvolved);

	}

	void SpecialStrikeController::HandleMinor_BlockPunish(QueryContext& context, IAttackInfo& info)
	{
		//Damages poise, reduces stamina for block, etc.
		//For now, this will take the health damage done, and off load it into health.

		Utility::DebugNotification("Minor Blocking Special Strike Handle", context.isPlayerInvolved);

		auto& guard_control = owner->guardControl;

		bool is_weak = guard_control.IsWeakGuard();

		static constexpr float k_temp_raise_dam_mult = 0.5f;

		//If someone's guard is broken, it takes the raise first, then action. I'm thinking action if it's taken first it serves as a sort of buffer.
		// additionally, if it starts taking from raise it will be reduced, and if it's doing the reverse progression, whatever it took from, it will
		//  take whatever percent of the reduced amount, and take that percent from the real amount.
		// Basically, if your guard isn't weakened, you get a breaking one shot protection from stamina loss.
		//A revision, while your raise stamina allows for a sort of stamina one shot protection (IE you can't go from any stamina with raise
		// to guard break immediately, you must deplete raise, then the protection is gone). Same thing goes when you're depleting raised stamina first.
		float action_stamina = guard_control.GetStamina(true);
		float raise_stamina = guard_control.GetStamina(false);
		
		float total_stamina = raise_stamina + action_stamina;

		if (action_stamina <= 0 && raise_stamina <= 0) {
			//If you run out it will just do major.
			Utility::DebugMessageBox("Your lack of block stamina has elevated a minor strike to major.", IsPlayerRef());
			return HandleMajor_BlockPunish(context, info);
		}

		RE::HitData* hit_data = NULL_CONDITION_RET(info.AsHitInfo())->GetPhysicalHitData();

		//While this literally should never happen, I'm just accounting for it for now.
		if (!hit_data) {
			Utility::DebugNotification(std::format("no hit data found? {}/{}/{}", info.AsDodgeInfo() != nullptr, info.AsHitInfo() != nullptr, info.AsHitInfo()->GetPhysicalHitData() != nullptr), IsPlayerRef());
			return;
		}

		float take_stamina = hit_data->totalDamage;// *info.value;

		float original_take = take_stamina;
		//Doing this arrangement on first so any left overs from raise fall into the second take.
		float first_take = fmin(take_stamina * (is_weak ? k_temp_raise_dam_mult : 1.f), is_weak ? raise_stamina : action_stamina);
		float second_take = fmin((take_stamina - first_take) * (is_weak ? k_temp_raise_dam_mult : 1.f), !is_weak ? raise_stamina : action_stamina);
		
		take_stamina = first_take + second_take;

		guard_control.ModStamina(first_take, !is_weak);
		guard_control.ModStamina(second_take, is_weak);

		Utility::DebugNotification(std::format("Block Punish: {}({}), result {}",
			original_take, *info.value, guard_control.GetStamina(true) + guard_control.GetStamina(false)), IsPlayerRef());

		if (raise_stamina <= 0 && take_stamina >= total_stamina){
			Utility::DebugMessageBox("Your lack of block stamina has elevated a minor strike to major.", IsPlayerRef());
			return HandleMajor_BlockPunish(context, info);
		}

		return;
		//This is not happening as it should.

		

		
		if (action_stamina < take_stamina)
		{
			guard_control.ModStamina(action_stamina, true);

			take_stamina -= action_stamina;

			if (raise_stamina < take_stamina) {
				guard_control.ModStamina(raise_stamina, false);
				Utility::DebugMessageBox("Your lack of block stamina has elevated a minor strike to major.", IsPlayerRef());
				return HandleMajor_BlockPunish(context, info);
			}
			else
			{
				guard_control.ModStamina(take_stamina, false);
			}
		}
		else {
			guard_control.ModStamina(take_stamina, true);
		}

		Utility::DebugNotification(std::format("Block Punish: {}({}), result {}",
			original_take, *info.value, guard_control.GetStamina(true) + guard_control.GetStamina(false)), IsPlayerRef());
	}



	//Major Actions
	void SpecialStrikeController::HandleMajor_Stunning(QueryContext& context, HitInfo& info)
	{
		float difference = info.value;

		auto defender = GetActorOwner();
		auto attacker = context.GetForm<RE::TESObjectREFR>(queryAggressor);

		float percent = difference * 0.4f;

		Utility::DebugMessageBox(std::format("The defender lies stunned, {}", difference), context.isPlayerInvolved);


		//context.SetFloat(queryPostReflectDamage, 0);//No reflects on this either
		NULL_CONDITION_TMP(info.GetPhysicalHitData())->criticalDamageMult = 0;

#define situational_bash false
		//The timed stagger is a bit weak for this, so I'm gonna design what CC happens around that.
		if (situational_bash && context.GetBool(queryBash) == true) {
			//I don't care about stagger direction atm.
			//defender->SetGraphVariableFloat("recoilMagnitude", fullStaggerMagnitude);
			//defender->NotifyAnimationGraph("recoilStart");
			//StartStagger(attacker, defender, 0, true, true);
			StartStagger(CrowdControlType::kLargeRecoil, attacker, defender, k_temp_stagger_time);

		}
		else {
			//I don't care about stagger direction atm.
			//defender->SetGraphVariableFloat("staggerMagnitude", fullStaggerMagnitude);
			//defender->NotifyAnimationGraph("staggerStart");
			//StartStagger(attacker, defender, 0, true, false);//Its this?
			StartStagger(CrowdControlType::kStagger, attacker, defender, k_temp_stagger_time, majorStagger + percent * majorFlinch);

		}
		
		//This event has no parameters. At least for now.
		//EventObject params;
		owner->SendEvent("OnInterrupt");
	}

	void SpecialStrikeController::HandleMajor_Stumbling(QueryContext& context, DodgeInfo& info)
	{
		//Empty for now
		//auto defender = GetActorOwner();
	}

	void SpecialStrikeController::HandleMajor_Retorting(QueryContext& context, HitInfo& info)
	{
		float difference = info.value;

		auto defender = GetActorOwner();
		
		//No poise damage to self.
		//context.SetFloat(queryPostPoiseDamage, 0);
		info.poiseDamage = 0;
		float currHealthDamage = info.healthDamage;

		//This makes it so at LEAST, it will be as effective as minor glancing.
		float reduce = currHealthDamage * ((difference * _temp_majorDefendDamageMult) + _temp_minorDefendDamageMult);

		currHealthDamage -= __max(reduce, currHealthDamage);
		
		auto* hit_data = info.GetPhysicalHitData();

		float reflect = reduce;

		NULL_CONDITION(hit_data)->criticalDamageMult += reflect;

		info.healthDamage = currHealthDamage;

		Utility::DebugMessageBox(std::format("The attacker Shakes, {}, reflected {}/{}", difference, reflect, currHealthDamage), context.isPlayerInvolved);

		//Utility::DebugMessageBox(std::format("Res Check dif \n {}", difference, owner->resist.print()), context.isPlayerInvolved);

		//context.SetFloat(queryPostHealthDamage, currHealthDamage);

	}

	void SpecialStrikeController::HandleMajor_EvadeHit(QueryContext& context, DodgeInfo& info)
	{
		float difference = info.value;

		auto defender = GetActorOwner();
		auto attacker = context.GetForm<RE::TESObjectREFR>(queryAggressor);

		Utility::DebugMessageBox(std::format("The dodger crumples, {}", difference), context.isPlayerInvolved);
		
		float percent = difference * 0.4f;
		
		//I don't care about stagger direction atm.
		//defender->SetGraphVariableFloat("staggerMagnitude", fullStaggerMagnitude);
		//defender->NotifyAnimationGraph("staggerStart");
		StartStagger(CrowdControlType::kStagger, attacker, defender, k_temp_stagger_time, majorStagger + percent * majorFlinch);


		//This event has no parameters. At least for now.
		//EventObject params;

		//While this is included in interrupt, the forced version is not.
		//owner->actionPointControl.StopAction(true);
		owner->SendEvent("OnInterrupt");//, params);
	}


	void SpecialStrikeController::HandleMajor_Stumbling_Aggressor(QueryContext& context, DodgeInfo& info)
	{
		float difference = info.value;

		auto attacker = GetActorOwner();//context->GetForm<RE::Actor>(queryAggressor);
		auto defender = context.GetForm<RE::TESObjectREFR>(queryDefender);
		//if (!attacker)
		//	return; 

		Utility::DebugMessageBox(std::format("The attacker Stumbles, {}", difference), context.isPlayerInvolved);

		


		float percent = difference * 0.4f;

		//I don't care about stagger direction atm.
		//attacker->SetGraphVariableFloat("staggerMagnitude", fullStaggerMagnitude);
		//attacker->NotifyAnimationGraph("staggerStart");
		StartStagger(CrowdControlType::kStagger, defender, attacker, k_temp_stagger_time, majorStagger + percent * majorFlinch);


		//This event has no parameters. At least for now.
		owner->SendEvent("OnInterrupt");
	}

	void SpecialStrikeController::HandleMajor_Retorting_Aggressor(QueryContext& context, HitInfo& info)
	{
		float difference = info.value;

		auto attacker = GetActorOwner();
		auto defender = context.GetForm<RE::TESObjectREFR>(queryDefender);

		float currHealthDamage = info.healthDamage;

		float health = attacker->GetActorValue(RE::ActorValue::kHealth);

#define hp_percent 0.5f

		//Not sure how to handle reflect damage if it's magic. Also, this reflect will not kill.
		// I may make it so it can't exceed a certain value.
		//THIS is reflect
		NULL_CONDITION_TMP(info.GetPhysicalHitData())->criticalDamageMult += fmin(health - (health * hp_percent), currHealthDamage * _temp_majorDefendReflectMult * difference);

		//context.SetFloat(queryPostReflectDamage, currReflectDamage);//No reflects on this.


		float percent = difference * 0.4f;

		//I don't care about stagger direction atm.
		//attacker->SetGraphVariableFloat("staggerMagnitude", fullStaggerMagnitude);
		//attacker->NotifyAnimationGraph("staggerStart");
		StartStagger(CrowdControlType::kStagger, defender, attacker, k_temp_stagger_time, majorStagger + percent * majorFlinch);

		//This event has no parameters. At least for now.
		owner->SendEvent("OnInterrupt");
	}


	void SpecialStrikeController::HandleMajor_BashPunish(QueryContext& context, IAttackInfo& info)
	{
		//Utility::DebugMessageBox("Major Bashing Special Strike Handle", context.isPlayerInvolved);
	}

	void SpecialStrikeController::HandleMajor_BlockPunish(QueryContext& context, IAttackInfo& info)
	{
		auto& guard_control = owner->guardControl;

		if (guard_control.IsWeakGuard() == true){
			guard_control.BreakGuard();
			//Utility::DebugMessageBox("Guard broken!", context.isPlayerInvolved);
		}
		else {
			guard_control.WeakenGuard();
			//Utility::DebugMessageBox("Guard weakened!", context.isPlayerInvolved);
		}
	}


	//Incorporate distance rules here, and handle that calculation in hooks maybe?
	//PUT COMBAT DATA as a parameter, and then use that as the aggressor object. Simple.
	void SpecialStrikeController::HandleStrike(StrikeCategory category, bool major, QueryContext& context, CombatData* aggressor, IAttackInfo& info)
	{
#define block_handle_defender 1
#define block_handle_aggressor 2
	
		auto defender = GetActorOwner();
		
		if (!defender)
			return;//ERERORY
		
		auto attacker = aggressor ? aggressor->GetOwner() : nullptr;

		int actor_block_exclusion = 0;//If 1 or 2 it will deal with the defender or attacker differently

		

		bool boolean = false;
		
		float maxDistance = 350.f;
		float currentDistance = context.GetFloat(queryDistance);
		
		//I will currently do nothing with this.
		//if (currentDistance >= maxDistance)
		//{
			//Too far away, Sadge
		//	return;
		//}
		
		RE::Actor* actor = nullptr;
		CombatData* data = nullptr;
		int exclusion_type = 0;

		switch (category)
		{
		case StrikeCategory::kEvadeDefend:
		case StrikeCategory::kResistDefend:
			actor = attacker;
			data = aggressor;
			exclusion_type = block_handle_aggressor;
			break;

		case StrikeCategory::kResistAttack:			
		case StrikeCategory::kEvadeAttack:
			actor = defender;
			data = owner;
			exclusion_type = block_handle_defender;
			break;

		}


		if (actor)
		{
			//Make this do recoilStop too at some point, plz k thanks
			//bool boolean = false;
			//if (actor->GetGraphVariableBool("IsStaggering", boolean) && boolean) {
				//actor->NotifyAnimationGraph("staggerStop");
				//_temp_StaggerNextTime = true;
			//}

			actor_block_exclusion = exclusion_type;

			//Actor = aggressor
			//Bashing counts, and will go first so it's not mistaken for just blocking.
			if (actor->GetGraphVariableBool("IsBashing", boolean) && boolean){//Must require that the guard is not weak
				//Do block strike handling
				Utility::DebugNotification("Bashing Special Strike Handle", context.isPlayerInvolved);

				if (major) {
					data->strikeControl.HandleMajor_BashPunish(context, info);
				}
				else {
					data->strikeControl.HandleMinor_BashPunish(context, info);
				}

				//We still experience the thingy w
			}
			//Actor = defenders
			else if (actor->GetGraphVariableBool("Isblocking", boolean) && boolean){//was IsBlocking
				//Do block strike handling
				
				if (major) {
					data->strikeControl.HandleMajor_BlockPunish(context, info);
				}
				else {
					data->strikeControl.HandleMinor_BlockPunish(context, info);
				}
			}
			else{
				actor_block_exclusion = 0;
			}
		}
		
		//I'm thinking I'm gonna have 2 functions for each action soon, so I can 
		// handle defenders stuff like poise or damage reduction, without the fact that there is no actor on the other side.
		// Allows me to handle things a lot cleaner, imo.
		//For now, this works.
		//But later on, ALL of them may require this split, and more. Since aggressor handles some finer restoration values
		// or something like that, I think it would be prudent to handle those too.

		auto* dodge_info = info.AsDodgeInfo();
		auto* hit_info = info.AsHitInfo();

		switch (category)
		{
		case StrikeCategory::kEvadeDefend:
			if (major) {
				if (aggressor && actor_block_exclusion != block_handle_aggressor)
					aggressor->strikeControl.HandleMajor_Stumbling_Aggressor(context, *dodge_info);

				HandleMajor_Stumbling(context, *dodge_info);
			}
			else {
				if (aggressor && actor_block_exclusion != block_handle_aggressor)
					HandleMinor_Dodge_Aggressor(context, *dodge_info);

				HandleMinor_Dodge(context, *dodge_info);
			}
			//owner->actionPointControl.FlagSuccess(major ? k_temp_success_for_minor : k_temp_success_for_major, ActionState::Dodge);
			
			break;
		
		case StrikeCategory::kResistDefend:
			if (major) {
				if (aggressor && actor_block_exclusion != block_handle_aggressor)
					aggressor->strikeControl.HandleMajor_Retorting_Aggressor(context, *hit_info);

				HandleMajor_Retorting(context, *hit_info);
			}
			else {
				if (aggressor && actor_block_exclusion != block_handle_aggressor)
					aggressor->strikeControl.HandleMinor_Glancing_Aggressor(context, *hit_info);

				HandleMinor_Glancing(context, *hit_info);
			}
			//owner->actionPointControl.FlagSuccess(major ? k_temp_success_for_minor : k_temp_success_for_major, ActionState::Guard);

			break;

		case StrikeCategory::kResistAttack:
			if (major) {
				if (actor_block_exclusion != block_handle_defender)
					HandleMajor_Stunning(context, *hit_info);
			}
			else {
				if (actor_block_exclusion != block_handle_defender)
					HandleMinor_Flinch(context, *hit_info);
			}
			break;

		case StrikeCategory::kEvadeAttack:
			if (major) {
				if (actor_block_exclusion != block_handle_defender)
					HandleMajor_EvadeHit(context, *dodge_info);
			}
			else {
				if (actor_block_exclusion != block_handle_defender)
					HandleMinor_EvadeHit(context, *dodge_info);
			}
			break;
		}

		//This needs to properly go on the person being staggered, and only when they stagger.
		if (data)
			data->strikeControl.FlagStagger(category, major);
		
			//if (major)
		//	IncrementStrikeResistance(category);
	}
	

	
	StrikeResult FlagsToResult(StrikeCategory flags, bool resist, bool major)
	{
		//Kinda exists cause I jumped on the wrong flag to use
		// for this shit and was too lazy to uproot everything.

		StrikeResult result;

		//Only does 2. Used to start at 1.
		for (int index = (int)StrikeCategory::kResistAttack; index <= (int)StrikeCategory::kEvadeAttack; index <<= 1)
		{
			StrikeCategory category = static_cast<StrikeCategory>(index);
			//Utility::DebugNotification(std::format("cat {} vs index {} vs flag {}", (int)category, (int)index, (int)data.majorFlags), true);
			
			switch (category & flags)
			{
#ifdef defend_is_always_off
			case StrikeCategory::kEvadeDefend:
				if (!resist)
					result |= major ? StrikeResult::majorDefend : StrikeResult::minorDefend;
				break;
			case StrikeCategory::kResistDefend:
				if (resist)
					result |= major ? StrikeResult::majorDefend : StrikeResult::minorDefend;
				break;
#endif
				//Has to also include minor if it's major.
			case StrikeCategory::kEvadeAttack:
				if (!resist)
					result |= major ? StrikeResult::allAttack : StrikeResult::minorAttack;//but maybe don't include this one?
					break;

			case StrikeCategory::kResistAttack:
				if (resist)
					result |= major ? StrikeResult::allAttack : StrikeResult::minorAttack;
					break;
			}
		}

		return result;
	}
	
	//Checks if someone is in a busy position to see if they can do a strike or not.
	bool SpecialStrikeController::CheckActionRestrictions(RE::Actor* target, StrikeResult& restrictFlags, bool is_resist, bool& absolute)
	{
		//Note, force does not work on total entries.
		// Surrendering the extras. Change of plans.
		//Absolute is mainly for dodge, which when this returns false, will put restrict flags
		// on major attack, EXCEPT if the bool is absolute, in which
		// it will just exit out.
		//Ok nevermind, but the restrict flags wont be the primary function. It should be
		// for options that mean, keep running, but don't do these things.


		//The rundown of restrictions
		// No stagger nor recoil
		// No equipping or unequipping
		// No bleedout
		// A killmove check included, just in case.
		
		auto data = CombatData::GetDataRecord(target);

		//No matter what, those in bleed out and kill move do nothing
		if (!target || !data || target->IsBleedingOut() == true || target->IsInKillMove() == true || target->IsDead() == true)
		{
			absolute = true;
			return false;
		}
		
		bool boolean = false;

		auto minor = data->strikeControl.data.minorFlags;
		auto major = data->strikeControl.data.majorFlags;

		bool no_minor = StrikeCategory::kNone == minor;
		bool no_major = StrikeCategory::kNone == major;

		bool foreign_stagger = no_minor && no_major;

		bool stagger = target->GetGraphVariableBool("IsStaggering", boolean) && boolean;
		//if not us, treat it like normal. This shit generally happens on hitting shields (I mean no now, but you know).
		bool recoil = target->GetGraphVariableBool("IsRecoiling", boolean) && boolean && !foreign_stagger;

		if (stagger || recoil)
		{
//#ifdef  hold_demand
			restrictFlags |= StrikeResult::allDefend;//While in these, one cannot defend

			//Remember, this is about controlling what they can do, but also what can be done to them. 
			
			if (foreign_stagger){
				//If this isn't our stagger, treat it like flinch, only stun is allowed left, not even defend.
				restrictFlags |= is_resist ? StrikeResult::exceptMajorAttack : StrikeResult::total;
				//Utility::DebugMessageBox("foreign stagger", false);
			}
			else{
				//Utility::DebugMessageBox(std::format("{} in house stagger", (int)major), false);

				restrictFlags |= no_minor ? StrikeResult::none : FlagsToResult(minor, is_resist, false);
				restrictFlags |= no_major ? StrikeResult::none : FlagsToResult(major, is_resist, true);
			}
			//return false;
//#endif //  hold_demand
		}


		if (target->GetGraphVariableBool("IsEquipping", boolean) && boolean){
			restrictFlags |= StrikeResult::exceptMinorDefend;
		}
		else if (target->GetGraphVariableBool("IsUnequipping", boolean) && boolean){
			restrictFlags |= StrikeResult::exceptMinorDefend;
		}

		if (restrictFlags == StrikeResult::total) {
			absolute = true;//If the restrict flag is full, just skip it all
			return false;
		
		}
		return true;
	}
	bool SpecialStrikeController::CheckActionRestrictions(RE::Actor* target, StrikeResult& restrictFlags, bool resist)
	{
		bool dummy = false;
		return CheckActionRestrictions(target, restrictFlags, resist, dummy);
	}

	
	

	//Both main functions need a null check on getting the port list, and attackerdata
	// They additionally need something that says while the strike carried out it can't hit, like for ranged fair
	// Also, divide on all minor actions please.
	StrikeResult SpecialStrikeController::ProcessDodge(QueryContext& context, RE::TESObjectREFR* attacker, StrikeResult restrictFlags, DodgeInfo& dodge_info)
	{
		auto defender = GetActorOwner();

		//I will not null check these, if it crashes I wanna see if it fails.
		StrikeSetting* offense = ModSettingHandler::GetSetting<StrikeSetting>(StrikeThreshold::offenseEvade);
		StrikeSetting* defense = ModSettingHandler::GetSetting<StrikeSetting>(StrikeThreshold::defenseEvade);

		//Maybe no attacker is allowed...
		if (!defender)// || !attacker)
			return StrikeResult::none;

		bool noPassive = false;

		if (CheckActionRestrictions(defender, restrictFlags, false, noPassive) == false) {
			if (owner->actionPointControl.GetActionPoint() == ActionPoint::autoDodge && !noPassive)
			{
				//^ If viable for auto dodge. Its the job of the function to set this up.
				//I can check this by getting if the go to location is what we want it to be
				
				//This area denotes stagger or something of the like. During this,
				// regardless if what was used was a power action or not, the auto dodge
				// is unable to stumble anyone.
				restrictFlags |= StrikeResult::exceptMinorDefend;
			}
			else{
				return StrikeResult::none;
			}

		}
		//A few more notes on auto dodges, they effectively do not allow you to
		// take offensive strikes, but successfully getting hit during it will
		// put the ability on cooldown. This will only happen if it's viable.

		//Also, since I will need to use evade to tell if this happens, this needs
		// to get the query for evade to see if it should happen.

		

		context.SetForm(queryAgressor, attacker);
		context.SetForm(queryDefender, defender);


		CombatValueOwner* attackValueOwner = CombatData::GetValueOwner(attacker->GetFormID());
		CombatData* defenderData = owner;


		//Account for objects when you can.
		if (!attackValueOwner || !defenderData)
			return StrikeResult::none;

		CombatData* attackerData = attackValueOwner->AsCombatData();

		if (attackerData)
			attackerData->CheckSource(context);//Fixes null sources to fists.

		//Query if it exists already.
		float precision = attackValueOwner->GetPrecision(context, defenderData->GetPortList());
		
		context.SetFloat(queryPostPrecision, precision);

		
		float evade = 0;
		//This value will be gotten for a dodge, and may be submitted before this, if so, it should be pulled.
		if (context.HasValue(queryPostEvade) == true) {
			evade = context.GetFloat(queryPostEvade);
		}
		else {
			evade = defenderData->GetEvade(context, attackValueOwner->GetPortList());			
		}
		evade = fmax(defenderData->actionPointControl.GetStoredValue(), evade);
		context.SetFloat(queryPostEvade, evade);

		

		float difference = precision - evade;

		//Utility::DebugNotification(std::format("evade {}, precision {}, dif {}", evade, precision, difference), true);

		bool attacker_is_actor = attackerData != nullptr;


		//This will no longer handle the colliding, and instead that will be delegated to the hook who calls.
		// Or perhaps, dodge check will still exist, but it's function will be handling the collision stuff and returning
		// a bool if this thing was successful or not.



		
		float evadeModMult = 1.f;//0.85f;//Fixed for major calculation.


		float add_evade_off = GetStrikeResist(StrikeResistType::kEvadeAttack);
		float add_evade_def = attacker_is_actor ? attackerData->strikeControl.GetStrikeResist(StrikeResistType::kEvadeDefend) : 0;


		float off_MajorThreshold = offense->majorThreshold + add_evade_off;
		float def_MajorThreshold = defense->majorThreshold + add_evade_def;


		//for this we check the enemy first, to see if either of it's actions happen. THEN we do our bit
		// Also a note, major defend can only happen if minor defend happens.
	

		StrikeResult result = StrikeResult::none;
		float value = 0;

		//Offend
		if (difference >= off_MajorThreshold && (restrictFlags & StrikeResult::majorAttack) == StrikeResult::none)
		{
			value = fmax(difference - off_MajorThreshold, 0);
			result = StrikeResult::majorAttack;

			dodge_info.result = result;
			dodge_info.value = value;
			HandleStrike(StrikeCategory::kEvadeAttack, __major, context, attackerData, dodge_info);
			//HandleMajor_EvadeHit(context, fmax(difference - off_MajorThreshold, 0));
		}
		//This will explicitly only do if you are over but not equal.
		else if (difference > offense->minorThreshold && (restrictFlags & StrikeResult::minorAttack) == StrikeResult::none)
		{
			value = fmax(difference - off_MajorThreshold, 0);
			result = StrikeResult::minorAttack;

			dodge_info.result = result;
			dodge_info.value = value;
			HandleStrike(StrikeCategory::kEvadeAttack, __minor, context, attackerData, dodge_info);
			//HandleMinor_EvadeHit(context, fmax(difference / off_MajorThreshold, 0));
		}
		else if (-difference >= defense->minorThreshold && (restrictFlags & StrikeResult::minorDefend) == StrikeResult::none)
		{
			float stagger = attackValueOwner->GetStagger(context, defenderData->GetPortList());
			context.SetFloat(queryPostStagger, stagger);

			float majorCheck = def_MajorThreshold - evade * evadeModMult;
			float altDifference = (stagger - precision) - majorCheck;
			
			//Or query flagged non-ranged, also thinking of having query get this, for effective range usage.
			if ((restrictFlags & StrikeResult::majorDefend) == StrikeResult::none && 
				stagger - precision >= majorCheck)
			{
				value = altDifference;
				result = StrikeResult::majorDefend;
				
				dodge_info.result = result;
				dodge_info.value = value;
				HandleStrike(StrikeCategory::kEvadeDefend, __major, context, attackerData, dodge_info);
			}
			else
			{
				value = fmax((stagger - precision) / majorCheck, 0);
				result = StrikeResult::minorDefend;
				
				dodge_info.result = result;
				dodge_info.value = value;
				HandleStrike(StrikeCategory::kEvadeDefend, __minor, context, attackerData, dodge_info);
			}
		}
		
		dodge_info.Finalize();

		return result;
	}

	//Considering making this function be able to go through if a target is staggering, thus making 
	// stagger able to be followed up by a special strike IF it's a major one.
	StrikeResult SpecialStrikeController::ProcessHit(QueryContext& context, RE::TESObjectREFR* attacker, StrikeResult restrictFlags, HitInfo& hit_info)
	{
		//Source will need to be fixed as soon as possible. For now, you send it when setting up queries. 
		// if source doesn't exist and there's combat data of the attacker, use the check attack form function.

		//There is no need to set the attacker and defender, that is what this will do here.

		//These become mod settings btw.
		// They are seperated btween stumbles, staggers, glances, and catches. Each with minor and major actions,
		// and maybe a value to have inclusivity and exclusivity.
		//float minorAttackThreshold = 0;//This is basic stagger, exclusive.
		//float majorAttackThreshold = 5;//This is stunning blow, though I want this to include. a difference of 5.
		//float minorDefendThreshold = 4;//This should include as well.
		//float majorDefendThreshold = 6;// This should include

		auto defender = GetActorOwner();

		//I will not null check these, if it crashes I wanna see if it fails.
		StrikeSetting* offense = ModSettingHandler::GetSetting<StrikeSetting>(StrikeThreshold::offenseHit);
		StrikeSetting* defense = ModSettingHandler::GetSetting<StrikeSetting>(StrikeThreshold::defenseHit);

		//Maybe no attacker is allowed...
		if (!defender)// || !attacker)
			return StrikeResult::none;

		auto before_flags = restrictFlags;

		if (CheckActionRestrictions(defender, restrictFlags, true) == false)
		//if (defender->IsInKillMove() == true || defender->IsBleedingOut() == true)
			return StrikeResult::none;


		context.SetForm(queryAgressor, attacker);
		context.SetForm(queryDefender, defender);


		CombatValueOwner* attackValueOwner = CombatData::GetValueOwner(attacker->GetFormID());
		CombatData* defenderData = owner;


		//Account for objects when you can.
		if (!attackValueOwner || !defenderData)
			return StrikeResult::none;

		CombatData* attackerData = attackValueOwner->AsCombatData();

		if (attackerData)
			attackerData->CheckSource(context);//Fixes null sources to fists.

		//at a later point, this will need to get it's actor value
		float stagger = attackValueOwner->GetStagger(context, defenderData->GetPortList());
		float resist = fmax(defenderData->GetResist(context, attackValueOwner->GetPortList()), defenderData->actionPointControl.GetStoredValue());

		context.SetFloat(queryPostStagger, stagger);
		context.SetFloat(queryPostResist, resist);

		//Utility::DebugMessageBox(std::format("Stagger: {}, Resist: {}, before flags: {}, resist flags: {}", stagger, resist, (unsigned int)before_flags, (unsigned int)restrictFlags), context.isPlayerInvolved);

		float difference = stagger - resist;

		bool attacker_is_actor = attackerData != nullptr;

		//Guard point calc. Note, at a later point it will have
		if (defenderData->actionPointControl.IsGuarding() == true)
		{
			Utility::DebugMessageBox("Guard Point Enabled", IsPlayerRef());
			if (difference < 0) {
				difference *= 2;
			}
		}

		//Would like a better way to evaluate this.

		//I think I'll make evaluate functions for this then loop them.
		// A last thing that I wanted to note. Defender actions cannot (or rather shouldn't happen)
		// When someone is staggered.

		//port plz
		//int add_evade_off = GetStrikeResist(StrikeResistType::kEvadeAttack);
		//int add_evade_def = GetStrikeResist(StrikeResistType::kEvadeDefend);
		float add_resist_off = GetStrikeResist(StrikeResistType::kResistAttack);
		float add_resist_def = attacker_is_actor ? attackerData->strikeControl.GetStrikeResist(StrikeResistType::kResistDefend) : 0;
		
		float off_MajorThreshold = offense->majorThreshold + add_resist_off;
		float def_MajorThreshold = defense->majorThreshold + add_resist_def;

		//Utility::DebugMessageBox(std::format("resulting resist {}, \n {}", resist, owner->resist.print()), context.isPlayerInvolved);


		StrikeResult result = StrikeResult::none;
		float value = 0;


		//Offend
		if (difference >= off_MajorThreshold && (restrictFlags & StrikeResult::majorAttack) == StrikeResult::none)
		{
			value = difference - off_MajorThreshold;
			result = StrikeResult::majorAttack;

			//Utility::DebugMessageBox(std::format("Stagger: {}, Resist: {}, dif: {}, thresh: {}, res: {}", stagger, resist, difference, off_MajorThreshold, value), context.isPlayerInvolved);

			hit_info.result = result;
			hit_info.value = value;
			HandleStrike(StrikeCategory::kResistAttack, __major, context, attackerData, hit_info);
			//HandleMajor_Stunning(context, difference - off_MajorThreshold);
		}
		//Don't forget the time check.
		else if (!IsInStaggerCooldown() && difference > offense->minorThreshold && (restrictFlags & StrikeResult::minorAttack) == StrikeResult::none)
		{
			value = fmax(difference / off_MajorThreshold, 0);
			result = StrikeResult::minorAttack;

			hit_info.result = result;
			hit_info.value = value;
			HandleStrike(StrikeCategory::kResistAttack, __minor, context, attackerData, hit_info);
			//HandleMinor_Flinch(context, fmax(difference / off_MajorThreshold, 0));
		}
		//Defend, happens to the attacker.
		else if (-difference >= def_MajorThreshold && (restrictFlags & StrikeResult::majorDefend) == StrikeResult::none)
		{
			value = -difference - def_MajorThreshold;
			result = StrikeResult::majorDefend;

			hit_info.result = result;
			hit_info.value = value;
			HandleStrike(StrikeCategory::kResistDefend, __major, context, attackerData, hit_info);
			//attackerData->strikeControl.HandleMajor_Retorting(context, -difference - def_MajorThreshold);
		}
		else if (-difference >= defense->minorThreshold && (restrictFlags & StrikeResult::minorDefend) == StrikeResult::none)
		{
			//This is your poise percent flipped around 1
			float minorPercent = 1 - defenderData->GetResourcePercent(CombatResource::Poise);

			//A major question is should this focus on the resist levels climbing, or should it just focus on the regular offsets.
			// It should focus on all. A climbing resistance to staggers would allow for a window for minor staggers after all.
			if (minorPercent == 1 || minorPercent != 0 && -difference < defense->minorThreshold + (def_MajorThreshold - defense->minorThreshold) * minorPercent)
			{
				//I wish to keep this specifically, mainly because I want a sound similar to glancing to play when you do one of these, but it's duller.
				RE::DebugMessageBox(std::format("your weak poise prevents glancing blows.\nPerc:{}, dif:{} thresholds:{}/{}, result:{}",
					minorPercent, -difference, def_MajorThreshold, defense->minorThreshold,
					defense->minorThreshold + (def_MajorThreshold - defense->minorThreshold) * minorPercent));

				goto skip_state;
			}

			value = fmax(-difference / def_MajorThreshold, 0);
			result = StrikeResult::minorDefend;

			hit_info.result = result;
			hit_info.value = value;
			HandleStrike(StrikeCategory::kResistDefend, __minor, context, attackerData, hit_info);
			//attackerData->strikeControl.HandleMinor_Glancing(context, fmax(-difference / def_MajorThreshold, 0));

		}

		skip_state:

		//owner->actionPointControl.FlagSuccess(k_temp_success_for_effort, ActionState::Guard);

		//hit_info.SetResults(result, value);

		hit_info.Finalize();

		return result;
	}


	void SpecialStrikeController::Initialize()
	{
		//Note, a lot of this stuff can be done in it's implicit constructor. No need to
		// fill initiali
		
//#define testing_member_function
#ifdef testing_member_function


		auto& evade_defend = resistMap[StrikeResistType::kEvadeDefend];
		auto& resist_attack = resistMap[StrikeResistType::kResistAttack];
		auto& evade_attack = resistMap[StrikeResistType::kEvadeAttack];
		auto& resist_defend = resistMap[StrikeResistType::kResistDefend];

		auto _id = GetID();

		//I would much rather a better way, but for now, this ugly 
		// awfulness will have to do. Please avert thine eyes.
		evade_defend.second = Timer::Local();
		resist_attack.second = Timer::Local();
		evade_attack.second = Timer::Local();
		resist_defend.second = Timer::Local();

		resist_attack.second.SetID(_id);
		evade_attack.second.SetID(_id);
		evade_defend.second.SetID(_id);
		resist_defend.second.SetID(_id);

		//Can be replaced with a single function now, please do so at your leisure
		auto decrement_evadeDefend = [this](Timer* timer_source, float total_time)
		{
			auto& pair = resistMap[StrikeResistType::kEvadeDefend];

			pair.first -= pair.first < k_temp_StrikeResistClimb_EvdDef ? pair.first : k_temp_StrikeResistClimb_EvdDef;
			if (pair.first != 0)
			{
				pair.second.Start(k_temp_ClimbTimer);
			}

			Utility::DebugNotification("M1", show);
		};

		auto decrement_resistAttack = [this](Timer* timer_source, float total_time)
		{
			auto& pair = resistMap[StrikeResistType::kResistAttack];

			pair.first -= pair.first < k_temp_StrikeResistClimb_ResAtk ? pair.first : k_temp_StrikeResistClimb_ResAtk;
			if (pair.first != 0) {
				pair.second.Start(k_temp_ClimbTimer);
			}
			Utility::DebugNotification("M2", show);
		};

		auto decrement_evadeAttack = [this](Timer* timer_source, float total_time)
		{
			auto& pair = resistMap[StrikeResistType::kEvadeAttack];

			pair.first -= pair.first < k_temp_StrikeResistClimb_EvdAtk ? pair.first : k_temp_StrikeResistClimb_EvdAtk;
			if (pair.first != 0) {
				pair.second.Start(k_temp_ClimbTimer);
			}
			Utility::DebugNotification("M3", show);
		};

		auto decrement_resistDefend = [this](Timer* timer_source, float total_time)
		{
			auto& pair = resistMap[StrikeResistType::kResistDefend];

			pair.first -= pair.first < k_temp_StrikeResistClimb_ResDef ? pair.first : k_temp_StrikeResistClimb_ResDef;
			if (pair.first != 0) {
				pair.second.Start(k_temp_ClimbTimer);
			}
			Utility::DebugNotification("M4", show);
		};


		evade_defend.second.SetTimerFinishCallback(decrement_evadeDefend);
		resist_attack.second.SetTimerFinishCallback(decrement_resistAttack);
		evade_attack.second.SetTimerFinishCallback(decrement_evadeAttack);
		resist_defend.second.SetTimerFinishCallback(decrement_resistDefend);


#else
		//Has issues.
		auto _id = GetID();
		
		for (int i = 1; i <= (int)StrikeResistType::kEvadeAttack; i <<= 1)
		{
			auto& timer = resistMap[(StrikeResistType)i].second;
			timer = Timer::Local();
			timer.SetID(_id);
			timer.AddCallback(this);
		}

#endif

		//Want to do THIS ^ first, and see if it still gets saved.
		
		//It is this that broke the movement stuff. Everything else works fine.
		//The starts used to be output graph events (erroneously), so maybe it is entirely these.
		CreateMemberEvent("OnAction", &SpecialStrikeController::OnAction);

		CreateMemberEvent("OnGraphInputEvent_staggerStart", &SpecialStrikeController::OnStaggerStart);
		CreateMemberEvent("OnGraphInputEvent_recoilStart", &SpecialStrikeController::OnStaggerStart);//Unsure if I even want this anymore.
		CreateMemberEvent("OnGraphInputEvent_recoilLargeStart", &SpecialStrikeController::OnStaggerStart);

		CreateMemberEvent("OnGraphOutputEvent_staggerStop", &SpecialStrikeController::OnStaggerStop);
		CreateMemberEvent("OnGraphOutputEvent_recoilStop", &SpecialStrikeController::OnStaggerStop);
	}
}