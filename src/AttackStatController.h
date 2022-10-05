#pragma once

#include "Utility.h"
#include "StatedObject.h"
#include "SerialConstructor.h"
#include "IUpdateBehaviour.h"
#include "CombatDataBehaviour.h"

namespace Crucible
{
	enum class AttackState {kNone, kPrehitCalc, kHitCalc, kPosthitCalc };

	

	class AttackStatController :
		public IUpdateBehaviour,
		public StatedObject<AttackState>
	{
		

	protected:
		void OnUpdate(RE::Actor* self, float delta) override;
		void OnStateBegin() override;
		//void OnStateFinish() override;
		

		void BeforeCombatGraphEvent(EventArgument& argument, std::string anim_event, RE::BGSAttackData* attack_data);
		//I still wish to keep this, because I will want to use it for replacing power attacks later.
		//void OnCombatIdle(EventArgument& argument, RE::TESIdleForm* idle, RE::Actor* actor, const RE::BGSAttackData* attack_data);


		void SerializeData(SerialArgument& serializer, bool& success) override
		{
			//Note, this shit is likely some level of boilerplay code, given handle change is in a weird situation of only being
			// for on state begin.
			AutoSerialize(serializer);

			serializer.Serialize(storedAttackEnum);
			serializer.Serialize(resistTaken);

			if (serializer.IsDeserializing() == true)
				HandleChange(GetCurrentState(), true);

		}

		void Initialize() override
		{
			//CreateMemberEvent("OnCombatIdle", &AttackStatController::OnCombatIdle);
			CreateMemberEvent("BeforeCombatGraphEvent", &AttackStatController::BeforeCombatGraphEvent);


			//TestType(&AttackStatController::OnCombatIdle);
		}
		uATTACK_ENUM storedAttackEnum;

		float resistTaken = 0;
	private:
		void HandleChange(AttackState _state, bool assign);
	};

}