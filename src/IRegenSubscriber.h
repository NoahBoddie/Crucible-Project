#pragma once

#include "SerialComponent.h"
#include "CombatDataBehaviour.h"


namespace Crucible
{


	class IRegenSubscriber : 
		public CombatDataBehaviour,
		public SerialComponent<std::uint8_t>
	{
	private:
		friend class RecoveryNode;

		std::uint8_t& _subscriptionBit = data;

	public:
	//On deserialize, you need to evaluate the conditions of this function.
		void SetRegenSubscription(RE::ActorValue av, bool is_subscribing);
		void SetRegenSubscription(RE::ActorValue av, bool is_subscribing, bool force_clear);
	
		bool IsSubscribed() { return _subscriptionBit != 0; }

		//For now does nothing. But later is to be used to register how much of a penalty there is if when all recovery in a stat is complete,
		// what level of penalty there is. This function is likely going to be kept in pairs, so I can have magnitude work different from
		// duration.
		virtual float GetRegenPenalty() { return 0.f; }

		virtual void SerializeData(SerialArgument& serializer, bool& success) override { /*remains to be unimplemented*/ }
	};
}