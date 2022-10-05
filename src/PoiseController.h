#pragma once
#include "StatedObject.h"
#include "CombatResource.h"
#include "CombatDataBehaviour.h"

namespace Crucible
{
	class EventObject;
	enum class EventControl;
	enum class RecoveryFinishReason;

	enum class PoiseState { Normal, Exposed, Broken, Recovering };

	class PoiseController :
		public CombatDataBehaviour,
		public StatedObject<PoiseState>

	{
	public:

		void DamagePoise(float value, float& regenDamage, bool allowBreak = false);
		

		void OnRecoverStart(int value);

		void OnRecoverFinish(int value, RecoveryFinishReason reason);

		void BreakPoise();

		inline bool IsPoiseBroken() { auto state = GetCurrentState(); return state == PoiseState::Broken || state == PoiseState::Recovering; }

	protected:
		void Initialize() override;
		
		void HandleChange(bool apply);
		void OnStateBegin() override;

		void SerializeData(SerialArgument& serializer, bool& success) override
		{
			//Remains, unimplementedThis was a lie, if the state is in poise handle it please.

			AutoSerialize(serializer);

			if (serializer.IsDeserializing() == true && IsPoiseBroken() == true)
				HandleChange(true);
		}

		float timestamp;
	};
}