#pragma once
#include "Events.h"
#include "CombatData.h"
#include "Utility.h"

#include "QueryContext.h"

#include "SpecialStrikeController.h"//needed?

namespace Crucible
{
	CrucibleEvent* CrucibleEvent::GetSingleton()
	{
		static CrucibleEvent singleton;
		return &singleton;
	}


	//*
	//Want to move this to a hook IF I CAN so I can perhaps get and manipulate more information about hit detection.
	EventResult CrucibleEvent::ProcessEvent(const HitEvent* a_event, HitSource* a_eventSource)
	{
		return EventResult::kContinue;

		//Was gonna make the hit stuff for real.
		//Todo: May not be an actor. For something like this, we have to question if we want stagger on traps and such
		auto attacker = a_event->cause.get();
		auto target = a_event->target.get();
		auto source = RE::TESForm::LookupByID(a_event->source);

		//if (source && source->GetFormType() == RE::FormType::Explosion)
			//Utility::DebugNotification(std::format("name: {} , formID {}", source->GetName(), source->GetFormID()).c_str());

		if (!attacker || !target || attacker == target)
			return EventResult::kContinue;	//later, this is gonna need to evaluate regardless of target.
											// that will be an object, attached to or generated from a source.

		//Might even let this go with no attacker actually.
		if (!attacker || target->IsNot(RE::FormType::ActorCharacter) == true)
			return EventResult::kContinue;

		

		CombatData* targetData = CombatData::GetDataRecord(target->GetFormID());

		auto ignoreStrike = targetData->temp_IgnoreHitStrike;
		//Easier to purge this here, close to the border so might as well. Might be better to place in hooks though.
		targetData->temp_IgnoreHitStrike = false;

		if (!targetData || targetData->temp_IgnoreHitStrike)
			return EventResult::kContinue;

		auto targetActor = target->As<RE::Actor>();


		//Kill move hits, nor hits on dead people ever count.
		if (targetActor->IsInKillMove() == true || targetActor->IsDead() == true)
			return EventResult::kContinue;

		bool isBash = a_event->flags.any(HitEventFlag::kBashAttack);
		bool isPower = a_event->flags.any(HitEventFlag::kPowerAttack);
		bool isBlocked = a_event->flags.any(HitEventFlag::kHitBlocked);
		//bool isSneak = a_event->flags.any(HitEventFlag::kSneakAttack);//I would like sneak attacks to have more precision
																		// I think it would make them not get glancing.
																		// or maybe sneak attacks should never be glancing?
		
		//Critical hits might boost resist(did I mean precision?) a little, maybe a mult or something like that.
		QueryContext context;

		context.SetForm(querySource, source);
		
		//context.SetForm(queryAgressor, attacker);
		//context.SetForm(queryDefender, targetActor);

		context.SetBool(queryBash, isBash);
		context.SetBool(queryBlocked, isBlocked);
		context.SetBool(queryPowerAction, isPower);

		//targetData->strikeControl.ProcessHit(context, attacker);

		
		CombatData* attackerData = CombatData::GetDataRecord(attacker->GetFormID());

		if (attackerData)
		{
			//Need to move this shit to melee hit so it can stop fucking me in the ass.
			RE::TESObjectWEAP* weaponSource = context.GetForm<RE::TESObjectWEAP>(querySource);
			//Even if it's nothing, that's a melee attack.
			switch (Utility::GetWeaponType(weaponSource))
			{
			case WeaponType::kBow:
			case WeaponType::kCrossbow:
			case WeaponType::kStaff:
				break;
			default:
				//Make this an event instead plz k thankies
				attackerData->meleeCombo.TrySuccess(weaponSource);
				//attackerData->meleeCombo.IncrementHit();
				break;
			}
		}

		return EventResult::kContinue;
	}

	EventResult CrucibleEvent::ProcessEvent(const CombatEvent* a_event, CombatSource* a_eventSource)
	{
		//Delete me already
		return RE::BSEventNotifyControl::kContinue;
	}

	EventResult CrucibleEvent::ProcessEvent(const AnimEvent* a_event, AnimSource* a_dispatcher)
	{
		


		//Utility::DebugNotification(a_event->tag.c_str());
		return RE::BSEventNotifyControl::kContinue;
	}


	EventResult CrucibleEvent::ProcessEvent(const REFRLoadEvent* a_event, REFRLoadSource* a_eventSource)
	{
		if (a_event->loaded) {
			//RE::TESForm* loadedForm = RE::TESForm::LookupByID(a_event->formID);
			
			//if (loadedForm && !loadedForm->IsPlayerRef() && loadedForm->GetFormType() == RE::FormType::ActorCharacter) {
			//	Utility::DebugNotification(std::format("{}, {}", loadedForm->GetFormID()).c_str(), std::to_string(CombatData::AddDataRecord(a_event->formID)).c_str());
			//}
			//Gathered data for later, the main issue is the add function. It seems to only work for the player, and does absolutely nothing
			// for anyone else. It's not that they don't show up either, it just doesn't register, and I have no idea why.
			
			return EventResult::kContinue;
			//Kinda don't need the below.

			RE::Actor* actor = RE::TESForm::LookupByID<RE::Actor>(a_event->formID);

			if (!actor || CombatData::AddDataRecord(actor) == nullptr)
			{
				Utility::DebugNotification(std::format("{} did not load", actor->GetFormID()));
			}
		}
		else {
			CombatData::RemoveDataRecord(a_event->formID);
			//Scream when this happens.
		}

		return EventResult::kContinue;

	}


}