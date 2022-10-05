#include "Hooks.h"
#include "Utility.h"
#include "CombatData.h"
#include "ItemStatHandler.h"
#include "TimeManager.h"
#include "AttackStatController.h"
#include "CollisionIgnoreHandler.h"
#include <cmath>

#include "QueryContext.h"

#include "TemporaryFlag.h"

#include "AttackInfo.h"
//#include "AttackInfo.hpp"


namespace Crucible
{
	//purpose of this map is to have a thead safe way to statically store an object.
	//template <class Object>
	//ThreadMap



	std::int32_t CrucibleHook::BlockHitHook::thunk(RE::Actor* actor)
	{
		auto combat_data = CombatData::GetDataRecord(actor);

		if (!combat_data || combat_data->guardControl.ShouldBlockHitIdle() == true)
		{
			return func(actor);
		}

		//Block hit ignored.
		return 0;

		//Utility::DebugNotification("Block hit");
		//return 0;
	}
	


	RE::TESIdleForm* CrucibleHook::AllowIdleHook::thunk(RE::TESIdleForm* a_this, RE::ConditionCheckParams* params, void* unk3)
	{
		//This needs refining, and much of the same treatment that notify graph was going to get.
		//First thing I would like, is to seperate this into 3 functions. Not as many as notify graph was to get,
		// but enough given the altered purpose of the hook.
		// First, AllowIdle_name. Then AllowIdle all.
		// THEN, allow attack idle. Find a way to shorten the find attackdata btw, it's in there somewhere.
		// The main being, this would be a way to prevent all of these idles, however, it will not be a way to supplant them
		// with a completely different idle. There is normally, nothing wrong with doing so. But for stagger, recoil, and block
		// I've seen that special things are associated with those.
		//Wait a moment, lastly, there's a kill move one I would like to do Unspecific again.

		auto* result = func(a_this, params, unk3);

		/*
		std::string namee{}; //(result && result->animEventName.data() && !result->animEventName.empty()) ? result->animEventName.data() : "No Result";
		namee = (a_this && a_this->animEventName.data() && !a_this->animEventName.empty()) ? a_this->animEventName.data() : "No Name";
		RE::FormID id = a_this ? a_this->formID : 0;


		auto* parent = a_this->parentIdle;
		std::string namee2{}; //(result && result->animEventName.data() && !result->animEventName.empty()) ? result->animEventName.data() : "No Result";
		namee2 = (parent && parent->animEventName.data() && !parent->animEventName.empty()) ? parent->animEventName.data() : "No Name";
		RE::FormID id2 = parent ? parent->formID : 0;
		

		

		logger::info("this ({}, {:08X})", namee, id);
		logger::info("parent ({}, {:08X})", namee2, id2);

		if (parent)
		{
			auto* parent_parent = parent->parentIdle;
			std::string namee3{}; //(result && result->animEventName.data() && !result->animEventName.empty()) ? result->animEventName.data() : "No Result";
			namee3 = (parent_parent && parent_parent->animEventName.data() && !parent_parent->animEventName.empty()) ? parent_parent->animEventName.data() : "No Name";
			RE::FormID id3 = parent_parent ? parent_parent->formID : 0;
			logger::info("parent parent ({}, {:08X})", namee3, id3);

		}
		else
		{
			logger::info("parent parent doesn't exist");
		}
		//*/

		//I don't know how else I can prevent this from not working.
		if (!result || !result->animEventName.data() || result->animEventName.empty() == true) {// ||  result == a_this
			//logger::info("first return ({}, {:08X})", namee, id/*, !result, result == a_this, !result->animEventName.data(), result->animEventName.empty() */);//");//
			//logger::info("follow return ({}, {:08X})", namee2, a_this->formID/*, !result, result == a_this, !result->animEventName.data(), result->animEventName.empty() */);//");//
			//logger::info("first return3 ({})", id );

			return result;
		}
		


		RE::Actor* action_ref = params->actionRef->As<RE::Actor>();
		RE::TESObjectREFR* target_ref = params->targetRef;


		if (!action_ref) {
			//logger::info("second return");//({}), no action ref", namee);
			return result;
		}

		auto* combat_data = CombatData::GetDataRecord(action_ref);

		if (!combat_data) {
			//logger::info("third return");//({}), no data for {}", namee, action_ref->GetName());
			return result;
		}
		//When filled, a task will be made for this to notify next frame.
		//Also an easy way to get the name
		std::string notify_event = result->animEventName.data();
		//std::string full_notify_event = notify_event;
		//auto predicate = [](std::string& specific_name, EventControl& control)
		//{
		//	control = control == EventControl::kStop ? EventControl::kGroupStopAll : control;
		//};
		//std::string t = "ddd";
		//RE::DebugMessageBox(t + "4");
		//if (combat_data->SendGroupedEvent<predicate>({"OnQueryIdle", "OnCombatIdle"}, {result->animEventName.data()}, result, action_ref, target_ref) == EventControl::kGroupStopAll)
		//	return a_this;

		//For all of this, I would sorely like a way to make this functionally allow me to send in me own idle, but doing so 
		// would require me to check run it's condition. Actually, I think I'll do just that. I've got the function after all.

		bool is_killmove = false;


		if (result->animEventName.length() >= 7 && strncmp(result->animEventName.data(), "pa_Kill", 7) == 0) {
			//The front part of the animation is cut off to to make it more presentable
			notify_event = notify_event.substr(3);
			//For now, I'm preventing kill moves form happening to the player. Just in general.
			if (combat_data->SendEvent("OnKillMoveIdle_" + notify_event, result, action_ref, target_ref) == EventControl::kStop ||
				combat_data->SendEvent("OnKillMoveIdle", result, action_ref, target_ref) == EventControl::kStop || (target_ref && target_ref->IsPlayerRef())){
				logger::info("Stopped a kill move");
				return nullptr;
			}

			is_killmove = true;
		}


		if (action_ref->IsPlayerRef() && !is_killmove)
		{
			//This is only really accurate when used on the player, so if its the player and no killmove is happening, possible replacement will be used.
			//The attack data seen here is something I have no intention of changing. So I want to test out const on this and if it will let me do this.
			DisallowReference<RE::BGSAttackData*> attackData = Utility::GetAttackData(action_ref, result->animEventName);

			if (attackData)
			{
				//Some how, this never happens if someone is not the player. But, it's important to note the actual part of the event, this, happens.
				if (combat_data->SendEvent("OnPlayerCombatIdle_" + notify_event, result, target_ref, attackData) == EventControl::kStop ||
					combat_data->SendEvent("OnPlayerCombatIdle", result, target_ref, attackData) == EventControl::kStop) {
					return nullptr;
				}
			}
		}
		//Note to this, it should alter what happens if the string changes, but it should
		if (combat_data->SendEvent("OnQueryIdle_" + notify_event, result, action_ref, target_ref) == EventControl::kStop || 
			combat_data->SendEvent("OnQueryIdle", result, action_ref, target_ref) == EventControl::kStop) {
			//While this can't provide all the functionalit of launching an action, it can get the visuals you desire.
			/*
			auto task = SKSE::GetTaskInterface();
			if (result->animEventName.data() != notify_event && task) {

				auto notify_task = [action_ref, notify_event]()
				{
					if (action_ref)
						action_ref->NotifyAnimationGraph(notify_event.c_str());
				};

				task->AddTask(notify_task);
			}
			//*/
			//Was returning a_this. That happens on successes, this weren't no success
			return nullptr;
		}
		
		return result;


		//if (combat_data->SendEvent("AllowIdle", result, action_ref, target_ref) == EventControl::kStop)
		//	return a_this;

		
	}
	//There is no third param, change
	inline void ActorUpdate(RE::Actor* a_this, float delta, void* a3)
	{

		//This shit is likely busted for actors. fair notes.
		//std::int64_t result = a_this->IsPlayerRef() ? _PlayerUpdate(a_this, delta, a3) : _CharacterUpdate(a_this, delta, a3);

		//One of these, is causing a freeze, perhaps with something new we introduced.
		// with this, I would like to ask,

		if (!a_this || a_this->formType != RE::FormType::ActorCharacter)
			return;


		auto ui = RE::UI::GetSingleton();
		auto main = RE::Main::GetSingleton();

		//REDO THIS, I want them to still update until further notice IF they already have a data record, but should they not,
		// and the are things like dead, no ai process, not in middle high, we don't want them.
		if (ui->numPausesGame || main->freezeTime || !main->gameActive) {
			return;
		}

		//We don't want to flush information on dead people before they've become unloaded, so we'll still do stuff while their dead.
		// The data then is the thing that excludes this, as even if dead and no longer processed, this needs to pass to the data to notify it
		// of that fact.
		bool force_get = !a_this->IsDead() && a_this->currentProcess && a_this->currentProcess->InMiddleHighProcess();

		//logger::info("{}/{:08X}", a_this->GetName(), a_this->formID);
		//This force get, not the functions used, but it being true is the thing that makes this crash?
		CombatData* data = CombatData::GetDataRecord(a_this, force_get);




		if (!data) {
			return;
		}

		if (!delta)
			delta = *g_deltaTime;

		data->Update(delta, a_this);

		return;
	}


