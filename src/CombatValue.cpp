#pragma once

#include "CombatValue.h"

namespace Crucible
{

	SimpleCombatValue& SimpleCombatValue::operator+(const SimpleCombatValue& a_rhs) {
		//makes a new value of the left hand values adds the values of the right hand
		SimpleCombatValue a_lhs = *this;

		a_lhs.totalMod += a_rhs.totalMod;

		a_lhs.positiveBase += a_rhs.positiveBase;
		a_lhs.negativeBase += a_rhs.negativeBase;

		a_lhs.generalMod += a_rhs.generalMod;
		a_lhs.positiveMod += a_rhs.positiveMod;
		a_lhs.negativeMod += a_rhs.negativeMod;
		return a_lhs;
	}

	SimpleCombatValue& SimpleCombatValue::operator-(const SimpleCombatValue& a_rhs) {
		//makes a new value of the left hand values reduced by the right hand;
		SimpleCombatValue a_lhs = *this;

		a_lhs.totalMod -= a_rhs.totalMod;

		a_lhs.positiveBase -= a_rhs.positiveBase;
		a_lhs.negativeBase -= a_rhs.negativeBase;

		a_lhs.generalMod -= a_rhs.generalMod;
		a_lhs.positiveMod -= a_rhs.positiveMod;
		a_lhs.negativeMod -= a_rhs.negativeMod;
		return a_lhs;
	}

	CacheCombatValue& SimpleCombatValue::operator+(const CacheCombatValue& a_rhs) {
		//makes a new value of the left hand values adds the values of the right hand
		CacheCombatValue a_lhs;

		a_lhs.generalBase += a_rhs.generalBase;


		a_lhs.totalMod = totalMod + a_rhs.totalMod;

		a_lhs.positiveBase = positiveBase + a_rhs.positiveBase;
		a_lhs.negativeBase = negativeBase + a_rhs.negativeBase;

		a_lhs.generalMod = generalMod + a_rhs.generalMod;
		a_lhs.positiveMod = positiveMod + a_rhs.positiveMod;
		a_lhs.negativeMod = negativeMod + a_rhs.negativeMod;

		return a_lhs;
	}

	CacheCombatValue& SimpleCombatValue::operator-(const CacheCombatValue& a_rhs)
	{
		//makes a new value of the left hand values reduced by the right hand;
		CacheCombatValue a_lhs;

		a_lhs.generalBase -= a_rhs.generalBase;


		a_lhs.totalMod = totalMod - a_rhs.totalMod;

		a_lhs.positiveBase = positiveBase - a_rhs.positiveBase;
		a_lhs.negativeBase = negativeBase - a_rhs.negativeBase;

		a_lhs.generalMod = generalMod - a_rhs.generalMod;
		a_lhs.positiveMod = positiveMod - a_rhs.positiveMod;
		a_lhs.negativeMod = negativeMod - a_rhs.negativeMod;

		return a_lhs;

	}





	
}