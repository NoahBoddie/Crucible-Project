#pragma once
namespace Crucible
{
	//The idea of this struct is supposed to a constant value that's put on a piece of combat data,
	// which then removes itself, taking the exact value it started with.
	class CombatValueEffect
	{
	public:
		CombatValueEffect()
		{

		}

		int specialID;//Supposed to be the special id of the thing that 

		//Chains of CombatValueEffects are loaded to keep track of collect changes from objects
		CombatValueEffect* next;
								
	};

}