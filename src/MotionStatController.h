#pragma once
#include "CombatDataBehaviour.h"
#include "IUpdateBehaviour.h"
#include "StatedObject.h"
#include "Utility.h"


namespace Crucible
{
	//Null action, though not yet used should be for actions that I seek to have no input such as swimming and such.
	// A

	enum class MovementState 
	{ 
		kNullAction = -1, 
		kStill, 
		kForwardWalk, 
		kSideWalk, 
		kBackWalk, 
		kForwardRun, 
		kSideRun, 
		kBackRun, 
		kSprint, 
		kSneaking 
	};

	class MotionStatController :
		public IUpdateBehaviour,
		public StatedObject<MovementState>
	{
	public:

	protected:
		void OnStateBegin() override;
		//void OnStateFinish() override;
		void OnUpdate(RE::Actor* a_owner, float delta) override;


		void SerializeData(SerialArgument& serializer, bool& success) override
		{
			//Note, this shit is likely some level of boilerplay code, given handle change is in a weird situation of only being
			// for on state begin.
			AutoSerialize(serializer);

			if (serializer.IsDeserializing() == true)
				HandleChange(GetCurrentState(), true);

		}

		void Build() override { HandleChange(GetCurrentState(), true); }

	private:
		void HandleChange(MovementState _state, bool assign);
	};

}