#pragma once
#include "ItemStats.h"
#include "ItemStatHandler.h"

namespace Crucible
{
	
	float StatBase::GetValue(StatType getStat) 
	{ 
		return -1; 
	}

	float MinimumStat::GetValue(StatType getStat)
	{
		switch (getStat)
		{
		case StatType::baseStagger:
		case StatType::bashStagger:
			return baseStagger;
		case StatType::guardDamageMult:
			return guardDamageMult;
		case StatType::specialValue:
			return specialValue;
		default:
			return __super::GetValue(getStat);
		}
	}

	float BasicStat::GetValue(StatType getStat)
	{
		switch (getStat)
		{
		case StatType::bashPrecision:
		case StatType::basePrecision:
			return basePrecision;
		default:
			return __super::GetValue(getStat);
		}
	}

	float ShieldStat::GetValue(StatType getStat) 
	{
		switch (getStat)
		{
		case StatType::blockResist:
			return blockResist;
		case StatType::regWeightMult:
			return regWeightMult;
		case StatType::powWeightMult:
			return powWeightMult;
		case StatType::minorUpkeepMult:
			return minorUpkeepMult;
		case StatType::majorUpkeepMult:
			return majorUpkeepMult;
		case StatType::unguardedPenalty:
			return unguardedPenalty;
		case StatType::upkeepTime:
			return upkeepTime;
		case StatType::raiseBaseCost:
			return raiseBaseCost;
		case StatType::raiseWeightMult:
			return raiseWeightMult;
		case StatType::weakGuardTime:
			return weakGuardTime;
		case StatType::counterBashStagger:
			return counterBashStagger;
		case StatType::counterBashDamageMult:
			return counterBashDamageMult;
		case StatType::counterBashRevengeDamageMult:
			return counterBashRevengeDamageMult;
		case StatType::counterBashStaminaBase:
			return counterBashStaminaBase;
		case StatType::counterBashStaminaMult:
			return counterBashStaminaMult;
		case StatType::counterBashHealthReq:
			return counterBashHealthReq;
		case StatType::counterBashCountReq:
			return counterBashCountReq;
		case StatType::counterHitDecay:
			return counterHitDecay;
		case StatType::strongCounterHitDecay:
			return strongCounterHitDecay;
		case StatType::strongGuardRecovery:
			return strongGuardRecovery;
		case StatType::blockComboTime:
			return blockComboTime;
		case StatType::weakBlockComboTime:
			return weakBlockComboTime;
		case StatType::dropGuardTime:
			return dropGuardTime;
		case StatType::exhaustGuardTime:
			return exhaustGuardTime;
		default:
			return __super::GetValue(getStat);
		}
	}

	float BashStat::GetValue(StatType getStat)
	{
		//This overrides the call for precision on the basic stat, since for shields it stored there, but for 
		// weapons the bash data is seperate.
		switch (getStat)
		{
		case StatType::bashPrecision:
			return bashPrecision;
		case StatType::bashStagger:
			return bashStagger;
		case StatType::regBashWeightMult:
			return regBashWeightMult;
		case StatType::powBashWeightMult:
			return powBashWeightMult;
		default:
			return __super::GetValue(getStat);
		}
	}


	float ItemStat::GetValue(StatType getStat)
	{
		switch (getStat)
		{
		case StatType::baseStaminaCost:
			return baseStaminaCost;
		case StatType::staminaCostClimb:
			return staminaCostClimb;
		case StatType::staminaLossRateClimb:
			return staminaLossRateClimb;
		case StatType::regWeightMult:
			return regWeightMult;
		case StatType::firstComboTime:
			return firstComboTime;
		case StatType::penaltyComboTime:
			return penaltyComboTime;
		default:
			return __super::GetValue(getStat);
		}
	}

	float MeleeStat::GetValue(StatType getStat)
	{
		switch (getStat)
		{
		case StatType::firstBlowCost:
			return firstBlowCost;
		case StatType::staminaHitCostMult:
			return staminaHitCostMult;
		case StatType::firstBlowPowerRestore:
			return firstBlowPowerRestore;
		case StatType::followComboTime:
			return followComboTime;
		case StatType::swingPowerRestore:
			return swingPowerRestore;
		case StatType::hitPowerRestore:
			return hitPowerRestore;
		case StatType::firstBlowPowerCooldown:
			return firstBlowPowerCooldown;
		case StatType::successPowerCooldown:
			return successPowerCooldown;
		case StatType::failurePowerCooldown:
			return failurePowerCooldown;
		case StatType::blockResist:
			return blockResist;
		case StatType::raiseBaseCost:
			return raiseBaseCost;
		case StatType::raiseWeightMult:
			return raiseWeightMult;
		case StatType::regWeightMult:
			return regWeightMult;
		case StatType::minorUpkeepMult:
			return minorUpkeepMult;
		case StatType::majorUpkeepMult:
			return majorUpkeepMult;
		case StatType::unguardedPenalty:
			return unguardedPenalty;
		case StatType::powerAttackCostRate:
			return powerAttackCostRate;
		case StatType::weakGuardTime:
			return weakGuardTime;
		case StatType::upkeepTime:
			return upkeepTime;
		case StatType::blockComboTime:
			return blockComboTime;
		case StatType::weakBlockComboTime:
			return weakBlockComboTime;
		case StatType::dropGuardTime:
			return dropGuardTime;
		case StatType::exhaustGuardTime:
			return exhaustGuardTime;
		default:
			return __super::GetValue(getStat);
		}
	}

	float RaceStat::GetValue(StatType getStat)
	{
		switch (getStat)
		{
		case StatType::baseResist:
			return baseResist;
		case StatType::baseEvade:
			return baseEvade;
		case StatType::basePoise:
			return basePoise;
		case StatType::poiseHealthMult:
			return poiseHealthMult;
		case StatType::poiseBonusHealthMult:
			return poiseBonusHealthMult;
		default:
			return __super::GetValue(getStat);
		}
	}

	float RangedStat::GetValue(StatType getStat)
	{
		switch (getStat)
		{
			//case StatType::__:
			//	return __;
		default:
			return __super::GetValue(getStat);
		}
	}

	float MagicSpellStat::GetValue(StatType getStat)
	{
		switch (getStat)
		{
			//case StatType::__:
			//	return __;
		//This will just opt to the magic proj singleton for these

		//case StatType::firstComboTime:
		//	return firstComboTime;
		case StatType::concMagicPrecision:
		case StatType::beamMagicPrecision:
		case StatType::missileMagicPrecision:
		case StatType::coneMagicPrecision:
		case StatType::arrowMagicPrecision:
		case StatType::lobberMagicPrecision:
			//This will have to get fixed.
			return ItemStatHandler::magicProjectileStats.defaultStat.GetValue(getStat);//return ItemStatHandler::magicProjectileStats.GetValue(getStat);//

		default:
			return __super::GetValue(getStat);
		}
	}

	
	float MagicProjStat::GetValue(StatType getStat)
	{
		switch (getStat)
		{
		case StatType::concMagicPrecision:
			return concMagicPrecision;
		case StatType::beamMagicPrecision:
			return beamMagicPrecision;
		case StatType::missileMagicPrecision:
			return missileMagicPrecision;
		case StatType::coneMagicPrecision:
			return coneMagicPrecision;
		case StatType::arrowMagicPrecision:
			return arrowMagicPrecision;
		case StatType::lobberMagicPrecision:
			return lobberMagicPrecision;

		default:
			return __super::GetValue(getStat);
		}
	}
}