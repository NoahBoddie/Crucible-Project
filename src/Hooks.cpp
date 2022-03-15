#include "Hooks.h"
#include "Utility.h"
#include "CombatData.h"
#include "ItemStatHandler.h"
#include "TimeManager.h"
#include "AttackStatController.h"
#include "CollisionIgnoreHandler.h"
#include <cmath>

#include "QueryContext.h"

namespace Crucible
{
	std::int64_t CrucibleHook::ActorUpdate(RE::Actor* a_this, float delta, void* a3)
	{

		//This shit is likely busted for actors. fair notes.
		std::int64_t result = a_this->IsPlayerRef() ? _PlayerUpdate(a_this, delta, a3) : _CharacterUpdate(a_this, delta, a3);


		if (1==2)
		{
			static int lastR = 0;
			static int lastL = 0;
			static int lastI = 0;
			static int lastO = 0;

			auto mrCast = a_this->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand);
			auto mlCast = a_this->GetMagicCaster(RE::MagicSystem::CastingSource::kLeftHand);
			auto moCast = a_this->GetMagicCaster(RE::MagicSystem::CastingSource::kOther);
			auto miCast = a_this->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);

			int r_value = (int)(*mrCast->state);
			int l_value = (int)(*mlCast->state);
			int o_value = (int)(*moCast->state);
			int i_value = (int)(*miCast->state);

			if (mrCast && r_value != lastR)
			{
				RE::DebugMessageBox(std::format("cast state right: {}", r_value).c_str());
				lastR = r_value;
			}

			if (mlCast && l_value != lastL)
			{
				RE::DebugMessageBox(std::format("cast state left: {}", l_value).c_str());
				lastL = l_value;
			}

			if (miCast && i_value != lastI)
			{
				RE::DebugMessageBox(std::format("cast state instant: {}", i_value).c_str());
				lastI = i_value;
			}

