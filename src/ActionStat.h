#pragma once

#include "ModSettings.h"

namespace Crucible
{
	enum class ActionState : std::uint8_t 
	{ 
		Ready, 
		Dodge, 
		Guard 
	};
	
	enum class ActionPoint : std::uint8_t
	{
		//All three generic types are to be unused directly in the final product.
		// generic, being the tag for an action point going to the generic timers.
		generic = 0,
		genericDodge,
		genericGuard
	};

	class ActionStat : public SettingTemplate<ActionPoint>
	{
	public:

		//In a variable all this is sel
		ActionState actionType;//Ready types will do fuck all.

		// if there's a bonus it will be positive, negative if negative, nothing if nothing.
		// currently, there's nothing for either type.
		float bonus;

		bool isGeneric;	//since looking this up already entails knowing where it should go if its good, this flag
						// serves as the method to tell if it goes to neutral or not.
		float requirement;

		float uptime;
		float cooldownTime;
	};
}