#pragma once

#include "QueryContext.h"
#include "CombatEffect.h"

namespace Crucible
{
	class CombatData;
	class CacheCombatValue;
	
	enum class CombatValue
	{
		//Core
		kResist,
		kEvade,
		kStagger,
		kPrecision,
		//Resource
	};

	

	



	struct SimpleCombatValue
	{
		//const int kUnits = 4;//A concept to preserve numbers against floating point inaccuracy. not sure if needed.

		std::float_t totalMod{ 1.f };

		std::float_t positiveBase{ 0 };
		std::float_t negativeBase{ 0 };

		std::float_t generalMod{ 1.f };
		std::float_t positiveMod{ 1.f };
		std::float_t negativeMod{ 1.f };


		virtual const char* print()
		{ 
			return std::format("genMult: {}\n pos:{} \nposMult: {} \nneg: {} \nnegMult: {} \ntotalMult: {}",
			generalMod, positiveBase, positiveMod, negativeBase, negativeMod, totalMod).c_str();
		}

		static SimpleCombatValue zero() 
		{ 
			SimpleCombatValue value;
			value.generalMod = 0;
			value.positiveMod = 0;
			value.negativeMod = 0;
			value.totalMod = 0;
			return value; 
		}

		virtual SimpleCombatValue& operator=(const std::float_t& a_rhs) {
			//sets the generalBase, but here does nothing
			return *this;
		}

		virtual SimpleCombatValue& operator&=(const std::float_t& a_rhs) {
			//Increments generalBase, but here does nothing
			return *this;
		}

		SimpleCombatValue& operator^=(const std::float_t& a_rhs) {
			//Increments the totalMod

			totalMod += a_rhs;
			return *this;
		}
		SimpleCombatValue& operator%=(const std::float_t& a_rhs) {
			//Increments the generalMod
			generalMod += a_rhs;
			return *this;
		}
		SimpleCombatValue& operator+=(const std::float_t& a_rhs) {
			//Increments the postiveBase
			positiveBase += a_rhs;
			return *this;
		}
		SimpleCombatValue& operator-=(const std::float_t& a_rhs) {
			//Increments the negativeBase
			negativeBase += a_rhs;
			return *this;
		}
		

		SimpleCombatValue& operator*=(const std::float_t& a_rhs) {
			//Increments the positiveMod
			positiveMod += a_rhs;
			return *this;
		}
		SimpleCombatValue& operator/=(const std::float_t& a_rhs) {
			//Increments the negativeMod
			negativeMod += a_rhs;
			return *this;
		}


		virtual SimpleCombatValue& operator+(const SimpleCombatValue& a_rhs);

		virtual SimpleCombatValue& operator-(const SimpleCombatValue& a_rhs);

		virtual CacheCombatValue& operator+(const CacheCombatValue& a_rhs);

		virtual CacheCombatValue& operator-(const CacheCombatValue& a_rhs);


		//Want an operator that makes this into a cached if operated on with a cache. But need a source file to do so.

	protected:
		virtual void Clear()
		{
			totalMod = 0;
			positiveBase = 0;
			negativeBase = 0;

			generalMod = 0;
			positiveMod = 0;
			negativeMod = 0;
		}
	};


	struct CacheCombatValue : public SimpleCombatValue
	{

	public:
		//As confusing as I'm sure it will be, the general base represents many things now.
		// it represents the damage mod for resources, and a post mod for core values. Either way, when loaded
		// the original general base is thrown aside, and replaced with the calculated general base. Notable exceptions
		// are core values resist and evade which both need it.
		std::float_t generalBase{ 0 };

		const char* print() override
		{
		
			return std::format("gen: {} \n{}", generalBase, __super::print()).c_str();
		}

		static CacheCombatValue zero() { CacheCombatValue value; return value; }


		CacheCombatValue& operator=(const std::float_t& a_rhs) override {
			//sets the generalBase

			generalBase = a_rhs;
			return *this;
		}

		CacheCombatValue& operator&=(const std::float_t& a_rhs) override {
			//Increments generalBase

			generalBase += a_rhs;
			return *this;
		}

		CacheCombatValue& operator+=(const std::float_t& a_rhs) {
			//Increments the postiveBase, needs to be redone because of later operators? I guess????
			positiveBase += a_rhs;
			return *this;
		}

		CacheCombatValue& operator+(const SimpleCombatValue& a_rhs) override  {
			//makes a new value of the left hand values adds the values of the right hand
			CacheCombatValue a_lhs = *this;

			a_lhs.totalMod += a_rhs.totalMod;

			a_lhs.positiveBase += a_rhs.positiveBase;
			a_lhs.negativeBase += a_rhs.negativeBase;

			a_lhs.generalMod += a_rhs.generalMod;
			a_lhs.positiveMod += a_rhs.positiveMod;
			a_lhs.negativeMod += a_rhs.negativeMod;
			return a_lhs;
		}

