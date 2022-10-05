#pragma once

#include "CombatDataBehaviour.h"
#include "CombatResource.h"
#include "CombatResourceOwner.h"

#include "ModSettingHandler.h"
#include "IRegenSubscriber.h"


#include "SerializableMap.h"
//#include "SerializableObject.h"

#include "Timer.h"

namespace Crucible
{
	class CombatData;

	enum class RecoveryType { ActorValue, CombatResource };
	
	enum class RecoveryFinishReason 
	{ 
		noComment,	//Reason irrelevant. Will not send an event (shouldn't rather)
		maxValue,	//The max value is reached. This currently clears pool as well.
		exhaustPool,//When the pool is naturally exhausted via means of regeneration, this will trigger. 
		clearPool	//When the pool is cleared on account of user action (so like starting another combo).
	};


	class RecoveryNode
	{
		//This object is in control of the recovery of specific ActorValues or resources.
		// It can use an actor value or resource if one would prefer in how to proceed in the reduction
		// of the damage value. In addition, it is capable of only reducing it by a certain amount and if such a setting
		// exists, it will expire once full value (0 damage) is reached. if the specific amount is below 0, then it will 
		// just keep decreasing the value until its done.

	protected:
		float _regenPool{ -1 };
		RecoverValue recoverSettings;


		float originalTimestamp = -1.f;
		std::uint8_t subscriptions;//This thing will have to be a serializable probably.


		friend class RecoveryController;
	public:

		//DELETE THESE SOON, if the other bits work I'll weed out the uses of these soon enough.
		//regen value, regenValueMult, regenMinValue are all things that can be, and will be placed in a setting.
		/*/Settings stuff, eventually I'll move this, and it will be stored based on associate of ID.
		std::int32_t regenValue;
		float regenValueMult;//Might have this double as a union, either pointing toward a real value, or an int.
		float regenMinValue;//This is the minimum percieved value for the gotten regen value.

		ReadableValue regenValue2;//This could be put into the recovery settings as well
		//*///end

		float regenDelay;

		bool isRecovering = false;

		RecoverySetting* GetSetting()
		{
			return ModSettingHandler::GetSetting<RecoverySetting>(recoverSettings);
		}

		bool IsPooled()
		{
			auto setting = GetSetting();

			return setting ? setting->isPool : false;
		}

		bool ClearRegenPool()
		{
			if (_regenPool < 0)//IsPooled() == false)//_regenPool < 0)//
				return false;

			_regenPool = 0;

			return true;
		}

		void SetRegenPool(float newLimit)
		{
			if (_regenPool < 0)//IsPooled() == false)//_regenPool < 0)//
				return;

			_regenPool = newLimit;
		}

		void IncRegenPool(float newLimit)
		{
			if (_regenPool < 0)//IsPooled() == false)//
				return;

			_regenPool += newLimit;
		}

		float GetRegenPool()
		{
			return _regenPool;
		}
		


		//These return true if this is the first time it's being subscribed, or the last unsubscription.
		bool RegenSubscribe(IRegenSubscriber* subscriber, float time_stamp)
		{
			if (!subscriber || subscriber->_subscriptionBit)
				return false;

			auto assignment = NextAssignment();
			//check for 7777777.0f, or something within untenable ranges
			if (!assignment)
				return false;

			auto returned = !subscriptions;

			subscriptions |= assignment;
			subscriber->_subscriptionBit = assignment;
			
			if (time_stamp < 777) {
				//RE::DebugMessageBox(std::format("timestamp {}, viable {}", time_stamp, time_stamp < 777));
				originalTimestamp = time_stamp;
			}
			else if (originalTimestamp == -1.f)
				//Mainly for the edge case where the original is -1. Need to have something to set it back to.
				originalTimestamp = 0;
			
			//RE::DebugMessageBox(std::format("ori gen {}, ", originalTimestamp));

			return returned;
		}

		bool RegenUnsubscribe(IRegenSubscriber* subscriber, float& out_time)
		{
			if (!subscriber || !subscriber->_subscriptionBit) {
				//RE::DebugMessageBox("subscriber was likely subscribed to begin with.");
				return false;
			}
			auto deassignment = subscriber->_subscriptionBit;

			subscriptions &= ~deassignment;
			subscriber->_subscriptionBit = 0;

			if (originalTimestamp >= 0) {
				out_time = originalTimestamp;
				
				if (!subscriptions)
					originalTimestamp = -1;
			}

			return !subscriptions;
		}


		inline bool IsSubscribed() { return subscriptions; }
		//RecoveryNode& operator=(const RecoveryNode& node){}

	private:

		std::uint8_t NextAssignment()
		{
			if (!subscriptions)
				return 1;
			else if (subscriptions == 0xFF)
				return 0;

			for (std::uint8_t bit = 1; bit != 0; bit <<= 1) {
				if (bit & subscriptions)
					continue;

				return bit;
			}

			return 0;
		}


	};