	std::int64_t CrucibleHook::CharacterUpdate(RE::Actor* a_this, float delta, void* a3)
	{
		std::int64_t result = _CharacterUpdate(a_this, delta, a3);
		//logger::info("EXTRA 2 {}/{:08X}", a_this->GetName(), a_this->formID);
		ActorUpdate(a_this, delta, a3);
		return result;
	}


	std::int64_t CrucibleHook::PlayerUpdate(RE::Actor* a_this, float delta, void* a3)
	{
		std::int64_t result = _PlayerUpdate(a_this, delta, a3);

		RE::Actor* new_actor = RE::TESForm::LookupByID<RE::Actor>(a_this->formID);

		//logger::info("EXTRA {}/{:08X}", new_actor->GetName(), new_actor->formID);
		ActorUpdate(new_actor, delta, a3);
		return result;
	}



	void CrucibleHook::MainUpdate(RE::Main* a_this, float a_val)
	{
		//I wish to undo this over time.

		_MainUpdate(a_this, a_val);

		auto a_ui = RE::UI::GetSingleton();
			
		//if (!a_ui->numPausesGame && !a_this->freezeTime) TaskTimerHandler::GetSingleton()->Update();
		if (!a_ui->numPausesGame && !a_this->freezeTime && a_this->gameActive) {
			TimeManager::UpdateDelta(*g_deltaTime);
			TimeManager::UpdateTimer(0, *g_deltaTime);

		}
		//Will use update delta eventually probably. Global WILL NOT use timescale however, if I want timescaled time, I think 
		// I'll just make a setting for it, or make an initializer, or a bool that declares it time scaled.

	}
	
	float CrucibleHook::GetAttackStaminaCost(RE::ActorValueOwner* a_this, RE::BGSAttackData* attack_data)
	{
		//Via this function I can control how all attacks cost, even reroute them.

		auto result = o__GetAttackStaminaCost(a_this, attack_data);
		
		auto abs_res = abs(result);
		auto actor = skyrim_cast<RE::Actor*>(a_this);//There's a function for this.

		float stamina = actor->GetActorValue(RE::ActorValue::kStamina);
		

		if (actor) {
			CombatData* data = CombatData::GetDataRecord(actor);

			if (data)
			{
				if (result > 0)
				{
					if (attack_data->data.flags.any(AttackDataFlags::kBashAttack) == true)
					{
						//I don't really want to do this, I just want a seperation of value.
						abs_res -= data->guardControl.GetBashStamina();
						result = abs_res * (result / abs(result));

						data->guardControl.TryTempStamina(abs_res > stamina ? stamina : abs_res);
					}
					else
					{
						if (attack_data->data.flags.any(AttackDataFlags::kPowerAttack) == true)
						{
							//It will obviously be a power attack, this is just a formality at this point.
							RE::TESForm* attackForm = Utility::GetAttackingWeapon(actor);

							attackForm = data->CheckAttackForm(attackForm);

							//This ain't handled right I don't think? Its not using the actual data's reduction value?
							float powerCostReduction = ItemStatHandler::GetStatValue(attackForm, powerAttackCostRate);

							powerCostReduction *= data->meleeCombo.GetHits();

							//Want some proper settings or constants for this.
							//This has no clamping.
							abs_res = Utility::Clamp(abs_res - powerCostReduction, 0, abs_res);//, abs_res * 0.5f, abs_res * 2);
							result = abs_res * (result / abs(result));
							//if (actor->IsPlayerRef() == true)
							//	Utility::DebugMessageBox(std::format("{} price", abs_res));

						}




						//data->meleeCombo.usedResource += abs_res;
						data->meleeCombo.ModifyValue(usedStamina, abs_res > stamina ? stamina : abs_res);
					}
					
					//Utility::DebugMessageBox(std::format("{} price", result), data->IsPlayerRef());
				}
			}
		}

		return result;
	}

	float CrucibleHook::GetBlockStaminaCost(RE::HitData& a_hitData)
	{
		float result = o__GetBlockStaminaCost(a_hitData);



		if (!a_hitData.target)
			return result;
		
		auto data = CombatData::GetDataRecord(a_hitData.target.get()->GetFormID());

		if (!data)
			return result;
		
		//a_hitData.stagger = 0;

		//Utility::DebugMessageBox(std::format("{} stamina on the block", result), data->IsPlayerRef());
		data->guardControl.TryTempStamina(abs(result));

		return result;
	}


	//Pull function master TODO list
	// ProcessRestrictFlags
	//  should take the query and set them accordingly. Should tell between dodge or hit, but should be friendly to both

	//struct ActivityFlag