		CacheCombatValue& operator-(const SimpleCombatValue& a_rhs) override {
			//makes a new value of the left hand values reduced by the right hand;
			CacheCombatValue a_lhs = *this;

			a_lhs.totalMod -= a_rhs.totalMod;

			a_lhs.positiveBase -= a_rhs.positiveBase;
			a_lhs.negativeBase -= a_rhs.negativeBase;

			a_lhs.generalMod -= a_rhs.generalMod;
			a_lhs.positiveMod -= a_rhs.positiveMod;
			a_lhs.negativeMod -= a_rhs.negativeMod;
			
			return a_lhs;
		}



		CacheCombatValue& operator+(const CacheCombatValue& a_rhs) override 
		{
			//makes a new value of the left hand values adds the values of the right hand
			CacheCombatValue a_lhs = *this;

			a_lhs.generalBase += a_rhs.generalBase;
			
			a_lhs.totalMod += a_rhs.totalMod;

			a_lhs.positiveBase += a_rhs.positiveBase;
			a_lhs.negativeBase += a_rhs.negativeBase;

			a_lhs.generalMod += a_rhs.generalMod;
			a_lhs.positiveMod += a_rhs.positiveMod;
			a_lhs.negativeMod += a_rhs.negativeMod;

			return a_lhs;
		}

		CacheCombatValue& operator-(const CacheCombatValue& a_rhs) override
		{
			//makes a new value of the left hand values reduced by the right hand;
			CacheCombatValue a_lhs = *this;

			a_lhs.generalBase -= a_rhs.generalBase;


			a_lhs.totalMod -= a_rhs.totalMod;

			a_lhs.positiveBase -= a_rhs.positiveBase;
			a_lhs.negativeBase -= a_rhs.negativeBase;

			a_lhs.generalMod -= a_rhs.generalMod;
			a_lhs.positiveMod -= a_rhs.positiveMod;
			a_lhs.negativeMod -= a_rhs.negativeMod;

			return a_lhs;

		}


		CacheCombatValue& operator=(const CacheCombatValue& a_rhs) {
			//makes a new value of the left hand values adds the values of the right hand
			generalBase = a_rhs.generalBase;
			totalMod = a_rhs.totalMod;

			positiveBase = a_rhs.positiveBase;
			negativeBase = a_rhs.negativeBase;

			generalMod = a_rhs.generalMod;
			positiveMod = a_rhs.positiveMod;
			negativeMod = a_rhs.negativeMod;

			return *this;
		}

		CacheCombatValue& operator=(const SimpleCombatValue& a_rhs) {
			//makes a new value of the left hand values adds the values of the right hand
			totalMod = a_rhs.totalMod;

			positiveBase = a_rhs.positiveBase;
			negativeBase = a_rhs.negativeBase;

			generalMod = a_rhs.generalMod;
			positiveMod = a_rhs.positiveMod;
			negativeMod = a_rhs.negativeMod;

			return *this;
		}


		CacheCombatValue& operator+=(const CacheCombatValue& a_rhs) {
			//makes a new value of the left hand values adds the values of the right hand
			generalBase += a_rhs.generalBase;
			totalMod += a_rhs.totalMod;

			positiveBase += a_rhs.positiveBase;
			negativeBase += a_rhs.negativeBase;

			generalMod += a_rhs.generalMod;
			positiveMod += a_rhs.positiveMod;
			negativeMod += a_rhs.negativeMod;

			return *this;
		}

		CacheCombatValue& operator+=(const SimpleCombatValue& a_rhs) {
			//makes a new value of the left hand values adds the values of the right hand
			totalMod += a_rhs.totalMod;

			positiveBase += a_rhs.positiveBase;
			negativeBase += a_rhs.negativeBase;

			generalMod += a_rhs.generalMod;
			positiveMod += a_rhs.positiveMod;
			negativeMod += a_rhs.negativeMod;

			return *this;
		}



		//CacheCombatValue(float init)
		//	: generalBase(init)
		//{ }

		float operator()() {
			float returnValue = totalMod * (generalBase * generalMod) + (positiveBase * positiveMod - negativeBase * negativeMod);
			return returnValue;
		}

	protected:
		void Clear() override
		{
			generalBase = 0;
			__super::Clear();
		}
	};

	//I want an interface between this and ConditionalCombatValue