	class RecoveryController : 
		public IUpdateBehaviour,
		public ISerializer
	{
		//This object focuses on the recovery of ones values, it's doled overtime and can be interrupted or lost.
		// It is also important to note it will immediately recovery you upto a certain amount of resource.
		
	private:

		RecoveryType GetValueType(std::int32_t& value);

		
		
		//This is made once, and isn't really intended on changing to be honest.
		
		//The value for the key is checked bitwise against .
		// if the value is true, then it's an actor value. If not it's a combat resource.
		// Not super compatible with future additions, but who cares for now. These will be the only 2 ways to
		// regen to, anything else would be a reader value.
		SerializableMap<std::int32_t, RecoveryNode> _recoveryMap;
		SerializableMap<RE::ActorValue, float> _recoverContinueQueue;
		//SerialUnorderedMap<RE::ActorValue, float> test2;
		
		void EndRecovery(RecoveryNode& node, std::int32_t valueType, RecoveryFinishReason reason = RecoveryFinishReason::noComment);


		void StartRecovery(RecoveryNode& node, std::int32_t valueType);

		
		//float GetRegenValue(std::int32_t value, RE::Actor* actor = nullptr);
		//void HandleActorValue(RE::ActorValue av, RecoveryNode& node, float updateDelta);
		//void HandleCombatResource(CombatResource cr, RecoveryNode& node, float updateDelta);



		void HandleRecovery(RecoveryType type, std::int32_t value, RecoveryNode& node, float updateDelta);




		void AddRecoveryNode(std::int32_t gotoValue, RecoverValue recoverType);
	
		RecoveryNode* GetRegenNode(std::int32_t restoreValue, RecoveryType type)
		{
			//Add an inc later.

			restoreValue = type == RecoveryType::ActorValue ? restoreValue | 0x80000000 : restoreValue;

			if (_recoveryMap->contains(restoreValue) == false)
				return nullptr;

			return &_recoveryMap[restoreValue];
		}

	protected:
	
		void Build() override;


	public:

		void HandleSerialize(SerialArgument& serializer, bool& success)
		{
			serializer.Serialize(_recoveryMap);
			serializer.Serialize(_recoverContinueQueue);

			return;//Not sure I need this.

			if (serializer.IsDeserializing() == true)
			{
				auto actor = GetActorOwner();

				if (!actor || !actor->currentProcess || !actor->currentProcess->high)
					return;

				for (auto& [loc, node] : *_recoveryMap)
				{
					if (node.IsSubscribed() == false)
						continue;

					auto av = static_cast<RE::ActorValue>(loc);

					switch (av)
					{
					case RE::ActorValue::kHealth:
						actor->currentProcess->high->healthRegenDelay = 7777777;
						break;

					case RE::ActorValue::kMagicka:
						actor->currentProcess->high->magickaRegenDelay = 7777777;
						break;

					case RE::ActorValue::kStamina:
						actor->currentProcess->high->staminaRegenDelay = 7777777;
						break;
					}
				}
			}
		}

		//Deprecated Relic
		//Last 2 aren't gonna be used past a point,  so they default to how I'd do em
		void AddRecoveryNode(std::int32_t restoreValue, std::int32_t regenValue, RecoveryType type, bool poolType, float recoveryMin = 15, float recoveryMult = 1);
		//friender class combat data would be the normal way to do this.

		void AddRecoveryNode(CombatResource restoreValue, RecoverValue recoverType);

		void AddRecoveryNode(RE::ActorValue restoreValue, RecoverValue recoverType);





		//Should have a raw version
		//Additional notes, I really want to make it so if you start recovery with a pool and the resource is already full
		// it sends something that says, already full 
		void SetRegenPool(std::int32_t restoreValue, RecoveryType type, float value)
		{
			//Add an inc later.

			restoreValue = type == RecoveryType::ActorValue ? restoreValue | 0x80000000 : restoreValue;
			
			if (_recoveryMap->contains(restoreValue) == false)
				return;

			_recoveryMap[restoreValue].SetRegenPool(value);
		}

		void IncRegenPool(std::int32_t restoreValue, RecoveryType type, float value)
		{
			//Add an inc later.

			restoreValue = type == RecoveryType::ActorValue ? restoreValue | 0x80000000 : restoreValue;

			if (_recoveryMap->contains(restoreValue) == false)
				return;

			_recoveryMap[restoreValue].IncRegenPool(value);
		}
		
		void ClearRegenPool(std::int32_t restoreValue, RecoveryType type)
		{
			restoreValue = type == RecoveryType::ActorValue ? restoreValue | 0x80000000 : restoreValue;

			if (_recoveryMap->contains(restoreValue) == false)
				return;

			auto& node = _recoveryMap[restoreValue];

			if (node.ClearRegenPool() == true)
				//We send this so it doesn't send clear pool erroneously
				EndRecovery(node, restoreValue, RecoveryFinishReason::clearPool);
		}



		float GetRegenDelay(std::int32_t restoreValue, RecoveryType type)
		{
			//Add an inc later.

			restoreValue = type == RecoveryType::ActorValue ? restoreValue | 0x80000000 : restoreValue;

			if (_recoveryMap->contains(restoreValue) == false)
				return -1;

			return _recoveryMap[restoreValue].regenDelay;
		}

		void SetRegenDelay(std::int32_t restoreValue, RecoveryType type, float value)
		{
			restoreValue = type == RecoveryType::ActorValue ? restoreValue | 0x80000000 : restoreValue;

			if (_recoveryMap->contains(restoreValue) == false)
				return;

			_recoveryMap[restoreValue].regenDelay = value;
		}

		void IncRegenDelay(std::int32_t restoreValue, RecoveryType type, float value)
		{
			restoreValue = type == RecoveryType::ActorValue ? restoreValue | 0x80000000 : restoreValue;

			if (_recoveryMap->contains(restoreValue) == false)
				return;

			auto& delay = _recoveryMap[restoreValue].regenDelay;

			delay += value;
			delay = delay < 0 ? 0 : delay;
		}

		void SetRegenSubscription(RE::ActorValue av, IRegenSubscriber* subscriber, bool is_subscribing, bool* force_clear = nullptr)
		{
			if (!subscriber)// || subscriber->IsSubscribed() == is_subscribing)
				return;

			//For now, this is the only type.
			RecoveryType rec_type = RecoveryType::ActorValue;//Probably gonna need later, so might as well put it here.
				
			RecoveryNode* regen_control = GetRegenNode(static_cast<__int32>(av), rec_type);

			if (!regen_control)
				return;

			float* regen_ptr = nullptr;
			float regen_time = -1;


			auto actor = subscriber->GetActorOwner();

			switch (av)
			{
			//Handling for the native regen methods.
			case RE::ActorValue::kHealth:
				if (actor && actor->currentProcess && actor->currentProcess->high)
					regen_ptr = &actor->currentProcess->high->healthRegenDelay;
				break;

			case RE::ActorValue::kMagicka:
				if (actor && actor->currentProcess && actor->currentProcess->high)
					regen_ptr = &actor->currentProcess->high->magickaRegenDelay;
				break;

			case RE::ActorValue::kStamina:
				if (actor && actor->currentProcess && actor->currentProcess->high)
					regen_ptr = &actor->currentProcess->high->staminaRegenDelay;
				
				break;

				//If no regen_ptr, it should likely be one of these internal onces
			}

			if (regen_ptr)
				regen_time = *regen_ptr;


			auto setting = regen_control->GetSetting();


			if (is_subscribing)
			{
				if (regen_control->RegenSubscribe(subscriber, regen_time) == false)
					return;

				if (regen_ptr)
					*regen_ptr = 7777777;//regen paused.
				else 
					_recoverContinueQueue[av] = 7777777.f;

				//Utility::DebugMessageBox(std::format("new sub: {}", regen_control->subscriptions), IsPlayerRef());

				//Deal with the crucible regen stuff now. Clear pool, set it's value super high, the works. 

				//If it has this setting clear, otherwise do nothing, it's paused already. Optionally, this can be made to not
				// force a clear against the wish of the settings, or to force it to do so against the wish of the settings.
				if (!force_clear && setting && setting->pauseClear || force_clear && *force_clear ) {
					ClearRegenPool(static_cast<__int32>(av), rec_type);
				}
			}
			else//if (unsubscribing)
			{
				if (regen_control->RegenUnsubscribe(subscriber, regen_time) == false) {
					//Utility::DebugMessageBox(std::format("rem subs: {}",regen_control->subscriptions), IsPlayerRef());
					return;
				}
				
				//Utility::DebugMessageBox(std::format("subs free, t: {}, {}", regen_time, regen_ptr == nullptr), IsPlayerRef());
				//This should be cognizant of whether there will never be a regen timer. I think for something like this,
				// a dummy value should be implemented.
				if (regen_ptr) {
					//RE::DebugMessageBox(std::format("FREED {}", regen_control->_regenPool));
					*regen_ptr = regen_time;//resumed
				}
				else {
					_recoverContinueQueue[av] = regen_time < 0 ? 0 : regen_time;
				}
			}
		}

		void SetRegenSubscription(RE::ActorValue av, IRegenSubscriber* subscriber, bool is_subscribing, bool force_clear)
		{
			SetRegenSubscription(av, subscriber, is_subscribing, &force_clear);
		}
			
		void OnUpdate(RE::Actor* self, float updateDelta) override;



		

	};

}