			if (moCast && o_value != lastO)
			{
				RE::DebugMessageBox(std::format("cast state other: {}", o_value).c_str());
				lastO = o_value;
			}
		}
		
		CombatData* data = CombatData::GetDataRecord(a_this);

		if (!data) {
			data = CombatData::AddDataRecord(a_this);
		}


		if (!data)
			return result;
		
		if (!delta)
			delta = *g_deltaTime;

		data->Update(delta, a_this);
		//data->attackControl.EvaluateState(a_this->GetAttackState());
		//data->motionControl.EvaluateState();
		//data->recoverControl.RecoveryUpdate(a2);
		return result;
	}



	void CrucibleHook::MainUpdate(RE::Main* a_this, float a_val)
	{
		_MainUpdate(a_this, a_val);

		auto a_ui = RE::UI::GetSingleton();
			
		//if (!a_ui->numPausesGame && !a_this->freezeTime) TaskTimerHandler::GetSingleton()->Update();
		if (!a_ui->numPausesGame && !a_this->freezeTime) {
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
		
		auto actor = skyrim_cast<RE::Actor*>(a_this);


		if (actor) {
			CombatData* data = CombatData::GetDataRecord(actor);

			if (data)
			{
				if (result > 0)
				{
					if (attack_data->data.flags.any(AttackDataFlags::kBashAttack) == true)
					{
						//I don't really want to do this, I just want a seperation of value.
						data->blockControl.IncrementActionStamina(abs(result));
					}
					else
					{
						data->meleeCombo.usedResource += abs(result);
					}
					
					//RE::DebugMessageBox(std::format("{} price", result));
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
		
		data->blockControl.IncrementActionStamina(abs(result));

		//RE::DebugMessageBox(std::format("{} stamina on the block", result));

		return result;
	}


	/// <summary>
	/// Processes the hit data, handles stagger and other hit related calculations
	/// </summary>
	/// <param name="a_actor"></param>
	/// <param name="a_hitData"></param>
	void CrucibleHook::ProcessHitEvent(RE::Actor* a_actor, RE::HitData& a_hitData, float* a3)
	{
		//RE::DebugMessageBox(std::format("Crit: {}, Extra: {}, Stagger: {}, pushback: {}",
		//	a_hitData.criticalDamageMult, a_hitData.bonusHealthDamageMult, a_hitData.stagger, a_hitData.pushBack));
		auto aggressor = a_hitData.aggressor.get();
		auto aggData = CombatData::GetDataRecord(aggressor.get());

		

		//This function requires 1 last value, a float seemingly
		// int32 __usercall TESObjectREFR::sub_140626400@<eax>(TESObjectREFR *target@<rcx>, int64 a2@<rdx>, __m128 *xmm0_0@<xmm0>)
		auto data = CombatData::GetDataRecord(a_actor);

		if (data)
		{
			if ((a_hitData.flags & HitFlag::kBlocked) == HitFlag::kBlocked)
			{
				//RE::DebugMessageBox(std::format("flags: {} / {}",
				//	(int)a_hitData.flags, (int)HitFlag::kPowerAttack));
				
				if (aggressor && aggData)
				{
					//at a later point, I'd like this to post processing the hit event, remove this itself
					// That way, no matter whats on recoil conditions, this is always done.

					aggData->ignoreRecoil = true;
				}	
				else
				{
					aggData->ignoreRecoil = false;
				}
			}
			else
			{
				//Move this functionality so magic can use it later.
				float t = 4;
				float v = 10;

				//Deviation on stagger to make the value more varied
				//float staggerDev = 0;

				bool isBash = (a_hitData.flags & HitFlag::kBash) == HitFlag::kBash;

				//if (isBash)
				//RE::DebugMessageBox()

				RE::TESForm* hitForm = isBash ? aggData->GetBlockingForm() : a_hitData.weapon;

				//This must get the blocking item if it's a bash. 
				float precision = aggData->GetPrecision(hitForm);
				float weaponPrecision = ItemStatHandler::GetStatValue(hitForm, isBash ? bashPrecision : basePrecision);
				float poiseDamage = powf(a_hitData.stagger + 1, precision / t) * v - (v - weaponPrecision);
				//float poiseDamage = powf(a_hitData.stagger + 1, aggData->GetPrecision(a_hitData.weapon) / 4.f);//for now, weapon soz.


				bool power = (a_hitData.flags & HitFlag::kPowerAttack) == HitFlag::kPowerAttack;

				if (aggressor->IsPlayerRef() == true)
					RE::DebugMessageBox(std::format("Stagger: {}, formular: {}, power: {} / {}",
						a_hitData.stagger, poiseDamage, (int)a_hitData.flags, (int)HitFlag::kPowerAttack));

				float placeHolder = 5.f;

				//data->DamagePoise(poiseDamage, placeHolder, power);
				data->poiseControl.DamagePoise(poiseDamage, placeHolder, power);
			}

		}

		return _ProcessHitEvent(a_actor, a_hitData, a3);

	};



	

	bool DodgeCheck(RE::TESObjectREFR* a_this, RE::Actor* target)
	{
		//Surprisingly enough, I'm still gonna use dodge check to avoid rewriting a ton of unimportant code.

		if (!target || !a_this || target->IsInKillMove() == true || target->IsDead() == true)
			return false;

		//If dodge is false, and I think it depends on what type of false it is, all hit actions will be null.
		// The only exception is when being hit during the auto dodge.


		if (CollisionIgnoreHandler::IsIgnoring(a_this, target) == true) {
			return true;
		}

		bool dodge = false;
		//bool isAuto = false;

		QueryContext context;
		
		StrikeResult restrictFlags;


		RE::Projectile* projectile = a_this->As<RE::Projectile>();
		//RE::NiPointer<RE::TESObjectREFR>
		RE::TESObjectREFR* attacker = projectile ? projectile->shooter.get().get() : a_this;
		RE::Actor* attackingActor = nullptr;
		if (!attacker)//no attacker ok?
			return false;

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


		//Source needs to be checked
		context.SetForm(querySource, source);
		context.SetBool(queryBash, isBash);
		context.SetBool(queryPowerAction, isPower);
		
		if (projectile)
			context.SetForm(queryProjectile, projectile);
		
		
		
		if (defenderData->actionPointControl.IsDodging() == false)
		{
			//if we are viable for auto dodge.
			if (defenderData->actionPointControl.IsOnCooldown(ActionPoint::autoDodge) == true)
				return false;
			

			CombatValueOwner* attackValueOwner = CombatData::GetValueOwner(attacker->GetFormID());

			if (!attackValueOwner)
				return false;

			float evade = defenderData->GetEvade(context, attackValueOwner->GetPortList());

			context.SetFloat(queryPostEvade, evade);

			if (defenderData->actionPointControl.StartDodge(ActionPoint::autoDodge, evade) == false)
				return false;

			//When this is happening, only minor defend can happen.

			if (isPower)
				restrictFlags |= StrikeResult::allAttack;
			else
				restrictFlags |= StrikeResult::exceptMinorDefend;


			//isAuto = true;

		}
		


		auto result = defenderData->strikeControl.ProcessDodge(context, attacker, restrictFlags);

		if ((result & StrikeResult::allDefend) != StrikeResult::none)
		{
			//We succesfully dodged.
			CollisionIgnoreHandler::IgnoreReference(a_this, target);
			dodge = true;
		}
		else if ((result & StrikeResult::allAttack) != StrikeResult::none)
		{
			//We got hit out of the dodge.
			//Send according flags.
			defenderData->temp_IgnoreHitStrike = true;// Hooks will have to clear this as soon as possible.
		}
		else
		{
			//else you just got hit. When this happens,
		}

		//if (isAuto)//Replace this with a function call that specifically puts this shit on cooldown yeah?
		defenderData->actionPointControl.StopAction(ActionPoint::autoDodge, true);


		return dodge;
	}



	//Handles melee collisions, and evade related activities
	void CrucibleHook::OnMeleeHit(RE::Actor* hit_causer, RE::Actor* hit_target, std::int64_t a_int1, bool a_bool, void* a_unkptr)
	{
		//if (hit_target)
		//	RE::DebugMessageBox(std::format("Hit: ", hit_target->GetName()).c_str());
		if (DodgeCheck(hit_causer, hit_target) == false){
			return _OnMeleeHit(hit_causer, hit_target, a_int1, a_bool, a_unkptr);
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
			RE::DebugMessageBox("BIG GUY");

		bool dodge = false;

		for (auto& point : cdPoints->hits)
		{

			auto objRef = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);
			
			if (objRef)
			{
				dodge = DodgeCheck(a_this, objRef->As<RE::Actor>());

				if (dodge)
					break;
			}
		}
		
		if (!dodge)
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
				return _ProjectileCollision_Missile(a_this, cdPoints);

			case RE::FormType::ProjectileArrow:
				return _ProjectileCollision_Arrow(a_this, cdPoints);

			case RE::FormType::ProjectileBeam:
				return _ProjectileCollision_Beam(a_this, cdPoints);

			case RE::FormType::ProjectileFlame:
				return _ProjectileCollision_Flame(a_this, cdPoints);

			case RE::FormType::ProjectileCone:
				return _ProjectileCollision_Cone(a_this, cdPoints);
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
		
		if (refr && refr->GetFormType() == RE::FormType::ActorCharacter)
		{
			bool dodge = DodgeCheck(projectileStore, refr->As<RE::Actor>());

			if (dodge)
				return nullptr;
			//RE::DebugMessageBox(std::format("{} vs attacker ({})",
			//	refr->GetName(), attacker->GetName()));
		}

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
			if (combatData)
			{
				combatData->crossbowLoaded = true;
			}

		case "AttackWinStart"_h:
		case "AttackWinStartLeft"_h:
			//combatData->swingEvent.TryFailure();
			//combatData->attackControl.GoToState(AttackState::kPosthitCalc);
			break;


			
		}

		//auto combatData = CombatData::GetDataRecord(a_event->holder->formID);
		//if (!combatData) {combatData = CombatData::AddDataRecord(a_event->holder->formID);}

	}

	bool CrucibleHook::NotifyAnimationGraph(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName)
	{
		auto actor = skyrim_cast<RE::Actor*>(a_this);
		auto combatData = CombatData::GetDataRecord(actor);

		if (!actor || !combatData) {
			return _NotifyAnimationGraph(a_this, a_eventName);
		}

		const char* sendEvent = nullptr;

		//RE::DebugNotification(a_eventName.c_str());
		//make sure to cover dwarven as well later.
		
		switch (hash(a_eventName.data(), a_eventName.size()))
		{
		case "crossbowAttackStart"_h:
		{
			if (combatData && combatData->crossbowLoaded == false)
			{
				//reload fast?
				sendEvent = "reloadStart";
			}
		}
		break;

		case "recoilStart"_h:
		{
			if (combatData && combatData->ignoreRecoil) {

				sendEvent = "";
				combatData->ignoreRecoil = false;
			}
		}
		break;

		case "blockStart"_h:
		{
			if (combatData->blockControl.TryRaise() == false)
				sendEvent = "";
				//return;//This actually works just fine funny enough.
			
		}
		break;

		case "blockStop"_h:
		{
			bool result;
			if (!actor->GetGraphVariableBool("isBashing", result) || !result)
				combatData->blockControl.LowerGuard();

		}
		break;
		/*
		case "attackStop"_h:
		{
			bool result;
			if (!actor->GetGraphVariableBool("isBashing", result) || !result)
				combatData->blockControl.LowerGuard();
		}
		break;
		//*/

		default:
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
				else if (attackData->data.flags.all(AttackDataFlags::kBashAttack) == true)
				{
					combatData->blockControl.TryBashingBlock(actor);
				}
				//Make a function in melee controller for this.
				else if (combatData && combatData->meleeCombo.IsComboActive() && actor->GetActorValue(RE::ActorValue::kStamina) <= 0) {
					sendEvent = "";
				}
			}
		}
		break;

		}
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

		if (sendEvent)
		{
			const RE::BSFixedString sending = sendEvent;
			return _NotifyAnimationGraph(a_this, sending);
		}

		return _NotifyAnimationGraph(a_this, a_eventName);
	}


	/// <summary>
	/// This exists for npcs rather than playable characters.
	/// </summary>
	/// <param name="a_this"></param>
	/// <param name="a_event"></param>
	/// <param name="a_dispatcher"></param>
	EventResult CrucibleHook::ProcessPlayerAnim(AnimSink* a_this, const AnimEvent* a_event, AnimSource* a_dispatcher)
	{
		//RE::DebugNotification(a_event->tag.c_str());
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
		//RE::DebugNotification(a_event->tag.c_str());
		ProcessAnimEvent(a_this, a_event, a_dispatcher);
		return _ProcessCharacterAnim(a_this, a_event, a_dispatcher);
	}

	//Used to be int32
	uMagicCastState CrucibleHook::Func29(RE::ActorMagicCaster* a_this, void* a2, float delta)
	{
		
		CombatData* combatData = CombatData::GetDataRecord(a_this->actor);

		if (!combatData)
			return _Func29(a_this, a2, delta);

		auto castData = combatData->magicControl.GetCasterData(a_this);

		if (!a_this->currentSpell && (!castData || castData->previousState == uMagicCastState::kNone) == true)
			return _Func29(a_this, a2, delta);


		//if instant, return. Me no care.
		bool isRightHand = a_this->GetCastingSource() == uCastingSource::kRightHand;

		RE::ActorValue av = a_this->currentSpell ? a_this->currentSpell->GetActorValueForCost(isRightHand) : RE::ActorValue::kNone;

		//RE::DebugNotification(std::format("av {}", (int)av).c_str());

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
					RE::DebugMessageBox(std::format("{} build, {} amount", build, amount).c_str());
				amount += build;
				build = 0;
				break;
			case 6://Concentrate
				amount += delta * a_this->currentSpellCost;
				RE::DebugNotification(std::format("{} build, {} amount", build, amount).c_str());
				break;
			default:
				break;
			}
		}
		//*/

		return result;
	}
}