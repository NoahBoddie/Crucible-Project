#pragma once

#include "Timer.h"
#include "Utility.h"
#include "SerializablePair.h"
#include "SerializableMap.h"
#include "SimpleSerializable.h"
#include "ComplexSerializable.h"
#include "CombatDataBehaviour.h"

#include "QueryContext.h"
#include "ModSettingHandler.h"
#include "EventObject.h"
#include "EventSystem.h"

#include "SerialComponent.h"

#include "StrikeEnumerations.h"

namespace Crucible
{
	class CombatData;

	enum CrowdControlType
	{
		kNone,
		kStagger,
		kRecoil,
		kLargeRecoil
	};


	using StrikeResistType = StrikeCategory;

	using DelayStrikeType = std::uint8_t;

	class IAttackInfo;
	class DodgeInfo;
	class HitInfo;

#define minorFlinch 0.00f
#define majorFlinch 0.25f
#define minorStagger 0.50f
#define majorStagger 0.75f
#define fatalStagger 1.00f

//#define None 0
//#define Stagger 1
//#define Recoil 2
//#define LargeRecoil 3




	struct StrikeData
	{
		//empty for now, stuff coming s
		StrikeCategory minorFlags;//dodge obviously does nothing for this.
		StrikeCategory majorFlags;
	};



