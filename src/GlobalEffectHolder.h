#pragma once

#include "CombatValue.h"
#include "QueryContext.h"

namespace Crucible
{
	//Consider having this include the value queried, so I can decide to how to handle it. This is mainly for the ranged offset
	//functionality. If the source isn't ranged it will merely tack the offset on top. But it will need to know
	// what one to pick.
	//using EffectFunction = void(SimpleCombatValue&, QueryContext&);
	using EffectFunction = void(CombatValue, SimpleCombatValue&, QueryContext&);

	class GlobalEffectHolder
	{
	private:
		
		using GlobalEffectContainer = std::map<const CombatValue, std::vector<EffectFunction*>>;

		
		//These lists don't reload. Instead of putting these on the characters themselves, these exist independently
		// loaded, and run on every single character
		
		//Note, these will not, and should not need smart pointers, they don't get deleted. Ever.
		// Even through saves, they don't change much.
		static inline GlobalEffectContainer _effectContainer;
		
	public:
		
		static void Initialize();

		//should make a new list, don't want to make any changes from here.
		static std::vector<EffectFunction*>& GetFunctionList(const CombatValue value);

		static bool AddFunctionToGlobal(const CombatValue listToAddTo, EffectFunction* newFunction);

		//How many lists, does one fucking have on these?
		// There's always the global, then theres the shit added to npcs by effects and what not
		// then there's the shit added by the OTHER person in calculation. So from this, it should produce
		// an array that is the combinate of 3 lists.
	};

}