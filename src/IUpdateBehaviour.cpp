#pragma once

#include "IUpdateBehaviour.h"


namespace Crucible
{
	//Legacy behaviour
	//void IUpdateBehaviour::OnUpdate_INTERNAL(EventObject& parameters, EventControl& control)
	//{
	//	RE::Actor* self = nullptr;
	//	float delta = 0;
	//	if (parameters[0].AsFormType<RE::Actor>(self) == false || parameters[1].AsFloat(delta) == false)
	//		return;	
	//	OnUpdate(self, delta);
	//}

	void IUpdateBehaviour::Initialize_INTERNAL()
	{
		CreateMemberEvent("OnUpdate", &IUpdateBehaviour::OnUpdate);
		__super::Initialize_INTERNAL();
	}
}


