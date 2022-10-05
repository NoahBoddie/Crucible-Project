#pragma once
#include "CombatData.h"
#include "IRegenSubscriber.h"
#include "RecoveryController.h"


namespace Crucible
{
	void IRegenSubscriber::SetRegenSubscription(RE::ActorValue av, bool is_subscribing)
	{
		owner->recoverControl.SetRegenSubscription(av, this, is_subscribing);


		//if (is_subscribing)
		//	RE::DebugMessageBox(std::format("sub-bit: {}, setting: {}", _subscriptionBit, is_subscribing));
	}

	void IRegenSubscriber::SetRegenSubscription(RE::ActorValue av, bool is_subscribing, bool force_clear)
	{

		owner->recoverControl.SetRegenSubscription(av, this, is_subscribing, force_clear);


	}
}