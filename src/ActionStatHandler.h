#pragma once

#include "ActionStat.h"

namespace Crucible
{

	class ActionStatHandler
	{
	private:
		//static inline ActionStat genericDodge;
		//static inline ActionStat genericGuard;
		
		//These are the generic actions, used for testing purposes.
		static void LoadGenericActions()
		{
			/*
			genericDodge.actionType = ActionState::Dodge;
			genericDodge.cooldownTime = 15;
			genericDodge.requirement = 2;
			genericDodge.isGeneric = true;
			genericDodge.uptime = 1;
		
			genericGuard.actionType = ActionState::Guard;
			genericGuard.cooldownTime = 15;
			genericGuard.requirement = 4;
			genericGuard.isGeneric = true;
			genericGuard.uptime = 1;
			//*/
		}

	public:
		static void Initialize()
		{
			//Since these aren't stored via value, and their settings are all the same
			// I can load these via loop. Probably. Would need a list of shit to go with it.
			//Load action stats

			//ModSettingHandler* handler = new ModSettingHandler();

			//SuperCoolReplicatableModSetting newGuy;
			//SuperCoolReplicatableModSetting* newSetting = new SuperCoolReplicatableModSetting(newGuy);

			//handler->AddSetting(newSetting, ActionPoint::genericDodge);
			LoadGenericActions();
		}

	private:
		//The use of this function is deprecated. Use the ModSettingHandler.
		static ActionStat* GetActionStat(ActionPoint action)
		{
			switch (action)
			{
			//case ActionPoint::genericDodge:
				//return &genericDodge;
			//case ActionPoint::genericGuard:
				//return &genericGuard;
			}

			return nullptr;
		}
	};
}