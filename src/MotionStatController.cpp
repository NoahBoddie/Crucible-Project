#pragma once
#include "MotionStatController.h"
#include "CombatData.h"
#include "Utility.h"

#include "ActionPointController.h"



namespace Crucible
{
	void MotionStatController::OnUpdate(RE::Actor* a_owner, float delta)
	{//Can be override, otherwise does nothing.
		//You can toss, ALL this processing, it's no longer needed.
		//There should be a player specific version that measures joystick inputs and/or if always walk is enabled.
		

		if (!a_owner || a_owner->Is3DLoaded() == false)
			return;
		/*
		RE::NiPoint3 velocity;

		a_owner->GetLinearVelocity(velocity);

		float speed = velocity.Length();

		MovementState currState = GetCurrentState();

		if (a_owner->actorState1.sneaking > 0)
		{
			if (currState != MovementState::kSneaking)
			{
				if (owner->actionPointControl.StartDodge(ActionPoint::genericDodge) == false)
				{
					Utility::DebugNotification("dodge failed");
				}
				else
				{
					Utility::DebugNotification(std::format("dodge_start, cooldown {}", owner->actionPointControl.GetGeneralCooldown()), true);
				}
			}

			return currState != GoToState(MovementState::kSneaking);
		}

		if (speed < 1) {
			//Utility::DebugNotification(std::format("still end {}", speed).c_str());
			return currState != GoToState(MovementState::kStill);
		}

		bool sprint = false;//Utility::GetGraphVariableBool(graphManager, "IsSprinting");
		a_owner->GetGraphVariableBool("IsSprinting", sprint);

		if (sprint) {
			//Utility::DebugNotification(std::string("sprint end").c_str());
			return currState != GoToState(MovementState::kSprint);
		}




		float direction = 0.f;//Utility::GetGraphVariableFloat(graphManager, "Direction") * 360.f;
		a_owner->GetGraphVariableFloat("Direction", direction);
		direction *= 360.f;


		float walk = 0.f;//Utility::GetGraphVariableFloat(graphManager, "SpeedWalk");
		a_owner->GetGraphVariableFloat("SpeedWalk", walk);

		float run = 0.f;//Utility::GetGraphVariableFloat(graphManager, "SpeedRun");
		a_owner->GetGraphVariableFloat("SpeedRun", run);
		float newDir = direction * pi / 180.f;

		const float xPer = 0.65f;		//This is to reduce the impact of horizontal movement. Makes it so
									// horizontal movement needs a lot more side movement to qualify. Was 0.65

		const float yPer = 0.5f;	//This is similar to xPer, but for y. Means you have to be walking more
									// forward for it to count.


		float x = velocity.x;//sin(newDir);//direction);//
		float y = velocity.y;//cos(newDir);//direction);//
		//Utility::DebugNotification(std::format("{}", y * yPer), true);
		bool vertical = abs(y) > abs(x * xPer);
		//Utility::DebugNotification(std::format("{}, {}", vertical, y), true);

		bool positive = vertical ? y >= 0 : x >= 0;
		bool running = speed >= run * 0.5f;//speed > walk;

		int newState = vertical ? 1 : 2;
		newState += !positive && vertical ? 2 : 0;
		newState += running ? 3 : 0;//This never triggers some how. Rather, it's alats 3?
		Utility::DebugNotification(std::format("x {}, y {}, speed {}", x, y, speed), true);
		return currState != GoToState(static_cast<MovementState>(newState));
		

#ifdef NoUse
		MovementState state; //vertical ? 1 : 2;
		//newState += !positive && vertical ? 2 : 0;
		//newState += running ? 3 : 0;//This never triggers some how. Rather, it's alats 3?
		if (a_owner->actorState1.sprinting > 0)
		{
			state = MovementState::kSprint;
		}
		if (a_owner->actorState1.sneaking > 0)
		{
			state = MovementState::kSneaking;
		}
		else if (a_owner->actorState1.movingForward > 0)
		{
			state = MovementState::kForwardWalk;
		}
		else if (a_owner->actorState1.movingBack > 0)
		{
			state = MovementState::kBackWalk;
		}
		else if (a_owner->actorState1.movingRight > 0 || a_owner->actorState1.movingLeft > 0)
		{
			state = MovementState::kSideWalk;
		}
		else
		{
			state = MovementState::kStill;
		}

		switch (state)
		{
		case MovementState::kForwardWalk:
		case MovementState::kSideWalk:
		case MovementState::kBackWalk:
			if (a_owner->actorState1.running > 0)
			{
				state = static_cast<MovementState>((int)state + 3);
			}
		}

		GoToState(state);

		return true;
#endif
		/*/


		float speed = 0;//Utility::GetGraphVariableFloat(graphManager, "Speed");
		a_owner->GetGraphVariableFloat("Speed", speed);

		MovementState currState = GetCurrentState();
		
		if (a_owner->actorState1.sneaking > 0)
		{
			if (currState != MovementState::kSneaking)
			{
				if (owner->actionPointControl.StartDodge(ActionPoint::genericDodge) == false)
				{
					Utility::DebugNotification("dodge failed", IsPlayerRef());
				}
				else
				{
					Utility::DebugNotification(std::format("dodge_start {}", owner->actionPointControl.GetGeneralCooldown()).c_str(), IsPlayerRef());
				}
			}
			
			GoToState(MovementState::kSneaking);
			return;
		}
		
		if (speed < 1) {
			//Utility::DebugNotification(std::format("still end {}", speed).c_str());
			GoToState(MovementState::kStill);
			return;
		}

		bool sprint = false;//Utility::GetGraphVariableBool(graphManager, "IsSprinting");
		a_owner->GetGraphVariableBool("IsSprinting", sprint);

		if (sprint) {
			//Utility::DebugNotification(std::string("sprint end").c_str());
			GoToState(MovementState::kSprint);
			return;
		}

		
		

		float direction = 0.f;//Utility::GetGraphVariableFloat(graphManager, "Direction") * 360.f;
		a_owner->GetGraphVariableFloat("Direction", direction);
		direction *= 360.f;


		float walk = 0.f;//Utility::GetGraphVariableFloat(graphManager, "SpeedWalk");
		a_owner->GetGraphVariableFloat("SpeedWalk", walk);

		float run = 0.f;//Utility::GetGraphVariableFloat(graphManager, "SpeedRun");
		a_owner->GetGraphVariableFloat("SpeedRun", run);
		float newDir = direction * pi / 180.f;

		const float xPer = 1.0f;	//This is to reduce the impact of horizontal movement. Makes it so
									// horizontal movement needs a lot more side movement to qualify. Was 0.65

		const float yPer = 0.5f;	//This is similar to xPer, but for y. Means you have to be walking more
									// forward for it to count.


		float x = sin(newDir);//direction);//
		float y = cos(newDir);//direction);//
		//Utility::DebugNotification(std::format("{}", y * yPer), true);
		bool vertical = abs(y * yPer) > abs(x * xPer);
		//Utility::DebugNotification(std::format("{}, {}", vertical, y), true);

		bool positive = vertical ? y >= 0 : x >= 0;
		bool running = speed >= run * 0.5f;//speed > walk;//a_owner->actorState1.running;//

		int newState = vertical ? 1 : 2;
		newState += !positive && vertical ? 2 : 0;
		newState += running ? 3 : 0;//This never triggers some how. Rather, it's alats 3?
		
		//if (vertical)
		//	Utility::DebugNotification(std::format("x {}, y {}, speed {}, dir {}, new dir {}", x, y, speed, direction, newDir), IsPlayerRef());
		
		GoToState(static_cast<MovementState>(newState));
		//*/
	}
	//x 1 y 1 //// 
	void MotionStatController::OnStateBegin()
	{
		HandleChange(GetPreviousState(), false);
		HandleChange(GetCurrentState(), true);
		 

		//float stagger = owner->stagger.DeleteLater();
		//float resist = owner->resist.DeleteLater();
		//float evade = owner->evade.DeleteLater();
		//std::string printString = std::format("[ {} ]: resist({}), stagger({}), evade: ({}), state ({})",
		//	GetActorOwner()->GetName(), resist, stagger, evade, (int)GetCurrentState());

		//Utility::DebugNotification(printString.c_str());
		//print if you can.
	}
	