	//What I want is combat effects, They work similar to the lambda that are kept. They have their effect's special id
	// kept close to them. They have apply, dispel, and change functions. CombatValueEffects Don't use any of these,
	// as they evaluate in the time, checking for it's own conditions, and if it has none sending it's value.


	struct CombatValueEffect : public CombatEffect
	{
		//Needs a centalized active effect, this should only be one branch of an effect.
		//Not quite finished yet, don't use.
	protected:
		CombatValue		valueType;
		ValueModifier	modifier;


		bool conditional;//I attached to an effect that has counter effects it's marked as conditional.


	public:

		bool IsConditional() { return conditional; }


		SimpleCombatValue temp_AsCombatValue()
		{
			SimpleCombatValue tempValue;

			float value = GetMagnitude();

			switch (modifier)
			{
			case ValueModifier::kGeneralMod:
				tempValue.generalMod = value;
				break;

			case ValueModifier::kNegativeBase:
				tempValue.negativeBase = value;
				break;

			case ValueModifier::kNegativeMod:
				tempValue.negativeMod = value;
				break;

			case ValueModifier::kPositiveBase:
				tempValue.positiveBase = value;
				break;

			case ValueModifier::kPositiveMod:
				tempValue.positiveMod = value;
				break;

			case ValueModifier::kTotalMod:
				tempValue.totalMod = value;
				break;
			}

			return tempValue;
		}


		void TryApplyValue(SimpleCombatValue& value, QueryContext& context)
		{
			if (true)//If the conditions set upon ye ring true,
			{
				value = temp_AsCombatValue();
			}
		}

		Archetype GetArchetype() override { return Archetype::kValueModifier; }
	};

	
	
	/*
	struct CombatValue
	{
	public:
		std::int8_t generalBase{ 0 };
		std::int8_t positiveBase{ 0 };
		std::int8_t negativeBase{ 0 };

		std::float_t generalMod{ 1.f };
		std::float_t positiveMod{ 1.f };
		std::float_t negativeMod{ 1.f };

		std::float_t totalMod{ 1.f };

		std::int8_t IncValue(std::int8_t inc)
		{
			generalBase += inc;
			return generalBase;
		}

		//The short hand, I would like this to be implicit later.
		float DeleteLater()
		{
			float returnValue = totalMod *(generalBase * generalMod) + (positiveBase * positiveMod - negativeBase * negativeMod);

			return returnValue / 2;
		}


		CombatValue& operator=(const std::int8_t& a_rhs) {
			//Unlike the others, sets the general base. It's annoying to have to set it like this but 
			// for the convience of the others.

			generalBase = a_rhs;
			return *this;
		}
		CombatValue& operator&=(const std::int8_t& a_rhs) {
			//Unlike the others, sets the general base. It's annoying to have to set it like this but 
			// for the convience of the others.

			generalBase += a_rhs;
			return *this;
		}
		CombatValue& operator^=(const std::float_t& a_rhs) {
			//Unlike the others, sets the general base. It's annoying to have to set it like this but 
			// for the convience of the others.

			totalMod = a_rhs;
			return *this;
		}
		CombatValue& operator+=(const std::int8_t& a_rhs) {
			//Increments the postiveBase
			positiveBase += a_rhs;
			return *this;
		}
		CombatValue& operator-=(const std::int8_t& a_rhs) {
			//Increments the negativeBase
			negativeBase += a_rhs;
			return *this;
		}
		CombatValue& operator%=(const std::float_t& a_rhs) {
			//Increments the generalMod
			generalMod += a_rhs;
			return *this;
		}
		CombatValue& operator*=(const std::float_t& a_rhs) {
			//Increments the positiveMod
			positiveMod += a_rhs;
			return *this;
		}
		CombatValue& operator/=(const std::float_t& a_rhs) {
			//Increments the negativeMod
			negativeMod += a_rhs;
			return *this;
		}
		//implicitly casts to a float, and when it does it is the result of everything together.
		// however, it's important to note that a function should be the thing that does this, and that function
		// has to accept conditional values, just the operator does it with none.
		
		//At a later point I want to override the operators for this. 
		// = sets generalBase, += sets positiveBase -= sets negative base.
		//And never mind, do it now.
	};


	struct DualCombatValue : public CombatValue
	{
		//This is the only thing that makes an impact on left hands.
		std::int8_t leftGeneralBase{ 0 };
	};
	//*/

	//Kill
	using ValueEffectContainer = std::map<const CombatValue, std::list<CombatValueEffect*>>;
	//using CombatValueContainer = std::map<const CombatValue, CacheCombatValue>;
}