	//Find some way to make this apply across both, and have an return object
	// I can use to manipulate the results on the other side. Like a hit data copy or something
	// maybe the context would be enough, if it actually escaped this function.
	//Also, seperate the functionality into different functions, that I can then source for both
	// dodge calc and hit calc
	void CrucibleHook::ProcessHitEvent(RE::Actor* target, RE::HitData& a_hitData)
	{
#define __return  return _ProcessHitEvent(target, a_hitData)
#define a_hitData_reflectedDamage  a_hitData.criticalDamageMult//This shit is named wrong, too lazy to rename.
#define a_hitData_pushBack  a_hitData.reflectedDamage//This shit too, is named incorrectly
		
		auto attacker = a_hitData.aggressor.get().get();

		//Should an explosion do no damage, nor have any enchant, we are gonna have to nope the fuck out.

		//if (source && source->GetFormType() == RE::FormType::Explosion)
			//Utility::DebugNotification(std::format("name: {} , formID {}", source->GetName(), source->GetFormID()).c_str());
	
		if (!attacker || !target || attacker == target)
			__return;	//Needs to be able to function when there is no attacker.


		CombatData* targetData = CombatData::GetDataRecord(target->GetFormID());

		if (!targetData)
			__return;


		


		//We should want to do this because it clears first,but I want to make an object that handles this
		// automatically.
		//auto ignoreStrike = targetData->temp_IgnoreHitStrike;//for this just get variable
		
		//targetData->temp_IgnoreHitStrike = false;
		//if (ignoreStrike)
		//	__return;


		//Kill move hits, nor hits on dead people ever count.
		if (target->IsInKillMove() == true || target->IsDead() == true)
			__return;


		//Around here make the attacker combat value dealer.
		CombatValueOwner* attackValueOwner = CombatData::GetValueOwner(attacker->GetFormID());
		CombatData* attackerData = attackValueOwner->AsCombatData();



		if (!attackValueOwner)//This will be removed in time.
			__return;


		//I think I actually do want this const, so events aren't allowed to fuck with it.
		//DisallowReference<DodgeInfo*> dodge_info = targetData->strikeControl.flag_AttackInfo ? targetData->strikeControl.flag_AttackInfo->AsDodgeInfo() : nullptr;
		DodgeInfo* dodge_info = targetData->strikeControl.flag_AttackInfo ? targetData->strikeControl.flag_AttackInfo->AsDodgeInfo() : nullptr;
		//DodgeInfo* dodge_info = NULL_CONDITION_RET(targetData->strikeControl.flag_AttackInfo)->AsDodgeInfo();
		//^ This no longer needs to get called anymore, so yeag

		//CALLER
		RequireReference<HitInfo> hit_info = HitInfo(a_hitData, dodge_info);

		//For now we're doing this dirty.
		hit_info->defender = targetData;
		hit_info->aggressor = attackValueOwner;
		hit_info->target = target;
		hit_info->attacker = attacker->As<RE::Actor>();
		bool test = false;
		bool* test2 = nullptr;
		////TemporaryFlag::Variable<bool> crash_flag(test, true);
		//TemporaryFlag::Variable<bool*> crash_flag2;(test2, &test);
		//A temporary flag, maybe recoil can use it. Right now, it's just for prosperities sake. But when magic comes in, THATS when this is really taking off.
		TemporaryFlag::Variable<IAttackInfo*> hit_info_flag(&targetData->strikeControl.flag_AttackInfo, &(*hit_info));


		bool isBash = (a_hitData.flags & HitFlag::kBash) == HitFlag::kBash;
		bool isPower = (a_hitData.flags & HitFlag::kPowerAttack) == HitFlag::kPowerAttack;
		bool isBlocked = (a_hitData.flags & HitFlag::kBlocked) == HitFlag::kBlocked;
		bool isTimed = (a_hitData.flags & HitFlag::kTimedBash) == HitFlag::kTimedBash;

		auto source = isBash && attackerData ? attackerData->GetBlockingForm() : a_hitData.weapon;

		
		{
			AttackType attack_type;

			//This should at a later point, only need to happen if dodge didn't happen or is invalid.
			if (isBash)
				attack_type = AttackType::Guard;
			else if (NULL_CONDITION_RET(a_hitData.weapon, false)->IsRanged() == true)
				attack_type = AttackType::Ranged;
			else
				attack_type = AttackType::Melee;

			hit_info->attackType = attack_type;
		}
		//For now, I will not bother with this.
		if (dodge_info)
		{
			//DisallowReference<DodgeInfo*, true> temp_info = const_cast<DodgeInfo*>(dodge_info);
			//These might take a pointer to hit info instead, so I can situationally use it. It's not set in stone, who knows.

			targetData->SendEvent("AfterContact", hit_info, attackValueOwner);

			//if (attackerData)
			//	attackerData->SendEvent("AfterAttackContact", dodge_info, hit_info, targetData);
			NULL_CONDITION(attackerData)->SendEvent("AfterAttackContact", hit_info, targetData);

			if (!dodge_info->allowStrike) {
				//Do AfterContact and return.
				RE::DebugMessageBox("Strike Ignored");
				__return;
			}


		}


		RequireReference<StrikeResult> restrictFlags = dodge_info ? dodge_info->hitRestrictions : StrikeResult::none;




		//Regardless remove this, so the timed recoil never goes through.
		// I have to do this reverse shit, cause otherwise it adds with ^, someone tell me I'm stupid and 
		// yell at me to fix this.
		//Note, instead of removing the flag I can use, prevent that type of recoil, I'm able, the hook is in there already
		//TESTING HOOK INSTEAD OF THIS
		//a_hitData.flags &= ~HitFlag::kTimedBash;


		//Delete this in time.
		if (!attackerData)//|| !source)
			__return;

#pragma region Guard::BashRestricts
		/*
		//source = !source ? attackerData->CheckAttackForm(source) : source;

		if (isBash)
		{
			if (isTimed)
				*restrictFlags |= StrikeResult::allDefend;
			else 
				a_hitData_pushBack = 1000;

			if (!isPower)
				*restrictFlags |= StrikeResult::majorAttack;

			//a_hitData_pushBack = 10000;

			//I'm thinking, should a block be neutral, I want it to do a minor stagger and forcibly push someone back.
			// dunno, it feels weird seeing it do no damage.
		}
		//*/
#pragma endregion


		//Critical hits might boost resist(did I mean precision?) a little, maybe a mult or something like that.
		QueryContext context;


		float distance = Utility::GetDistance(target, attacker);
		//bool isCounterBash = attackerData && isBash ? attackerData->guardControl.IsCounterBash() : false;

		context.SetForm(querySource, source);
		context.SetBool(queryBash, isBash);
		context.SetBool(queryTimed, isTimed);
		context.SetBool(queryBlocked, isBlocked);
		context.SetBool(queryPowerAction, isPower);
		context.SetFloat(queryDistance, distance);
#pragma region Guard::Counter
		//context.SetBool(queryCounter, isCounterBash);
#pragma endregion 


		using OnHitQuery = void(QueryContext& context, HitInfo& hit_info, StrikeResult& restrict_flags, CombatValueOwner* attacker);
		using OnAttackHitQuery = void(QueryContext& context, HitInfo& hit_info, StrikeResult& restrict_flags, CombatValueOwner* attacker);

		//NOTE ABOUT QUERY, it should be something that if someone wants to bail should allow someone to do so.

		//PLACAR::OnHitQuery-After the 
		targetData->SendEvent("OnHitQuery", context, hit_info, restrictFlags, attackValueOwner);


		//if (attackerData)
		//	attackerData->SendEvent("OnAttackHitQuery", dodge_info, context, restrictFlags, targetData);
		NULL_CONDITION(attackerData)->SendEvent("OnAttackHitQuery", context, hit_info, restrictFlags, targetData);




		//PLACAR::OnHitQuery-After the 
		targetData->SendEvent("BeforeHit", context, hit_info, restrictFlags, attackValueOwner);

		//if (attackerData)
		//	attackerData->SendEvent("BeforeAttackHit", dodge_info, context, restrictFlags, targetData);
		NULL_CONDITION(attackerData)->SendEvent("BeforeAttackHit", context, hit_info, restrictFlags, targetData);

		const float k_temp_DamagePercentPivot = 0.25f;//Doing less than this will reduce, doing more will increase.
		const float k_temp_DamagePointConversion = 0.1f;//Every point of damage does this much poise times weapon precision.
		static constexpr float k_temp_MinPoiseRegenDelay = 0.65f;//Minimum Time, minimum time it will take for poise to regen.


		float poiseDamage = 0;
		float poiseDelay = 0;
	
#pragma region Guard::HandleRecoil
		//DIVIDE: First should be guard, second should be poise Control
		//Should go to onAttackHit, that way I can have a weakened guard be responsible for allowing recoil again.
		if (isBlocked)
		{
			if (attackerData)//If defender data proclaims no recoil set ignore recoil
			{
				//at a later point, I'd like this to post processing the hit event, remove this itself
				// That way, no matter whats on recoil conditions, this is always done.
				//Make this use a temporary flag if you can. That thing does need some revisions ngl.
				//attackerData->ignoreRecoil = true;
				
				//attackerData->guardControl.ignoreRecoil = targetData->guardControl.ShouldRecoilIdle();
			}
		}
#pragma endregion 
#pragma region Poise::CalculatePoiseDamage
		//else//Block does not matter anymore.
		{
			//For now, we do it pure. Move this shit later plz. k thanks

			//Move this functionality so magic can use it later.
			float t = 4;
			float v = 6;//5;//I'm doing a rogue increase to 8. I was getting my poise broke too often.

			//Deviation on stagger to make the value more varied
			//float staggerDev = 0;


			//if (isBash)
			//Utility::DebugMessageBox()


			//float healthBase = a_actor->GetBaseActorValue(RE::ActorValue::kHealth);
			//float healthTemp = a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kHealth);
			//float healthPerm = a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kHealth);

			//float damagePercent = a_hitData.totalDamage / (healthBase + healthTemp + healthPerm);
			float damagePercent = a_hitData.totalDamage / (target->GetActorValue(RE::ActorValue::kHealth) * k_temp_DamagePercentPivot);

			damagePercent = Utility::Clamp(damagePercent, 0.5f, 2.f);

			float hitStagger = a_hitData.stagger;

			if (attacker && attacker->GetFormType() == RE::FormType::ActorCharacter)
				RE::ApplyPerkEntries(uEntryPoint::kModTargetStagger, attacker, target, &hitStagger);
			
			RE::ApplyPerkEntries(uEntryPoint::kModIncomingStagger, target, attacker, &hitStagger);


			//t /= Utility::Clamp(damagePercent, 2

			//This must get the blocking item if it's a bash. 
			float precision = attackValueOwner->GetPrecision(source);//Needs context.
			float weaponPrecision = ItemStatHandler::GetStatValue(source, isBash ? bashPrecision : basePrecision);
			poiseDamage = powf(hitStagger + 1, precision / t) * v - (v - weaponPrecision);
			//float poiseDamage = powf(hitStagger + 1, aggData->GetPrecision(a_hitData.weapon) / 4.f);//for now, weapon soz.
			float pre = poiseDamage;

			float damig = k_temp_DamagePointConversion * weaponPrecision * a_hitData.totalDamage;
			poiseDamage += damig * damagePercent;


			//poiseDamage += (damagePercent - k_temp_DamagePercentPivot) * poiseDamage;
			//The above is for chaos, to make it seem more varied. However, it needs one more element. A growth element.
			// that as you do more health damage point wise, you deal more poise damage. Which is important, given it
			// grows with health.
			//Also, if I want some real chaos, I can use current instead, capping the increase
			// to 50% of it's original value.
			//Lastly, I think what I'll do instead of applying this to the total, i'll 
			// apply it to stagger, where it will vary a lot more.

			//if (attacker->IsPlayerRef() == true)
			//	Utility::DebugMessageBox(std::format("Stagger: {}, formular: {}, pre: {}, dam: {}, per {}",
			//		a_hitData.stagger, poiseDamage, pre, damig, damagePercent));


			float speed = 1;
			float power = 0;
			
			if (1!=1 && source)
			{
				switch (source->GetFormType())
				{
					//The only one I'm doing right now.
				case RE::FormType::Weapon:
					auto weap = source->As<RE::TESObjectWEAP>();
					speed = weap->GetSpeed();
					power = weap->GetStagger();
				}
			}
			

			//poiseDelay = 2 * pow(power / speed, 2);
			//poiseDelay = __min(poiseDelay, k_temp_MinPoiseRegenDelay);//poiseDelay < k_temp_MinPoiseRegenDelay ? k_temp_MinPoiseRegenDelay : poiseDelay;
			poiseDelay = 2;//For now, I can't be bothered with this one.
		}
#pragma endregion 

		//Stagger has been removed, no longer used. But perhaps I should make a query for it...
		// while stagger has been removed, it will be done in block, so the stamina can actually be removed.
		//if (!isBlocked)//I want some way to put this into the block without breaking.
		//UPDATE, writecall over the function that does stuff here.
		a_hitData.stagger = 0;

/*
#pragma region Guard::DealCounterBash 
		//DIVIDE: Guard
		if (attackerData && isBash){
			a_hitData.totalDamage += attackerData->guardControl.GetBashDamage(target);
		}
#pragma endregion 
//*/
		//context.SetFloat(queryPostPoiseDamage, poiseDamage);
		//context.SetFloat(queryPostHealthDamage, a_hitData.totalDamage);
		//context.SetFloat(queryPostReflectDamage, a_hitData_reflectedDamage);



		//DIVIDE: Poise
#pragma region Poise::Expose
		float currentPoise = targetData->GetResource(CombatResource::Poise);
		auto state = targetData->poiseControl.GetCurrentState();

		//This needs to not include bash, at a later point it needs to actually fine out whether the poise break would happen.
		if ((isBash && !isPower) || state == PoiseState::Exposed && currentPoise - poiseDamage <= 0) {
			//RE::DebugMessageBox("cause poise");
			*restrictFlags |= StrikeResult::allDefend;//When your poise is going to be broken, no defensive actions.
		}
#pragma endregion
		//Figure out why poise was breaking so heavy when this didn't exist please.
		hit_info->poiseDamage = poiseDamage;

		StrikeResult result = targetData->strikeControl.ProcessHit(context, attacker, restrictFlags, hit_info);

		targetData->SendEvent("OnHit", context, hit_info, attackValueOwner);

		//if (attackerData)
		//	attackerData->SendEvent("OnAttackHit", context, hit_info, targetData);
		NULL_CONDITION(attackerData)->SendEvent("OnAttackHit", context, hit_info, targetData);

		//Will be cleared in time.
		poiseDamage = hit_info->poiseDamage;//context.GetFloat(queryPostPoiseDamage);
		//a_hitData.totalDamage = context.GetFloat(queryPostHealthDamage);
		//a_hitData_reflectedDamage = context.GetFloat(queryPostReflectDamage);

#pragma region Poise::Deliver
		//DIVIDE: Poise
		if ((result & StrikeResult::allMajor) != StrikeResult::none){
			//Major actions, no matter what they are remove poise damage. Mainly a debuff for heavy weaponry
			poiseDamage = 0;
		}
		

		//Things to factor now in post.
		// Removing lethiality when a blow is glanced, and reducing poise additively for minor actions
		// as well as doing some of the things major actions has lined up.

		if (poiseDamage > 0)//the whole, if a strike happens it breaks poise is temporary.
			targetData->poiseControl.DamagePoise(poiseDamage, poiseDelay, (isPower || (result | StrikeResult::allAttack) != StrikeResult::none) && !isBlocked);

		if (attackerData)
		{
			if (poiseDamage > 0 && poiseDelay > 0)
			{
				//EXPERIMENTAL: Poise Wait Time Drain
				attackerData->recoverControl.IncRegenDelay(0, RecoveryType::CombatResource, -poiseDelay);
			}

#pragma endregion 
	
#pragma region Melee::ComboFeedback
			//Need to move this shit to melee hit so it can stop fucking me in the ass.
			RE::TESObjectWEAP* weaponSource = context.GetForm<RE::TESObjectWEAP>(querySource);
			//Even if it's nothing, that's a melee attack.
			switch (Utility::GetWeaponType(weaponSource))
			{
			default:
				//Make this an event instead plz k thankies
				//attackerData->swingEvent.TrySuccess(weaponSource);
				//attackerData->meleeCombo.IncrementHit();
				attackerData->meleeCombo.TrySuccess(weaponSource);
				if (isBash) {
					attackerData->guardControl.SendTempStamina();
					//I might move this below, and have it work off of a value that's set to is bash if it passes through here.
				}
				break;

			case WeaponType::kBow:
			case WeaponType::kCrossbow:
			case WeaponType::kStaff:
				break;
			
			}
		}
#pragma endregion

		//I think I already added this, observe counter bash, because this might be doubling the effectiveness
#pragma region Guard::AddCounterHit
		/*
		if (!targetData->guardControl.IsWeakGuard() && isBlocked)
		{
			using func_t = float(*)(RE::HitData&);
			REL::Relocation<func_t> get_block_stamina{ REL::ID(25864) };
			float block_stamina = get_block_stamina(a_hitData);

			float blocked_damage = a_hitData.totalDamage * a_hitData.percentBlocked;
			float damage = a_hitData.totalDamage - blocked_damage;

			targetData->guardControl.AddCounterHit(attacker, damage, blocked_damage, block_stamina);
		}
		//*/
#pragma endregion 


		//if ((result | StrikeResult::allAttack) == StrikeResult::none) {
		//Utility::DebugMessageBox("A", targetData->IsPlayerRef());
		//targetData->guardControl.SendTempStamina();
		//}

		//__return;
		_ProcessHitEvent(target, a_hitData);


		targetData->SendEvent("AfterHit", context, hit_info, attackValueOwner);
		NULL_CONDITION(attackerData)->SendEvent("AfterAttackHit", context, hit_info, targetData);
		//if (attackerData)
		//	attackerData->SendEvent("AfterAttackHit", context, hit_info, targetData);
		
/*
#pragma region Guard::SendSpentStamina
		//DIVIDE:GuardControl
		if ((result & StrikeResult::exceptMajorAttack) == StrikeResult::none) {
			//float mod = result == StrikeResult::majorAttack ? -1 : 1;
			//float mod = result != StrikeResult::minorAttack;
			// I'm thinking up a scenerio where the perm stamina gets injuried.
			targetData->guardControl.SendTempStamina(result == StrikeResult::minorAttack ? -1 : 1);
		}
#pragma endregion 
//*/
	}


	

