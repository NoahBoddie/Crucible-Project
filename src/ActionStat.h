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
		//Generic and custom action are solely used for returning purposes (and generic ain't even long for this world).
		generic = 0,
		autoDodge,
		genericDodge,
		genericGuard, 
		//Dodge
		guardDropDodge,			//When a light shield willingly lowers it's guard after having a successful (something, maybe bash) this becomes a guard point. Smol
		//Guard
		lightStrongGuard,		//This one is used when a light shield is in strong guard
		heavyStrongGuard,		//This is the stat that is used when a heavy shield goes into guard point.
		customAction
	};

	class ActionStat : public SettingTemplate<ActionPoint>
	{
	public:

		uint8_t priority;//This shouldn't move, as the priority shouldn't move too much.

		//In a variable all this is sel
		ActionState actionType;//Ready types will do fuck all.

		// if there's a bonus it will be positive, negative if negative, nothing if nothing.
		// currently, there's nothing for either type.
		float bonus;

		bool isGeneric;	//since looking this up already entails knowing where it should go if its good, this flag
						// serves as the method to tell if it goes to neutral or not.
		float requirement;

		//What amount of success one must recieve to have this not go into cooldown.
		// A value of 0 will dictate that it's cooldown will never be void.
		std::uint8_t successRequirement = 1;


		float uptime;
		float cooldownTime;
	};
}