#pragma once

#include "Utility.h"
#include "StatedObject.h"
#include "CombatDataBehaviour.h"

namespace Crucible
{
	enum class AttackState {kNone, kPrehitCalc, kHitCalc, kPosthitCalc };

	class AttackStatController :
		public StatedObject<AttackState, uATTACK_ENUM>,
		public CombatDataBehaviour
	{
	public:
		bool EvaluateState(uATTACK_ENUM attackState) override;


	protected:
		void OnStateBegin() override;
		//void OnStateFinish() override;

		uATTACK_ENUM storedAttackEnum;

		float resistTaken = 0;
	private:
		void HandleChange(AttackState _state, bool assign);
	};

}