	DodgeInfo DodgeCheck(RE::TESObjectREFR* a_this, RE::Actor* target)
	{
		//Surprisingly enough, I'm still gonna use dodge check to avoid rewriting a ton of unimportant code.



		if (!target || !a_this || target->IsInKillMove() == true || target->IsDead() == true)
			return DodgeInfo::CreateGeneric(false);

		//If dodge is false, and I think it depends on what type of false it is, all hit actions will be null.
		// The only exception is when being hit during the auto dodge.


		if (CollisionIgnoreHandler::IsIgnoring(a_this, target) == true) {
			return DodgeInfo::CreateGeneric(true);
		}

		bool dodge = false;
		//bool isAuto = false;

		RequireReference<QueryContext> context{};
		
		//StrikeResult restrictFlags;

		RequireReference<StrikeResult> restrictFlags;//Mainly doing this so I don't have to flip every

		RE::Projectile* projectile = a_this->As<RE::Projectile>();
		//RE::NiPointer<RE::TESObjectREFR>
		RE::TESObjectREFR* attacker = projectile ? projectile->shooter.get().get() : a_this;
		RE::Actor* attackingActor = nullptr;
		
		if (!attacker)//no attacker ok?
			return DodgeInfo::CreateGeneric(false);


		CombatValueOwner* attackValueOwner = CombatData::GetValueOwner(attacker->GetFormID());



		RE::TESForm* source = nullptr;

		if (projectile)
		{
			source = projectile->weaponSource;
			source = source ? source : projectile->spell;// What of explosions?
		}
		else
		{
			attackingActor = a_this->As<RE::Actor>();

			source = Utility::GetAttackingWeapon(attackingActor);
		}



		//Move this shit into its own handler.				
		//CombatData* attackerData = CombatData::GetDataRecord(attacker->GetFormID());
		
		CombatData* defenderData = CombatData::GetDataRecord(target->GetFormID());
		
		if (!defenderData)
			return DodgeInfo::CreateGeneric(false);

		RequireReference<DodgeInfo> info{};

		info->defender = defenderData;
		info->aggressor = attackValueOwner;
		info->target = target;
		info->attacker = attacker->As<RE::Actor>();
		
		bool isBash = false;//a_event->flags.any(HitEventFlag::kBashAttack);
		bool isPower = false;//a_event->flags.any(HitEventFlag::kPowerAttack);

		if (attackingActor)
		{
			auto attackData = Utility::GetCurrentAttackData(attackingActor);

			if (attackData)
			{
				isBash = attackData->data.flags.any(AttackDataFlags::kBashAttack);
				isPower = attackData->data.flags.any(AttackDataFlags::kPowerAttack);
			}
		}


		float distance = Utility::GetDistance(target, attacker);

		//Source needs to be checked
		context->SetForm(querySource, source);
		context->SetBool(queryBash, isBash);
		context->SetBool(queryPowerAction, isPower);
		context->SetFloat(queryDistance, distance);

		if (projectile)
			context->SetForm(queryProjectile, projectile);
		
		//PLACAR::OnContactQuery-This is where action point loads itself, instead of this doing it.
		defenderData->SendEvent("OnContactQuery", context, restrictFlags, attackValueOwner);
		

		//Should get the action point instead.
		if (defenderData->actionPointControl.IsDodging() == false) {
			//Finalize info some how.
			defenderData->SendEvent("OnContact", context, info, attackValueOwner);
			return info;
		}

		//PLACAR:BeforeContact-Should be the place to edit restrict stuff. The above should be able to do it too, but this is just for when
		defenderData->SendEvent("BeforeContact", context, restrictFlags, attackValueOwner);



		const StrikeResult result = defenderData->strikeControl.ProcessDodge(context, attacker, restrictFlags, info);



		if ((result & StrikeResult::allDefend) != StrikeResult::none){
			//We succesfully dodged.
			CollisionIgnoreHandler::IgnoreReference(a_this, target);
			info->SetSuccess(true, StrikeResult::none);
		}
		else if (result == StrikeResult::majorAttack){
			info->SetSuccess(false, StrikeResult::total);
		}
		else{
			info->SetSuccess(false, StrikeResult::none);
		}
	
		//Should be the one to handle on stop This might have to go through whether it succeed or not, and as such should likely come
		// with 
		defenderData->SendEvent("OnContact", context, info, attackValueOwner);

		//if (isAuto)//Replace this with a function call that specifically puts this shit on cooldown yeah?
		//defenderData->actionPointControl.StopAction(ActionPoint::autoDodge, true);


		return info;
	}



