#pragma once

#include "EventSystem.h"
#include "CombatDataBehaviour.h"


namespace Crucible
{
	
	class IUpdateBehaviour : public CombatDataBehaviour
	{
	protected:
		virtual void OnUpdate(RE::Actor* self, float delta) = 0;
		
		void Initialize_INTERNAL() override;
	
	//Legacy event behaviour
	//private:
	//	void OnUpdate_INTERNAL(EventObject& parameters, EventControl& control);

	};
}