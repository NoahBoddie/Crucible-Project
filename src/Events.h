#include "Utility.h"
#include "EffectManager.h"
#include "EquipUtilities.h"


//#include "TESPackageEvent.h"
namespace Crucible
{

	class CrucibleEvent :
		public RE::BSTEventSink<HitEvent>,
		public RE::BSTEventSink<CombatEvent>,
		public RE::BSTEventSink<AnimEvent>,
		public RE::BSTEventSink<REFRLoadEvent>,
		public RE::BSTEventSink<EquipEvent>,
		public RE::BSTEventSink<RE::TESFormDeleteEvent>,
		public RE::BSTEventSink<RE::TESPackageEvent>
	{
	public:
		static CrucibleEvent* GetSingleton();

		static void Install()
		{
			//At a later point I would like to break this thing up into it's own set of respective classes.
			// While still having a patch events here to do the work in this regard.
			//
			auto sourceHolder = RE::ScriptEventSourceHolder::GetSingleton();

			if (!sourceHolder) {
				//report no source
				return;
			}

			auto c_event = GetSingleton();
			//auto effectManager = EffectManager::GetSingleton();//should do the events here.
			if (!c_event){// || !effectManager) {
				//report this thing no exist?
				return;
			}
			
			//sourceHolder->AddEventSink<HitEvent>(c_event);
			//sourceHolder->AddEventSink<RE::TESCombatEvent>(c_event);
			sourceHolder->AddEventSink<REFRLoadEvent>(c_event);
			//sourceHolder->AddEventSink<EffectEvent>(effectManager);
			sourceHolder->AddEventSink<EquipEvent>(c_event);
			sourceHolder->AddEventSink<RE::TESFormDeleteEvent>(c_event);

			//Testing concept
			//sourceHolder->AddEventSink<RE::TESPackageEvent>(c_event);
		}

		
		//*
		//Want to move this to a hook IF I CAN so I can perhaps get and manipulate more information about hit detection.
		EventResult ProcessEvent(const HitEvent* a_event, HitSource* a_eventSource) override;
		EventResult ProcessEvent(const CombatEvent* a_event, CombatSource* a_eventSource) override;
		EventResult ProcessEvent(const AnimEvent* a_event, AnimSource* a_dispatcher) override;
		EventResult ProcessEvent(const REFRLoadEvent* a_event, REFRLoadSource* a_eventSource) override;

		EventResult ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>* a_eventSource) override
		{
			CombatData::RemoveDataRecord(a_event->formID);
			//At a later point, I'm going to want this to remove dodge records of it, and do that before I start
			// making saving stuff for that.
			return EventResult::kContinue;
		}


		EventResult ProcessEvent(const RE::TESPackageEvent* a_event, RE::BSTEventSource<RE::TESPackageEvent>* a_eventSource) override
		{
			if (!a_event || !a_event->target)
				return EventResult::kContinue;


			//if (a_event->unk00->InHighProcess() == false)
			//	return EventResult::kContinue;

			//auto owner = a_event->unk00->currentPackage.target.get();//a_event->unk00->target.get();
			auto owner = a_event->target;

			//if (!owner || std::strcmp(owner->GetName(), "Belethor") != 0)
			//	return EventResult::kContinue;

			auto display = std::format("value: {}, package: {}, state: {}",
				owner->GetName(), a_event->package, (int)a_event->state);//a_event->target->GetName()

			Utility::DebugNotification(display.c_str());

			return EventResult::kContinue;
		}


		EventResult ProcessEvent(const EquipEvent* a_event, EquipSource* a_eventSource) override
		{
			//If its unequip and there's nothing in the node(s) we are looking for, we update here. Else, let it.
			//If it's equip it should prefer this. If it equips, it ALWAYS should assign.


			auto form = RE::TESForm::LookupByID(a_event->baseObject);

			auto target = a_event->actor.get();

			if (!target)
				return EventResult::kContinue;
			
			auto data = CombatData::GetDataRecord(target->GetFormID());

			if (!data)
				return EventResult::kContinue;

			if (!form)
			{
				Utility::DebugMessageBox("form was zero");
				return EventResult::kContinue;
				//EquipUtilities::HandleWeaponEquip(data, form, a_event->equipped);
				//return EventResult::kContinue;
			}

			switch (form->GetFormType())
			{
			case RE::FormType::Weapon:

				EquipUtilities::HandleWeaponEquip(data, form->As<RE::TESObjectWEAP>(), a_event->equipped);
				break;

			case RE::FormType::Armor:
				EquipUtilities::HandleArmorEquip(data, form->As<RE::TESObjectARMO>(), a_event->equipped);

				break;

			case RE::FormType::Scroll:
			case RE::FormType::Spell:
				EquipUtilities::HandleSpellEquip(data, form->As<RE::MagicItem>(), a_event->equipped);
				break;

			}

			
			if (data->IsPlayerRef() && a_event->equipped) {
				std::string display = std::format("[equipChange {} ]: stagger({}), precision({}), resist({}), evade: ({}) poise({})",
					data->GetOwner()->GetName(), data->GetStagger(data->rightHand), data->GetPrecision(data->rightHand), data->GetResist(), data->GetEvade(), data->GetResource(CombatResource::Poise));
				Utility::DebugMessageBox(display, true);
				
				float delay = data->recoverControl.GetRegenDelay((int32_t)CombatResource::Poise, RecoveryType::CombatResource);
				
				logger::info("{}: poise delay {}", target->GetName(), delay);

				if (delay > 20){
					logger::info("resetting poise");
					data->recoverControl.SetRegenDelay((int32_t)CombatResource::Poise, RecoveryType::CombatResource, 10);
				}
			}

			//how this works is if you unequip, it will print all viable slots for that piece of armor, and fire for each
			// If it's a weapon, it will fire on left and right depending on on which it was.

			return EventResult::kContinue;
		}


		//*/
		//*
	protected:
		CrucibleEvent() = default;
		CrucibleEvent(const CrucibleEvent&) = delete;
		CrucibleEvent(CrucibleEvent&&) = delete;
		virtual ~CrucibleEvent() = default;

		auto operator=(const CrucibleEvent&)->CrucibleEvent & = delete;
		auto operator=(CrucibleEvent&&)->CrucibleEvent & = delete;
		//*/
	};


}