	//Handles melee collisions, and evade related activities
	void CrucibleHook::OnMeleeHit(RE::Actor* hit_causer, RE::Actor* hit_target, std::int64_t a_int1, bool a_bool, void* a_unkptr)
	{
		//if (hit_target)
		//	Utility::DebugMessageBox(std::format("Hit: ", hit_target->GetName()).c_str());
		bool test = false;
		bool* test2 = nullptr;
		////TemporaryFlag::Variable<bool> crash_flag(test, true);
		//TemporaryFlag::Variable<bool*> crash_flag2;(test2, &test);
		auto dodge_result = DodgeCheck(hit_causer, hit_target);

		if (dodge_result == false){//Needs some way to interpret this dodge check, return hit results instead.

			CombatData* data = dodge_result.defender;

			TemporaryFlag::Variable<IAttackInfo*> dodge_data_flag = TemporaryFlag::Variable<IAttackInfo*>();

			if (data)
				dodge_data_flag(data->strikeControl.flag_AttackInfo, &dodge_result);

			_OnMeleeHit(hit_causer, hit_target, a_int1, a_bool, a_unkptr);
		}
	}

	//When loading projectiles, all I need to do is make sure the projectile exists, if it doesn't don't
	// load it.
	//Also, these are numbers so
	


	void CrucibleHook::ProjectileCollision(RE::Projectile* a_this, RE::hkpAllCdPointCollector* cdPoints)
	{
		//std::int64_t v31 = 0 << 6;
		//void* first = (void*)((int64_t)a2 + 0x10);
		if (a_this->GetFormType() == RE::FormType::ProjectileFlame)
			Utility::DebugMessageBox("BIG GUY");

		DodgeInfo dodge_result{};
		bool test = false;
		bool* test2 = nullptr;
		//TemporaryFlag::Variable<bool> crash_flag(test, true);
		//TemporaryFlag::Variable<bool*> crash_flag2;(test2, &test);
		//crash_flag2(test2, &test);
		TemporaryFlag::Variable<IAttackInfo*> dodge_data_flag = TemporaryFlag::Variable<IAttackInfo*>();

		for (auto& point : cdPoints->hits)
		{

			auto objRef = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);
			
			if (objRef)
			{
				dodge_result = DodgeCheck(a_this, objRef->As<RE::Actor>());
				
				if (dodge_result)
					break;
			}
			
			/*
			if (objRef)
			{
				dodge = DodgeCheck(a_this, objRef->As<RE::Actor>());

				if (dodge)
					break;
			}
			//*/
		}
		