	class SpecialStrikeController : // public SerialComponent<StrikeData1, StrikeData2>
		public IUpdateBehaviour,
		public SerialComponent<StrikeData>
		//public ComplexSerializable<StrikeData1>,
		//public SimpleSerializable<StrikeData2>
	{
	private:
		//Turn Serializable Map into a serializable vector, size is consistent, look up is fast and I never remove anything so it makes the most sense.
		using ResistMap = SerializableMap<StrikeResistType, SerializablePair<float, Timer>>;
	
	protected:
		ResistMap resistMap;
		TimeStamp lastStagger;

		//*

		//Include these as settings, perhaps associated with the strike thresholds
		static inline constexpr float k_temp_ClimbTimer = 10.f;//Remember, most actions will last a while, so best to set this high
		static inline constexpr float k_temp_StrikeResistClimb_EvdDef = 2.f;//This may have to vary, move to mod settings I guess?
		static inline constexpr float k_temp_StrikeResistClimb_ResAtk = 1.5f;
		static inline constexpr float k_temp_StrikeResistClimb_EvdAtk = 1.f;
		static inline constexpr float k_temp_StrikeResistClimb_ResDef = 3.f;
		static inline constexpr float k_temp_StaggerStrikeResist_ResAtk = 1.75f;//Only one of it's kind for right now
		static inline constexpr float k_temp_StaggerCooldown = 2.f;
		static inline constexpr float k_temp_PoiseMult = 0.85f;//0 percent mult will take this much stagger cooldown time with it.

		//I want to hook this up to pure animation events like stagger stop. 
		// The main reason being, I want to do this is so a small value is respected the same way across the board.
		// So, I would like an animation event function. But not for notifies which can change, just animation events.
		// it's use will be sparse.
		//bool _wantsToResist = false;
		

	public:

		IAttackInfo* flag_AttackInfo = nullptr;

		//The stagger resist cooldown should only start AFTER you're stagger resist has subsided.
		// Make an event for this, one for cancelling it, and another for starting it.
		void StartStaggerResist()
		{
			//Utility::DebugNotification("resist started", true);
			lastStagger.Start();
		}

		void ClearStaggerResistance()
		{
			if (lastStagger.IsActive() == true)
				Utility::DebugNotification("stagger resist fades", IsPlayerRef() == true);

			lastStagger.Clear();
		}




		void IncrementStrikeResistance(StrikeResistType resist, bool force = false, bool reapply_pause = false);

		



		bool IsInStaggerCooldown();

		//This represents different things in different contexts.
		// If the person being strike tested is the player, it should be using the one on them,
		// but if its an npcs, it needs to test how many times the enemy has done that effect.
		// making it so the player is not effected or thrown off by how many strikes have been
		// levied against this person. Objects, just straight up will not increment.
		float GetStrikeResist(StrikeResistType resist, RE::Actor* actor = nullptr)
		{


			

			int stagger_strike_resist = 0;

			if (1!=1 && resist == StrikeResistType::kResistAttack)
			{
				actor = actor ? actor : GetActorOwner();
				//stagger_strike_resist = actor->actorState2.staggered || actor->actorState2.recoil ? k_temp_StaggerStrikeResist_ResAtk : 0;

				bool boolean = false;

				if (actor->GetGraphVariableBool("IsStaggering", boolean) && boolean || actor->GetGraphVariableBool("IsRecoiling", boolean) && boolean)
				{
					stagger_strike_resist = k_temp_StaggerStrikeResist_ResAtk;
				}

				//if (stagger_strike_resist)
				//	Utility::DebugMessageBox("adding resist. . .", true);
			}

			return resistMap[resist].first + stagger_strike_resist;
		}


	private:
		//While the stagger all these actions produce aren't on a cooldown, they should get progressively harder.
		// The only one of these actions that are on a cooldown should be minor stagger.
		//The major actions will have climbing thresholds instead.

//I wish to go easy on this, I will make this stagger large, but it won't be so large immediately.
#define fullStaggerMagnitude 0.75f

		//Notice, all these really only need 1 guy to be taken, if I need more I'll just pull them from the context.
		//Also, all of these are basically to be done ON the person it's calling from. So change that bit please.

		bool CheckActionRestrictions(RE::Actor* target, StrikeResult& restrictFlags, bool is_resist, bool& absolute);
		bool CheckActionRestrictions(RE::Actor* target, StrikeResult& restrictFlags, bool resist);

		//Minor Actions
		// All results from minor actions (in terms of stagger) can be effected by stagger controls so note that.
		// On the other hand, results from major ones are hard cast and absolute.
		//Difference in these should be a value from 0-1, being the climb to a major action.
		// In a way, this makes it so stacking resistance for major actions affects the little ones.
		void HandleMinor_Flinch(QueryContext& context, HitInfo& info);
		void HandleMinor_Dodge(QueryContext& context, DodgeInfo& info);
		void HandleMinor_Dodge_Aggressor(QueryContext& context, DodgeInfo& info);
		void HandleMinor_Glancing(QueryContext& context, HitInfo& info);
		void HandleMinor_Glancing_Aggressor(QueryContext& context, HitInfo& info);
		void HandleMinor_EvadeHit(QueryContext& context, DodgeInfo& info);
		void HandleMinor_BashPunish(QueryContext& context, IAttackInfo& info);
		void HandleMinor_BlockPunish(QueryContext& context, IAttackInfo& info);


		//Major Actions
		// Since major actions don't have a cap like minor do, the difference is a value that determines the severity
		// of these actions.
		void HandleMajor_Stunning(QueryContext& contex, HitInfo& info);
		void HandleMajor_Stumbling(QueryContext& context, DodgeInfo& info);
		void HandleMajor_Stumbling_Aggressor(QueryContext& context, DodgeInfo& info);
		void HandleMajor_Retorting(QueryContext& context, HitInfo& info);
		void HandleMajor_Retorting_Aggressor(QueryContext& context, HitInfo& info);
		void HandleMajor_EvadeHit(QueryContext& context, DodgeInfo& info);
		void HandleMajor_BashPunish(QueryContext& context, IAttackInfo& info);
		void HandleMajor_BlockPunish(QueryContext& context, IAttackInfo& info);



		void HandleStrike(StrikeCategory category, bool major, QueryContext& context, CombatData* aggressor, IAttackInfo& info);



		void FlagStagger(StrikeCategory category, bool major)
		{
#ifdef not_estimated_for_use

			auto actor = GetActorOwner();


			if (!actor)
				return;

			if (!(actor->GetGraphVariableBool("IsStaggering", boolean) && boolean) == true ||
				!(actor->GetGraphVariableBool("IsRecoiling", boolean) && boolean) == true)
			{
				return;//This should not do anything
			}

#endif
			auto& flag = major ? data.majorFlags : data.minorFlags;

			flag |= category;
			//Utility::DebugNotification(std::format("+{}", (int)flag), true);
		}

		//static void HandleAction() {}
	public:

		//These both will need their own sets of settings.
		// They will be
		// minorThresh, majorThresh, one for attack, one for defend.

		//I'm thinking the resulting object should be different from this. 
		//Instead, it should be a struct that includes the result, but also the values of each individual 
		// so I don't have to call it again (because factors WILL change) and it makes accumulative values easier to store.
		// For stuff like base poise and such that can be gotten before if you need it.

		StrikeResult ProcessDodge(QueryContext& context, RE::TESObjectREFR* attacker, StrikeResult restrictFlags, DodgeInfo& dodge_info);

		StrikeResult ProcessHit(QueryContext& context, RE::TESObjectREFR* attacker, StrikeResult restrictFlags, HitInfo& hit_info);

		void OnUpdate(RE::Actor* owner, float updateDelta) override
		{

			if (delayPayload)
			{
				if (owner->NotifyAnimationGraph(delayPayload) == true) {
					delayPayload = nullptr;
					//Utility::DebugMessageBox(std::format("{} repeat", actionType), true);//owner->IsPlayerRef()
				}
				else
				{
					delayTimer -= updateDelta;
					if (delayTimer <= 0)
					{
						delayTimer = 0;
						delayPayload = nullptr;
						//Utility::DebugMessageBox("no redo", true);
					}
				}
			}

			
#ifdef enum_version
			if (delayType != CrowdControlType::kNone)
			{
				const char* actionType = nullptr;

				switch (delayType)
				{
				case CrowdControlType::kStagger:
					actionType = "staggerStart";
					break;
				case CrowdControlType::kRecoil:
					actionType = "recoilStart";
					break;
				case CrowdControlType::kLargeRecoil:
					actionType = "recoilLargeStart";
					break;
				}


				if (owner->NotifyAnimationGraph(actionType) == true) {
					delayType = CrowdControlType::kNone;
					//Utility::DebugMessageBox(std::format("{} repeat", actionType), true);//owner->IsPlayerRef()
				}
				else
				{
					delayTimer -= updateDelta;
					if (delayTimer <= 0)
					{
						delayTimer = 0;
						delayType = CrowdControlType::kNone;
						//Utility::DebugMessageBox("no redo", true);
					}
				}
			}
#endif
#ifdef old_version
			if (delayValue != None)
			{
				const char* actionType = delayValue == Stagger ? 
					"staggerStart" : Recoil ? 
					"recoilStart" : "recoilLargeStart";

				//owner->SetGraphVariableFloat("staggerMagnitude", 1);

				if (owner->NotifyAnimationGraph(actionType) == true) {
					delayValue = None;
					//Utility::DebugMessageBox(std::format("{} repeat", actionType), true);//owner->IsPlayerRef()
				}
				else
				{
					delayTimer -= updateDelta;
					if (delayTimer <= 0)
					{
						delayTimer = 0;
						delayValue = None;
						//Utility::DebugMessageBox("no redo", true);
					}
				}
			}
#endif
		}



		void OnTimerFinish(Timer* timer_source, float total_time)
		{
//#define show true
			
			SerializablePair<float, Timer>* pair_ptr = nullptr;

			StrikeResistType resist_type = StrikeResistType::kNone;

			for (int i = 1; !pair_ptr && i <= (int)StrikeResistType::kEvadeAttack; i <<= 1)
			{
				resist_type = static_cast<StrikeResistType>(i);

				auto& test_pair = resistMap[resist_type];

				if (timer_source == &test_pair.second)
					pair_ptr = &test_pair;
			}

			if (!pair_ptr) {
				//Error
				return;
			}

			auto& pair = *pair_ptr;

			float climb = 0;
			std::string show_string;

			switch (resist_type)
			{
			case StrikeResistType::kEvadeDefend:
				climb = k_temp_StrikeResistClimb_EvdDef;
				show_string = "M1";
				break;

			case StrikeResistType::kResistAttack:
				climb = k_temp_StrikeResistClimb_ResAtk;
				show_string = "M2";
				break;

			case StrikeResistType::kEvadeAttack:
				climb = k_temp_StrikeResistClimb_EvdAtk;
				show_string = "M3";
				break;

			case StrikeResistType::kResistDefend:
				climb = k_temp_StrikeResistClimb_ResDef;
				show_string = "M4";
				break;

			}

			float old = pair.first;
			pair.first -= pair.first < climb ? pair.first : climb;
			if (pair.first != 0) {
				pair.second.Start(k_temp_ClimbTimer);
			}

			//Utility::DebugMessageBox(std::format("old: {}, new: {}, show: {}", old, pair.first, show_string), IsPlayerRef());
		};


		//bool _temp_StaggerNextTime = false;
		//DelayStrikeType delayValue = None;
		const char* delayPayload = nullptr;
		float prevMagnitude;
		float delayTimer = 0;
	protected:
#ifdef old_start_stagger
		void StartStagger(RE::TESObjectREFR* from, RE::Actor* to = nullptr, float stagger_time = 0, bool large = false, bool is_recoil = false)
		{
			//0 is a literal flinch, an interupt for lack of a better word.


#define smallStaggerMagnitude 0.25f
#define largeStaggerMagnitude 1//play with this being 0.75 or smth, I believe those are the thresholds


#define maxDelay 0.35f;
			//A stagger time of 0 or lower does not care about time, it will finish the stagger.
			//Note, over time, this should do player stagger on player instead of the npc one, disabling the player movement if I cant
			
			//thresholds are as follows, and as such, this is what each are allowed to do. 0, .25, .50, .75, and 1.
			// SO this thing will indeed send a number, but will not do entry points, cause that's used for other stuff too.

			to = to ? to : GetActorOwner();
			
			if (!to)
				return;

			float direction = 0;

			if (from) {
				//Do stuff and set the direction. Don't want to here, because I'm fucking lazy
			}
			
			float mag = large ? largeStaggerMagnitude : smallStaggerMagnitude;

			const char* magnitudeType = nullptr;
			const char* actionType = nullptr;
			const char* stopType = nullptr;
			//const char* checkType = nullptr;
			DelayStrikeType delayType = None;
			
			if (is_recoil)
			{
				//magnitudeType = "recoilMagnitude";//Does fucking nothing.
				actionType = large ? "recoilLargeStart" : "recoilStart";
				//checkType = "IsRecoiling";
				delayType = large ? LargeRecoil : Recoil;//I actually think this never wants to play again should someone be staggering.
			}
			else{
				magnitudeType = "staggerMagnitude";
				actionType = "staggerStart";
				//checkType = "IsStaggering";
				delayType = Stagger;//Instead, why not just use string pointers? Easier

				to->SetGraphVariableFloat("staggerDirection", direction);

			}

			bool boolean = false;

			//Move to using actor states instead.
			if (to->GetGraphVariableBool("IsStaggering", boolean) && boolean){
				stopType = "staggerStop";
			}
			else if (to->GetGraphVariableBool("IsRecoiling", boolean) && boolean){
				stopType = "recoilStop";
			}

			if (magnitudeType)
				to->SetGraphVariableFloat(magnitudeType, mag);

			if (stopType && delayType){//to->GetGraphVariableBool(checkType, boolean) && boolean) {
				delayValue = delayType;
				delayTimer = maxDelay;
				to->NotifyAnimationGraph(stopType);//Needs a flag to prevent stuff from firing off.
			}
			
			//No idea why this being like this works, but fuck it, I guess.
			to->NotifyAnimationGraph(actionType);
		}
#endif	


		int GetStaggerLevel(float magnitude)
		{
			auto base_value = floorf((magnitude / 0.25f));// + 1);
			return Utility::iClamp(base_value, 0, 5);
		}

		float MergeStaggerPower(float current_stagger, float previous_stagger, CrowdControlType& type)
		{
			switch (type)
			{
			case CrowdControlType::kLargeRecoil:
				current_stagger = 0.25f;
				break;
			case CrowdControlType::kRecoil:
				current_stagger = 0.0f;
				break;
			}

			//I'm tired and lazy giv
			//auto curr_pow = GetStaggerLevel(current_stagger) * 0.25f;
			auto prev_pow = GetStaggerLevel(previous_stagger);

			auto new_stagger = Utility::Lerp(current_stagger, previous_stagger, 0.5f);
			

			
			
			auto new_pow = GetStaggerLevel(new_stagger);

			if (new_pow == prev_pow - 1)
			{
				if (prev_pow < 4){//Only promotes if the stagger is weak.
					new_stagger = previous_stagger;
					new_pow = prev_pow;
				}
			}

			switch (type)
			{
			case CrowdControlType::kLargeRecoil:
				if (new_pow == 1) {
					type = CrowdControlType::kRecoil;//Unlikely to occur, but demoted large recoils become recoils
				}
				//fall through, we want to do the stagger test as well.

			case CrowdControlType::kRecoil:
				if (new_pow == 2)
					type = CrowdControlType::kLargeRecoil;//If 2, timed recoil is what it should become.
				else if (new_pow >= 3)
					type = CrowdControlType::kStagger;//If its this high, it gets promoted to a stagger.
				break;
			}

			return new_stagger;
		}


		void StartStagger(CrowdControlType type, RE::TESObjectREFR* from, RE::Actor* to = nullptr, float stagger_time = 0, float magnitude = 0, bool use_delay = true)
		{
#define maxDelay() 0.35f;
			//A stagger time of 0 or lower does not care about time, it will finish the stagger.
			//Note, over time, this should do player stagger on player instead of the npc one, disabling the player movement if I cant

			//thresholds are as follows, and as such, this is what each are allowed to do. 0, .25, .50, .75, and 1.
			// SO this thing will indeed send a number, but will not do entry points, cause that's used for other stuff too.

			to = to ? to : GetActorOwner();

			if (!to && type == CrowdControlType::kNone)
				return;

			float direction = 0;

			if (from) {
				//Do stuff and set the direction. Don't want to here, because I'm fucking lazy
			}

			const char* magnitudeType = nullptr;
			const char* actionType = nullptr;
			const char* stopType = nullptr;
			//const char* checkType = nullptr;
			//DelayStrikeType delayType = None;
			

			bool boolean = false;

			//Move to using actor states instead.
			if (to->GetGraphVariableBool("IsStaggering", boolean) && boolean) {
				stopType = "staggerStop";
			}
			else if (to->GetGraphVariableBool("IsRecoiling", boolean) && boolean) {
				stopType = "recoilStop";
			}
			



			if (stopType && use_delay) {//to->GetGraphVariableBool(checkType, boolean) && boolean) {
				//delayValue = delayType;
				float prev_mag = prevMagnitude;//0;
				//temp, needs to account for recoil, instead, we make a serializable thing that will store it here.
				// On once of the events starts we take note of the stagger magnitude, OR the stagger type and put the stagger there
				// ourselves.
				//to->GetGraphVariableFloat("staggerMagnitude", prev_mag);

				if (magnitude < prev_mag)
					magnitude = MergeStaggerPower(magnitude, prev_mag, type);
			}


			switch (type)
			{
			case CrowdControlType::kStagger:
				magnitudeType = "staggerMagnitude";
				actionType = "staggerStart";
				to->SetGraphVariableFloat("staggerDirection", direction);
				break;

			case CrowdControlType::kRecoil:
				//magnitudeType = "recoilMagnitude";//Does fucking nothing.
				actionType = "recoilStart";
				break;

			case CrowdControlType::kLargeRecoil:
				//magnitudeType = "recoilMagnitude";//Does fucking nothing.
				actionType = "recoilLargeStart";
				break;

			}

			if (stopType && use_delay) {//to->GetGraphVariableBool(checkType, boolean) && boolean) {
				delayPayload = actionType;
				delayTimer = maxDelay();
				to->NotifyAnimationGraph(stopType);//Needs a flag to prevent stuff from firing off.
			}



			if (magnitudeType)
				to->SetGraphVariableFloat(magnitudeType, magnitude);

			//No idea why this being like this works, but fuck it, I guess.
			to->NotifyAnimationGraph(actionType);
		}
		
#ifdef old_cc_start_stagger
		void StartStagger(CrowdControlType type, RE::TESObjectREFR* from, RE::Actor* to = nullptr, float stagger_time = 0, float magnitude = 0, bool use_delay = true)
		{
#define maxDelay 0.35f;
			//A stagger time of 0 or lower does not care about time, it will finish the stagger.
			//Note, over time, this should do player stagger on player instead of the npc one, disabling the player movement if I cant

			//thresholds are as follows, and as such, this is what each are allowed to do. 0, .25, .50, .75, and 1.
			// SO this thing will indeed send a number, but will not do entry points, cause that's used for other stuff too.

			to = to ? to : GetActorOwner();

			if (!to)
				return;

			float direction = 0;

			if (from) {
				//Do stuff and set the direction. Don't want to here, because I'm fucking lazy
			}

			const char* magnitudeType = nullptr;
			const char* actionType = nullptr;
			const char* stopType = nullptr;
			//const char* checkType = nullptr;
			//DelayStrikeType delayType = None;

			switch (type)
			{
			case CrowdControlType::kNone:
				return;

			case CrowdControlType::kStagger:
				magnitudeType = "staggerMagnitude";
				actionType = "staggerStart";
				to->SetGraphVariableFloat("staggerDirection", direction);
				break;

			case CrowdControlType::kRecoil:
				//magnitudeType = "recoilMagnitude";//Does fucking nothing.
				actionType = "recoilStart";
				break;

			case CrowdControlType::kLargeRecoil:
				//magnitudeType = "recoilMagnitude";//Does fucking nothing.
				actionType = "recoilLargeStart";
				break;

			}


			bool boolean = false;

			//Move to using actor states instead.
			if (to->GetGraphVariableBool("IsStaggering", boolean) && boolean) {
				stopType = "staggerStop";
			}
			else if (to->GetGraphVariableBool("IsRecoiling", boolean) && boolean) {
				stopType = "recoilStop";
			}

			if (magnitudeType)
				to->SetGraphVariableFloat(magnitudeType, magnitude);

			if (stopType && use_delay) {//to->GetGraphVariableBool(checkType, boolean) && boolean) {
				//delayValue = delayType;
				delayPayload = actionType;
				delayTimer = maxDelay;
				to->NotifyAnimationGraph(stopType);//Needs a flag to prevent stuff from firing off.
			}

			//No idea why this being like this works, but fuck it, I guess.
			to->NotifyAnimationGraph(actionType);
		}
#endif




		void Initialize() override;


		void OnStaggerStart(std::string eventName)
		{
			//Utility::DebugMessageBox("start", true);
			//std::string eventName;
			
			//A legacy query, pay it no mind.
			//if (parameters[0].AsString(eventName) == true) {}
			
			switch (hash(eventName))
			{
			case "recoilStart"_h:
				prevMagnitude = 0.f;
				break;
			case "recoilLargeStart"_h:
				prevMagnitude = 0.25f;
				break;
			case "staggerStart"_h:
				GetActorOwner()->GetGraphVariableFloat("staggerMagnitude", prevMagnitude);
				break;
			}


			for (int index = 1; index > (int)StrikeCategory::kEvadeAttack; index <<= 1)
			{
				StrikeCategory category = static_cast<StrikeCategory>(index);
				
				auto& resistTimer = resistMap[category].second;

				//if (resistTimer.isActive() == true)
				resistTimer.Pause();
			}
		}

		void OnStaggerStop(std::string event_name)
		{
			if (delayPayload){//delayValue != None) {
				//Utility::DebugMessageBox(std::format("intercept stop {}", delayValue), true);
				return;
			}

			if ((data.minorFlags & StrikeCategory::kResistAttack) == StrikeCategory::kResistAttack)
				StartStaggerResist();

			for (int index = 1; index <= (int)StrikeCategory::kEvadeAttack; index <<= 1)
			{
				StrikeCategory category = static_cast<StrikeCategory>(index);
				//Utility::DebugNotification(std::format("cat {} vs index {} vs flag {}", (int)category, (int)index, (int)data.majorFlags), true);
				if ((data.majorFlags & category) == category)
					IncrementStrikeResistance(category);
				else
					resistMap[category].second.Resume();
			}

			prevMagnitude = 0;
			
			data.minorFlags = data.majorFlags = StrikeCategory::kNone;
		}

		void OnAction()
		{
			//bool agressiveAction = 
			//parameters[0].AsString(eventName) == true

			ClearStaggerResistance();
		}
		
		void SerializeData(SerialArgument& serializer, bool& success) override
		{
			serializer.Serialize(resistMap);
			serializer.Serialize(lastStagger);
		}
		//*/


	};
	


}