#pragma once
#include "Utility.h"


namespace Crucible
{
	class CombatData;
	enum class CombatResource;

	//Making readable values have a base option would be very useful. It's mostly set up for it already.
	// It would make searching out damage values very easy.

	// A later note if I want to roll through these, loop an int set to 1 and double it each round (or bit shift I don't care).
	// use bitwise and on it and see if anything comes out. keep going until the value of the loop index is larger than
	// the enum value given
	enum class IncludeFlags
	{
		kNone			= 0,
		kBase			= 1 << 0,
		kDamage			= 1 << 1,
		kTemporary		= 1 << 2,
		kPermenant		= 1 << 3,
		kMax			= kBase | kTemporary | kPermenant,
		kAll			= kMax | kDamage,
		kNoTemporary	= kAll ^ kTemporary,
		kNoPermenant	= kAll ^ kPermenant,
		kMaxNoTemporary	= kMax ^ kTemporary,
		kMaxNoPermenant = kMax ^ kPermenant
	};

	//Object presents issues, value presented based on health using kMax was inordinately high.
	// This is perhaps because I failed to load it with the sign bit, and as a result was defective.

	//Serves as interface between actor values and combat resources. Submitting an actor of combat data
	// will get the viable version of whichever is its settings imply, including modifiers choosen via settings.
	struct ReadableValue
	{
#define extra_bits		0xF8000000//function_bit | damage_bit | //
#define function_bit	sign_bit//Function bit is the base bit.
#define permenant_bit	0x40000000
#define temporary_bit	0x20000000
#define damage_bit		0x10000000
#define actorvalue_bit	0x8000000


	private:
		std::int32_t	_value;
		
		bool			_isFunction;
		bool			_isActorValue;
		bool			_includeDamage;
		bool			_includeTemporary;
		bool			_includePermenant;

	public:
		
		RE::ActorValue AsActorValue();

		CombatResource AsResource();

		void SetValue(std::int32_t newValue);


		std::int32_t FlagsToInt(IncludeFlags flags);

		void SetValue(CombatResource cr, IncludeFlags flags);

		void SetValue(RE::ActorValue av, IncludeFlags flags);


		IncludeFlags GetFlags()
		{
			if (_isFunction)
				return IncludeFlags::kNone;


			auto flags = IncludeFlags::kBase;

			if (_includeDamage)
				flags |= IncludeFlags::kDamage;

			if (_includeTemporary)
				flags |= IncludeFlags::kTemporary;

			if (_includePermenant)
				flags |= IncludeFlags::kPermenant;

			return flags;
		}

		bool IsFunction() { return _isFunction; }
		bool IncludeDamage() { return _includeDamage; }
		bool IncludeTemporary() { return _includeTemporary; }
		bool IncludePermenant() { return _includePermenant; }

		//Use these in the resource owner to prevent overflow
		bool IsActorValue() { return _isActorValue; }
		bool IsCombatResource() { return _isFunction && !IsActorValue(); }
		
		float GetValue(RE::Actor* actor);

		float GetValue(CombatData* data);

		//Basically serves as a way to easily increment the value at the location this readable value denotes.
		// Does nothing for the moment. positives go up, negatives go down. 
		// So for combat resources its inversed, and for actor values its the right way around.
		//Always does damage values.
		bool ModDistantValue(RE::Actor* actor, float value);

		bool ModDistantValue(CombatData* data, float value);
	private:

		

		float GetActorValue(RE::Actor* data);
		
		float GetCombatResource(CombatData* data);
	};

}