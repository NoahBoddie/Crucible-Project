#pragma once

#include "CombatDataBehaviour.h"
#include "Timer.h"
#include "Utility.h"
#include "EventSystem.h"
#include "EventObject.h"

namespace Crucible
{
	class MagicCasterData;


	/// <summary>
	/// The value to determine how the combo end should be percieved.
	/// </summary>
	enum class ResolveType { kNormal, kNegative, kPositive, kPositiveBonus, kClear };
	//Normal is normal.
	//Negative returns less gradual, and damages the return a bit. Minimum internally is half.
	//Negative bonus prevents the immediate return, causing a penalty immediately if via
	//Clear voids the combo's benefits.
	//Positive forces the gradual return to give less. I think?
	//PositiveBonus ???



	//Thinking about it, the combo itself should be set up around a timer
	// magic ones need it too.
	template <class... IncrementArgs>
	class Combo : 
		public CombatDataBehaviour,
		public ITimerCallback
	{
	public:

		std::uint8_t count;
		float usedResource;
	protected:
		Timer _comboTimer = Timer::Global();



	public:

		void IncrementCombo(IncrementArgs... args)
		{
			active = true;
			
			if (ShouldComboIncrease(args...) == true)
				count++;
			
			OnComboIncrease(args...);
		}

		void End(ResolveType resolve)
		{
			//First point of issue, this doesn't seem to be protected the below from firing off.
			if (!active)
				return;
		

			//return;


			ReleaseResource();

			OnEnd(resolve);
			count = 0;
			usedResource = 0;
			active = false;
		}


		virtual void StartEndComboWindow() { }
		
		virtual void StopEndComboWindow() { }

		virtual void ExtendEndComboWindow() { }

		virtual float GetComboDropTime() = 0;

		virtual RE::ActorValue GetResourceType() = 0;

		bool IsComboActive() { return count != 0; }

		void OnTimerFinish(float totalTime) override { End(ResolveType::kNormal); }


	protected:
		
		void OnInterrupt(EventObject& params, EventControl& control)
		{
			End(ResolveType::kNegative);
		}

		void Initialize() override 
		{ 
			_comboTimer.AddCallback(this); 
			CreateMemberEvent("OnInterrupt", owner, &Combo::OnInterrupt);
		}

		bool active{ false };

		virtual bool ShouldComboIncrease(IncrementArgs... args) { return true; }
		virtual void OnComboIncrease(IncrementArgs... args) {}

		virtual void OnEnd(ResolveType resolve) {}
		
		virtual void ReleaseResource()
		{
			RE::Actor* actorOwner = GetActorOwner();
			actorOwner->RestoreActorValue(AVModifier::kDamage, GetResourceType(), usedResource);
		}


	};

	class MeleeCombo :
		public Combo<>
	{
		//Things of note
		// *It is unknown how dual wielding will react to the system, especially the hit system, needs testing.

	public:
		std::uint8_t hits;
		float staminaLostRate;
		float climbingValue;


		float powerAttackReduction;//This likely isn't gonna be used here, magic has nothing for it.
		const float k_preemptiveTime = 0.3f;
		const float k_comboDropTime = 1.55f;

		const float posInc = 0.5f;
		const float negInc = 0.5f;
	protected:
		bool _confirmedHit = false;//This is to prevent cleaves from proccing more than needed.

	private:
		//Privates are marked as temp
		float _comboDropTime = 0;
	//Functions
	public:


		RE::ActorValue GetResourceType() override { return RE::ActorValue::kStamina; }

		void StartEndComboWindow() override;

		void ExtendEndComboWindow() override;

		void IncrementHit();

		bool IsInFirstBlow() { return count == 1; }
		
		float GetComboDropTime() override
		{
			if (_comboDropTime == 0)
			{
				if (IsInFirstBlow() == true)
					return 0.3f;
				else
					return 1.55f;
			}

			return _comboDropTime;
		}
		void SetComboDropTime(float value) 
		{
			_comboDropTime = value;
		}
	protected:
		void ReleaseResource() override;
		void OnEnd(ResolveType resolve) override;
		void OnComboIncrease() override;
	};

	class MagicCombo : public Combo<MagicCasterData*>
	{
	//Variables
	public:
		float magickaSpent;
		float& magickaRestore = usedResource;
		float dualCastReduction;
		float dualCastCooldown;

		std::uint8_t input = 0;

	protected:

		const float k_Temp_MagicComboTimer = 2.f;

	//Functions
	public:
		RE::ActorValue GetResourceType() override { return RE::ActorValue::kMagicka; }

		void StartEndComboWindow() override;
		void StopEndComboWindow() override;
		void ExtendEndComboWindow() override;

		//Want to replace this with something more dynamic
		float GetComboDropTime() override { return 2; }
	protected:
		
		void OnEnd(ResolveType resolve) override;

		bool ShouldComboIncrease(MagicCasterData* castData) override;
		
		void OnComboIncrease(MagicCasterData* castData) override;
		
		void ReleaseResource() override;
	};

}