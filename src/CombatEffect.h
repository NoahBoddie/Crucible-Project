#pragma once

namespace Crucible
{
	enum class Archetype
	{
		kNone,
		kValueModifier,
		kResourceModifier
	};

	enum class ValueModifier : std::uint8_t
	{
		kGeneralBase,	//This is rarely to be used. mainly just for the equip dealy.
		kGeneralMod,
		kNegativeBase,
		kNegativeMod,
		kPositiveBase,
		kPositiveMod,
		kTotalMod
		//,kPostTotal
	};

	//Core class for effect application. However as itself it does nothing.
	class CombatEffect
	{
		//These should be seperated by what their archetype is mind you.
	protected:
		void* target;
		
		//It's through this id that the magnitude of the original will be gotten and stored.
		std::uint16_t uniqueID;//If it's attached to a magic effect this is what will be used.


	public:
		//Gonna have to handle the create function. I think if I give it some of the numbers from papyrus,
		// and I hand over some of the active effect everything works out.


		//void Apply();
		//void Dispel();
		//void Change();
		virtual Archetype GetArchetype() = 0;
		float GetMagnitude() { return 0; }
		float GetDuration() { return 0; }

		
	};
}