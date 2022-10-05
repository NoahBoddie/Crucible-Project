#include "Utility.h"
#include "Resources.h"
#include "CombatData.h"
#include "WeaponSwing.h"
#include "MotionStatController.h"
namespace Crucible
{
	//This class handles the effect based processes of Crucible. If someone's poisoned they lose resist
	//if invisible they gain evade, that sort of thing. The question is do I want anything to be stored here?
	class EffectManager : public RE::BSTEventSink<EffectEvent>
	{
	public:

		static EffectManager* GetSingleton() 
		{
			static EffectManager singleton;
			return &singleton;
		}


		auto ProcessEvent(const EffectEvent* a_event, EffectSource* a_eventSource)->RE::BSEventNotifyControl override
		{
			return EventResult::kContinue;

			RE::TESObjectREFR* castObj = a_event->target.get();

			//if (castObj->GetFormType() != RE::FormType::ActorCharacter || castObj->IsPlayerRef() == false)
			//	return EventResult::kContinue;
			if (a_event->activeEffectUniqueID == 0)
				Utility::DebugNotification(a_event->isApplied ? ("startNs: " + std::to_string(a_event->pad14)).c_str() : ("endNs: " + std::to_string(a_event->pad14)).c_str());
			
			RE::Actor* castActor = skyrim_cast<RE::Actor*>(castObj);

			RE::ActiveEffect* effect = Utility::GetEffectFromID(castActor, a_event->activeEffectUniqueID);

			//CombatData* cData = CombatData::GetDataRecord(castActor);
			auto baseEffect = effect->GetBaseObject();
			std::string name(baseEffect->GetFullName());
			RE::FormID _id = effect->GetBaseObject()->GetFormID();
			if (effect->spell->GetFormID() == 0x11000D6A)
				Utility::DebugNotification(a_event->isApplied ?  ("start_is: " + name).c_str() : ("end_is" + name).c_str());
			else
				Utility::DebugNotification(a_event->isApplied ? ("start_not: " + name).c_str() : ("end_not: " + name).c_str());


			return EventResult::kContinue;
		}

	};

}