	void MotionStatController::HandleChange(MovementState _state, bool assign)
	{
		std::int8_t mod = assign ? 1 : -1;

		//assign = false;

		//Behaviour on display here is the perfect example of improper functionality. This adds, and fails to remove. So it begs the question
		// of whether it figures out that it's already in there or not.
		// First problem, this thing needs to be using something different. The way this is set up the changes will override themselves.  
		//*
		//When it's just the one, it works. There shouldn't be multiple ids though I'm not sure why any of this is happening.
		switch (_state)
		{
		case MovementState::kForwardWalk:
			//owner->resist %= CombatValueInput('MOV2', ValueInputType::Set, 0.25f, assign);
			//owner->precision %= CombatValueInput('MOV2', ValueInputType::Set, 0.25f, assign);
			break;
		}

		//return;
		static bool once = true;

		switch (_state)
		{
		case MovementState::kStill:
		stillness:
			//break;
			//owner->No idea what stillness did so not gonna do anything on it, for now.
			owner->precision += CombatValueInput('MOV1', ValueInputType::Set, 0.5f, assign);//0.5f * mod;//Increases this a tiny bit, then increases the general mult.
			// 0.25 seems also like a decent number, even lower could be fine
			owner->precision %= CombatValueInput('MOV1', ValueInputType::Set, 0.25f, assign);//0.25f * mod;//Weaker precision can use, but larger precision get more use out of it.
			//owner->precision %= CombatValueInput('MOV1', ValueInputType::Set, 0.25f, false);//0.25f * mod;//Weaker precision can use, but larger precision get more use out of it.
			owner->evade -= CombatValueInput('MOV1', ValueInputType::Set, 1.5f, assign);//1.5f * mod;
			break;

		case MovementState::kSneaking:
			//Place holder mostly, just an increase in precision
			owner->precision += CombatValueInput('MOV2', ValueInputType::Set, 0.5f, assign);//0.5f * mod;
			owner->precision %= CombatValueInput('MOV2', ValueInputType::Set, 0.25f, assign);//0.25f * mod;
			owner->evade -= CombatValueInput('MOV2', ValueInputType::Set, 0.5f, assign);//0.5f * mod;
			goto stillness;


			//Consider walking actions making you less evasive, making using less powerful. 
			// Walking foward would do it more while giving better buffs to resist, and back being the reverse. Remember,
			// small bonuses are fine. There's so much here, they can tip the scales. perc is also useful and valid.
		case MovementState::kForwardWalk:
			owner->resist += CombatValueInput('MOV2', ValueInputType::Set, 1.f, assign);// 1.f * mod;
			owner->resist %= CombatValueInput('MOV2', ValueInputType::Set, 0.25f, assign);//0.25f * mod;//Forward movement gives percent resist for heavy to use as a tool.
			owner->evade -= CombatValueInput('MOV2', ValueInputType::Set, 1.5f, assign);//1.5f * mod;
			goto walking;

		case MovementState::kBackWalk://This is consuming run, not sure why though.
			owner->resist += CombatValueInput('MOV2', ValueInputType::Set, 0.5f, assign);//0.5f * mod;//I will test with this amount, to think on how heavy this impact is.
			owner->evade -= CombatValueInput('MOV2', ValueInputType::Set, 1.f, assign);//1.f * mod;
			goto walking;

		case MovementState::kSideWalk:
		walking:
			owner->resist += CombatValueInput('MOV1', ValueInputType::Set, 1, assign);//1 * mod;
			break;


		case MovementState::kForwardRun:
			owner->stagger += CombatValueInput('MOV2', ValueInputType::Set, 1.5f, assign);//1.5f * mod;
			owner->resist -= CombatValueInput('MOV2', ValueInputType::Set, 2, assign);//2 * mod;
			goto running;

		case MovementState::kBackRun:
			owner->evade += CombatValueInput('MOV2', ValueInputType::Set, 1, assign);//1 * mod;//Might do a percent instead.
			owner->resist -= CombatValueInput('MOV2', ValueInputType::Set, 1.5f, assign);//1.5f * mod;
			owner->stagger -= CombatValueInput('MOV2', ValueInputType::Set, 1.5f, assign);//1.5f * mod;
			goto running;

		case MovementState::kSideRun:
		running:
			owner->evade += CombatValueInput('MOV1', ValueInputType::Set, 2, assign);//2 * mod;
			owner->resist -= CombatValueInput('MOV1', ValueInputType::Set, 1, assign);//1 * mod;//Slight amount of resist damage.
			break;


		case MovementState::kSprint:
			owner->stagger += CombatValueInput('MOV2', ValueInputType::Set, 3.f, assign);//3.f * mod;//2.5f * mod;//
			owner->resist -= CombatValueInput('MOV2', ValueInputType::Set, 2.5f, assign);//2.5f * mod;//2.f * mod;//
			break;
			//case MovementState::kSprint://Burst sprint
			//	owner->stagger += 0.5f * mod;
			//	owner->resist -= 0.5f * mod;
			//	break;
		}
		/*///Second Generation, in method only.
		switch (_state)
		{
		case MovementState::kStill:
			//owner->No idea what stillness did so not gonna do anything on it, for now.
			owner->precision += 0.5f * mod;//Increases this a tiny bit, then increases the general mult.
			// 0.25 seems also like a decent number, even lower could be fine
			owner->precision %= 0.25f * mod;//Weaker precision can use, but larger precision get more use out of it.
			owner->evade -= 1.5f * mod;
			break;
		case MovementState::kSneaking:
			//Place holder mostly, just an increase in precision
			owner->precision += 1.f * mod;
			owner->precision %= 0.5f * mod;
			owner->evade -= 2.f * mod;
			break;

		//Consider walking actions making you less evasive, making using less powerful. 
		// Walking foward would do it more while giving better buffs to resist, and back being the reverse. Remember,
		// small bonuses are fine. There's so much here, they can tip the scales. perc is also useful and valid.
		case MovementState::kForwardWalk:
			owner->resist += 2.f * mod;
			owner->resist %= 0.25f * mod;//Forward movement gives percent resist for heavy to use as a tool.
			owner->evade -= 1.5f * mod;
			break;
		case MovementState::kSideWalk:
			owner->resist += 1 * mod;
			break;
		case MovementState::kBackWalk://This is consuming run, not sure why though.
			owner->resist += 1.5f * mod;//I will test with this amount, to think on how heavy this impact is.
			owner->evade -= 1.f * mod;
			break;


		case MovementState::kForwardRun:
			owner->stagger += 1.5f * mod;
			owner->resist -= 2 * mod;
			owner->evade += 2 * mod;

			break;
		case MovementState::kSideRun:
			owner->evade += 2 * mod;
			break;
		case MovementState::kBackRun:
			owner->evade += 3 * mod;//Might do a percent instead.
			owner->resist -= 1.5f * mod;
			owner->stagger -= 1.5f * mod;
			break;


		case MovementState::kSprint:
			owner->stagger += 3.f * mod;//2.5f * mod;//
			owner->resist -= 2.5f * mod;//2.f * mod;//
		//case MovementState::kSprint://Burst sprint
		//	owner->stagger += 0.5f * mod;
		//	owner->resist -= 0.5f * mod;
		//	break;
		}
		//*/

		/*First Generation
		switch (_state)
		{
		case MovementState::kStill:
			//owner->No idea what stillness did so not gonna do anything on it, for now.
			owner->precision += 2.f * mod;//I'm thinking instead, it will apply a general mult. Maybe, it will apply a bit here.
			owner->evade -= 1.5f * mod;
			break;

		case MovementState::kForwardWalk:
			owner->resist += 2.5f * mod;
			break;
		case MovementState::kSideWalk:
			owner->resist += 1 * mod;
			owner->evade += 0.5f * mod;
			break;
		case MovementState::kBackWalk://This is consuming run, not sure why though.
			owner->evade += 1.5f * mod;
			break;


		case MovementState::kForwardRun:
			owner->stagger += 3.5f * mod;
			owner->resist -= 2 * mod;
			break;
		case MovementState::kSideRun:
			owner->evade += 2 * mod;
			owner->resist += 1.5f * mod;
			owner->stagger -= 2 * mod;
			break;
		case MovementState::kBackRun:
			owner->evade += 4 * mod;
			owner->resist -= 2.5f * mod;
			break;
		case MovementState::kSprint:
			owner->stagger += 4.5f * mod;
			owner->resist -= 3.5f * mod;
			break;
		}
		//*/
	}

}