		CombatData* data = dodge_result.defender;

		if (data) {//Does this crash because of the difference in assigning scope and defining scope?
			dodge_data_flag(data->strikeControl.flag_AttackInfo, &dodge_result);
		}

	

		if (!dodge_result)
		{
			//Need a way for this not to happen when cone, AND still do it when cone is done. Deconstructor?
			CollisionIgnoreHandler::ClearIgnoreList(a_this);


			switch (a_this->GetFormType())
			{
			default:
			case RE::FormType::ProjectileBarrier:
			case RE::FormType::ProjectileGrenade:
				//_ProjectileCollision_Beam//Nothing for now, this shouldn't happen but fire missile instead in case
			case RE::FormType::ProjectileMissile:
				_ProjectileCollision_Missile(a_this, cdPoints);
				break;

			case RE::FormType::ProjectileArrow:
				_ProjectileCollision_Arrow(a_this, cdPoints);
				break;

			case RE::FormType::ProjectileBeam:
				_ProjectileCollision_Beam(a_this, cdPoints);
				break;

			case RE::FormType::ProjectileFlame:
				_ProjectileCollision_Flame(a_this, cdPoints);
				break;

			case RE::FormType::ProjectileCone:
				_ProjectileCollision_Cone(a_this, cdPoints);
				break;
			}
		}
	}
	//*
	RE::TESObjectREFR* CrucibleHook::NiNodeTest(RE::NiNode* a1, std::int64_t a2)
	{
		//shit but it works. Needs further testing.

		if (!projectileStore || projectileStore->GetFormType() == RE::FormType::ProjectileGrenade) {
			return CrucibleHook::_NiNodeTest(a1, a2);
		}

		auto refr = CrucibleHook::_NiNodeTest(a1, a2);

		DodgeInfo dodge_result;

		if (refr && refr->GetFormType() == RE::FormType::ActorCharacter)
		{
			dodge_result = DodgeCheck(projectileStore, refr->As<RE::Actor>());

			if (dodge_result)
				return nullptr;
			//Utility::DebugMessageBox(std::format("{} vs attacker ({})",
			//	refr->GetName(), attacker->GetName()));
		}

		CombatData* data = dodge_result.defender;
		bool test = false;
		////TemporaryFlag::Variable<bool> crash_flag(test, true);
		TemporaryFlag::Variable<IAttackInfo*> dodge_data_flag = TemporaryFlag::Variable<IAttackInfo*>();

		if (data)
			dodge_data_flag(data->strikeControl.flag_AttackInfo, &dodge_result);

		return refr;
	}
	//*/




	/// <summary>
	///Default animation processing. All processing both characters and players go through will be done here.
	/// </summary>
	void CrucibleHook::ProcessAnimEvent(AnimSink* a_this, const AnimEvent* a_event, AnimSource* a_dispatcher)
	{
		if (!a_event || !a_event->holder || a_event->holder->GetFormType() != RE::FormType::ActorCharacter)
			return;
		RE::TESObjectREFR* holder = const_cast<RE::TESObjectREFR*>(a_event->holder);
		RE::Actor* actor = holder->As<RE::Actor>();

		//Want to figure out why this doesn't work for actor update. perhaps, put this in an init override?
		CombatData* combatData = CombatData::GetDataRecord(a_event->holder->formID);

		//if (!combatData)
		//	combatData = CombatData::AddDataRecord(a_event->holder->formID);

		if (!combatData) return;

		switch (hash(a_event->tag.data(), a_event->tag.size()))
		{
		case "arrowRelease"_h:
		{
			//RE::TESObjectWEAP* weap = combatData->rightHand ? nullptr : combatData->rightHand->As<RE::TESObjectWEAP>();
			ItemType rightType = Utility::GetItemType(combatData->GetHand(true));

			//if (weap && RE::WEAPON_TYPE::kCrossbow == *weap->weaponData.animationType)
			if (rightType == ItemType::kCrossBow)
			{
				holder->NotifyAnimationGraph("attackStop");
				combatData->crossbowLoaded = false;
			}
		}
			
			break;
		case "reloadStop"_h:
			combatData->crossbowLoaded = true;
			

		case "AttackWinStart"_h:
		case "AttackWinStartLeft"_h:
			//combatData->swingEvent.TryFailure();
			//combatData->attackControl.GoToState(AttackState::kPosthitCalc);
			//holder->NotifyAnimationGraph("BashFail");
			break;


			
		}

		auto eventName = std::format("OnGraphOutputEvent_{}", a_event->tag.data());
		
		//EventObject params;
		//params.PushString(a_event->tag.data());

		std::string event_name = a_event->tag.data();
		combatData->SendEvent(eventName, event_name);

		combatData->SendEvent("OnGraphOutputEvent", event_name);

		if (holder->IsPlayerRef() == true)
			logger::info("<{}>", a_event->tag.data());
		



		eventName = std::format("AfterGraphGraphOutputEvent_{}", event_name);

		combatData->SendLockedEvent(eventName, event_name);
		combatData->SendLockedEvent("AfterGraphOutputEvent", event_name);

		//auto combatData = CombatData::GetDataRecord(a_event->holder->formID);
		//if (!combatData) {combatData = CombatData::AddDataRecord(a_event->holder->formID);}

	}

	bool CrucibleHook::NotifyAnimationGraph(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName)
	{
#define __return return isPlayer ? _NotifyPlayerAnimationGraph(a_this, a_eventName) : _NotifyCharacterAnimationGraph(a_this, a_eventName)
#define __return_with(a_send_value) isPlayer ? _NotifyPlayerAnimationGraph(a_this, a_send_value) : _NotifyCharacterAnimationGraph(a_this, a_send_value)

		//Rework time, before events have a second parameter, force true, force false. For notifications, we can use this to send true
		// which will intercept the get want blocking for block.


		const char* empty = "";

		auto actor = skyrim_cast<RE::Actor*>(a_this);
		auto combat_data = CombatData::GetDataRecord(actor);

		bool isPlayer = actor ? actor->IsPlayerRef() : false;

		if (!actor || !combat_data) {
			__return;
		}

		std::string sendEvent = a_eventName.data();

		logger::info("Input: [{}]", sendEvent);

		//Utility::DebugNotification(a_eventName.c_str());
		//make sure to cover dwarven as well later.
		

		switch (hash(a_eventName.data(), a_eventName.size()))
		{
		//This needs to be moved to ranged,
		case "crossbowAttackStart"_h:
		{
			if (combat_data->crossbowLoaded == false)
			{
				//reload fast?
				sendEvent = "reloadStart";
			}
		}
		break;
		//This particular element is something that should be moved to guard in all likeliness.
		case "recoilStart"_h:
		{
			break;

			if (combat_data->ignoreRecoil) {
				combat_data->ignoreRecoil = false;
				return false;
			}
		}
		break;
		//*
	
		
		break;
		/*
		case "blockStop"_h:
		{
			bool result;//We can use attack states for this.
			if (!actor->GetGraphVariableBool("isBashing", result) || !result)
				combatData->guardControl.LowerGuard();

		}
		
		break;
		
		case "attackStop"_h:
		{
			bool result;
			if (!actor->GetGraphVariableBool("isBashing", result) || !result)
				combatData->guardControl.LowerGuard();
		}
		break;
		//*/

		default:
		{
			//Make this it's own functional event, after specific, and after general,
			// we should then send out an event, before attack, which is registered specifically to wait for attacks
			// if it doesn't know what attack it would be.

			//This is for non-specific application
			break;
			auto attackData = Utility::GetAttackData(actor, a_eventName);

			if (attackData)
			{
				if (attackData->data.flags.all(AttackDataFlags::kPowerAttack) == true)
				{
					if (actor->HasMagicEffect(Resources::paCooldown) == true) {
						return false;
						sendEvent = empty;
					
					}
				}
				//else if (attackData->data.flags.all(AttackDataFlags::kBashAttack) == true){combatData->guardControl.TryBashingBlock(actor);}
				//Make a function in melee controller for this.
				else if (combat_data && combat_data->meleeCombo.IsActive() && actor->GetActorValue(RE::ActorValue::kStamina) <= 0) {
					return false;
					sendEvent = empty;
				}
			}
		}
		break;

		}

		const char* original_event = a_eventName.data();
		std::string send_event = a_eventName.data();

		std::string data_event{};

		bool return_original = true;

		//The attack data seen here is something I have no intention of changing. So I want to test out const on this and if it will let me do this.
		RE::BGSAttackData* attack_data = Utility::GetAttackData(actor, send_event);

		if (attack_data)
		{
			logger::info("hit attack data");

			data_event = "BeforeCombatGraphEvent_" + send_event;

			for (int i = 0; i < 2 && attack_data; i++)
			{
				//We want to disallow attackData being a reference, changing it for others would yield nothing.
				if (combat_data->SendEvent(data_event, send_event, DisallowReference{ attack_data }) == EventControl::kStop || send_event == empty) {
					//Do not send null event.
					return false;
				}
				else if (send_event != original_event) {
					return_original = false;//Event has been corrupted, cannot return original anymore.
					attack_data = Utility::GetAttackData(actor, send_event);//We also want to check if it's even an attack event anymore.
				}

				data_event = "BeforeCombatGraphEvent";
			}
		}



		data_event = "BeforeGraphInputEvent_" + send_event;

		for (int i = 0; i < 2; i++)
		{
			if (combat_data->SendEvent(data_event, send_event) == EventControl::kStop || send_event == empty) {
				//Do not send null event.
				return false;
			}
			else if (send_event != original_event) {
				return_original = false;//Event has been corrupted, cannot return original anymore.
			}

			data_event = "BeforeGraphInputEvent";
		}
		

		const RE::BSFixedString sending = sendEvent.c_str();
		auto result = __return_with(sending);



		if (attack_data)
		{
			data_event = "AfterCombatGraphEvent_" + send_event;

			combat_data->SendLockedEvent(data_event, send_event, attack_data);
			combat_data->SendLockedEvent("AfterCombatGraphEvent", send_event, attack_data);
		}


		data_event = "AfterGraphInputEvent_" + send_event;

		combat_data->SendLockedEvent(data_event, send_event);
		combat_data->SendLockedEvent("AfterGraphInputEvent", send_event);

		return return_original ? result : false;



		/*
		if (a_eventName == "crossbowAttackStart")
		{
			if (combatData && combatData->crossbowLoaded == false)
			{
				//reload fast?
				sendEvent = "reloadStart";
			}

		}
		else if (a_eventName == "recoilStart")
		{
			if (combatData && combatData->ignoreRecoil) {

				sendEvent = "";
				combatData->ignoreRecoil = false;
			}
		}
		else
		{
			//This is for non-specific application
			auto attackData = Utility::GetAttackData(actor, a_eventName);

			if (attackData)
			{
				if (attackData->data.flags.all(AttackDataFlags::kPowerAttack) == true)
				{
					if (actor->HasMagicEffect(Resources::paCooldown) == true) {
						sendEvent = "";
					}
				}
				else if (combatData && combatData->meleeCombo.IsComboActive() && actor->GetActorValue(RE::ActorValue::kStamina) <= 0) {
					sendEvent = "";
				}
			}
		}
		//*/
		//Hook is currently broken, would like to rearrange this entire deal. Most notably, to remove the seperation of the events for
		// single types (good if you just want to do one thing and go) and all types (good when you have multiple events for 1 thing.
		// I would like this to functionally be able to have 1 function for this. The simple idea would be, one event node would point to another
		// or someting like that.
		//__return;

		/*

		if (sendEvent == "")
			return false;

		std::string send_arg = sendEvent != "none" ? a_eventName.data() : sendEvent;

		auto event_name = std::format("BeforeGraphInputEvent_{}", send_arg);

		if (combatData->SendEvent(event_name, send_arg) == EventControl::kStop) {
			//sendEvent = empty;
			return false;
		}
		else if ((sendEvent != "none" ? a_eventName.data() : sendEvent) != send_arg) {
			//This is so it doesn't fall out of scope, make reference functions for these if you can
			// and if advisable.
			//RE::DebugMessageBox(params[0].AsString());
			//eventName = params[0].AsString();
			sendEvent = eventName.c_str();
		}



		//*/


		//MAKE ALL EVENTS

		//Note, this shit is the worst shit I've ever seen and I should be put to death for it but for now, you know.
		/*
		if (sendEvent == "")
			return false;

		auto eventName = std::format("BeforeGraphInputEvent_{}", sendEvent);
		//EventObject params;
		//params.PushString(a_eventName.data());
		
		std::string param_1 = a_eventName.data();//Shouldn't this be loading send data?

		auto old_hash = hash(sendEvent);

		//pull the event here.
		if (combatData->SendEvent(eventName, sendEvent) == EventControl::kStop || sendEvent == "") {
			//sendEvent = empty;
			return false;
		}

		eventName = std::format("OnGraphInputEvent_{}", sendEvent);

		combatData->SendLockedEvent(eventName, sendEvent);
		combatData->SendLockedEvent("OnGraphInputEvent", sendEvent);
		


		//Send event shouldn't be "" ever, but just accounting for now.
		const RE::BSFixedString sending = sendEvent.c_str();
		auto result = __return_with(sending);


		eventName = std::format("AfterGraphInputEvent_{}", sendEvent);

		combatData->SendLockedEvent(eventName, sendEvent);
		combatData->SendLockedEvent("AfterGraphInputEvent", sendEvent);

		return result;
		//*/
	}


	/// <summary>
	/// This exists for npcs rather than playable characters.
	/// </summary>
	/// <param name="a_this"></param>
	/// <param name="a_event"></param>
	/// <param name="a_dispatcher"></param>
	EventResult CrucibleHook::ProcessPlayerAnim(AnimSink* a_this, const AnimEvent* a_event, AnimSource* a_dispatcher)
	{
		auto result  = _ProcessPlayerAnim(a_this, a_event, a_dispatcher);
		ProcessAnimEvent(a_this, a_event, a_dispatcher);
		return result;

		//Utility::DebugNotification(a_event->tag.c_str());
		ProcessAnimEvent(a_this, a_event, a_dispatcher);
		return _ProcessPlayerAnim(a_this, a_event, a_dispatcher);
	}

	/// <summary>
	/// This exists for npcs rather than playable characters.
	/// </summary>
	/// <param name="a_this"></param>
	/// <param name="a_event"></param>
	/// <param name="a_dispatcher"></param>
	EventResult CrucibleHook::ProcessCharacterAnim(AnimSink* a_this, const AnimEvent* a_event, AnimSource* a_dispatcher)
	{
		auto result = _ProcessCharacterAnim(a_this, a_event, a_dispatcher);
		ProcessAnimEvent(a_this, a_event, a_dispatcher);
		return result;

		//Utility::DebugNotification(a_event->tag.c_str());
		ProcessAnimEvent(a_this, a_event, a_dispatcher);
		return _ProcessCharacterAnim(a_this, a_event, a_dispatcher);
	}

	//Used to be int32
	uMagicCastState CrucibleHook::Func29(RE::ActorMagicCaster* a_this, void* a2, float delta)
	{
		return _Func29(a_this, a2, delta);

#ifdef kill_code
		CombatData* combatData = CombatData::GetDataRecord(a_this->actor);

		if (!combatData)
			return _Func29(a_this, a2, delta);

		auto castData = combatData->magicControl.GetCasterData(a_this);

		if (!a_this->currentSpell && (!castData || castData->previousState == uMagicCastState::kNone) == true)
			return _Func29(a_this, a2, delta);


		//if instant, return. Me no care.
		bool isRightHand = a_this->GetCastingSource() == uCastingSource::kRightHand;

		RE::ActorValue av = a_this->currentSpell ? a_this->currentSpell->GetActorValueForCost(isRightHand) : RE::ActorValue::kNone;

		//Utility::DebugNotification(std::format("av {}", (int)av).c_str());

		float prevAV = av == RE::ActorValue::kNone ?
			0.f : a_this->actor->GetActorValue(av);

		//this can be in the actor update, believe it or not.
		uMagicCastState result = _Func29(a_this, a2, delta);

		//*

		float avDiff = av == RE::ActorValue::kNone ?
			0.0f : prevAV - a_this->actor->GetActorValue(av);

		//Much to my surprise this is exactly what I'm looking for.
		if (avDiff > 0.0f)
		{
			if (!castData)
				castData = combatData->magicControl.GetCreateCasterData(a_this);
			
			castData->resourceStore += avDiff;

			//if (state <= RE::MagicCastState::kReleased)
			//else if (state == RE::MagicCastState::kConcentrating)


			//The accuracy is off because of this, but you should have something to account for this.
			//In this scenerio, conc goes straight to the total pool, while regular casts
			// get their values built up first in a map of casting types. The map should basically be
			// casting type, float. no need to make it super complicated or even reference the original magic casters
			// If I can initialize it with an allocated size for 3, that would be nice.
		}

		combatData->magicControl.EvaluateState(a_this, delta);

		/*/
		if (a_this->GetCastingSource() == RE::MagicSystem::CastingSource::kRightHand && a_this->currentSpell)
		{
			static float build = 0;
			static float amount = 0;

			auto castType = a_this->currentSpell->GetCastingType();
			switch (result)
			{
			case 0:
				build = 0;
			case 2://Charging

				if (castType != RE::MagicSystem::CastingType::kConcentration)
					build += delta * a_this->currentSpellCost;
			case 3://Charged
			//Nothing, it's a fall through.
				break;

			case 4://Release, submit like conc
				if (build)
					Utility::DebugMessageBox(std::format("{} build, {} amount", build, amount).c_str());
				amount += build;
				build = 0;
				break;
			case 6://Concentrate
				amount += delta * a_this->currentSpellCost;
				Utility::DebugNotification(std::format("{} build, {} amount", build, amount).c_str());
				break;
			default:
				break;
			}
		}
		//*/

		return result;
#endif
	}
}