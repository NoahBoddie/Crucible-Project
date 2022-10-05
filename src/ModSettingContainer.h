#pragma once

#include "ActionStat.h"
#include "StrikeSettings.h"
#include "RecoverySettings.h"
#include "GeneralSettings.h"

namespace Crucible
{
	//I think I would like to use initializers for all this shit after some point, really sell the idea

	namespace ActionSettings
	{
		inline ActionStat genericDodge;
		inline ActionStat genericGuard;

		inline ActionStat autoDodge;

		inline ActionStat guardDropDodge;
		inline ActionStat lightShieldGuard;
		inline ActionStat heavyShieldGuard;


		inline void Initialize()
		{
			genericDodge.actionType = ActionState::Dodge;
			genericDodge.cooldownTime = 10;
			genericDodge.requirement = 4;
			genericDodge.isGeneric = true;
			genericDodge.uptime = 2;

			genericGuard.actionType = ActionState::Guard;
			genericGuard.cooldownTime = 15;
			genericGuard.requirement = 4;
			genericGuard.isGeneric = true;
			genericGuard.uptime = 1;


			autoDodge.actionType = ActionState::Dodge;
			autoDodge.cooldownTime = 15;
			//Was 1 for testing purposes. 4 restricts auto dodging for clothes, but makes it able with walking. Maybe I'll make it 5,
			// so it's out of reach of heavy armor instead.
			// I'm troubled, no one dodges naturally anymore. I wonder if its the attacking and blocking thing that did it.
			autoDodge.requirement = 6;//REMEMBER, this was only to happen if evasion was HIGH make ways for evasion to climb. HIGHER is good.
			autoDodge.successRequirement = 5;
			autoDodge.isGeneric = false;
			autoDodge.uptime = 1;//Uptime doesn't matter.
			autoDodge.bonus = -2;


			//std::numeric_limits<T>::infinity

			guardDropDodge.actionType = ActionState::Dodge;
			guardDropDodge.cooldownTime = 6;
			//This requirement is just about the level where moving to the side and dropping your guard is a dodge in full light. Maybe drop it a tone
			guardDropDodge.requirement = 5;
			guardDropDodge.successRequirement = 5;
			guardDropDodge.isGeneric = false;
			guardDropDodge.uptime = 2.5f;
			guardDropDodge.bonus = 1;//because of the above, this is actually a really difficult dodge point, more bonus.


			//If one isn't currently in this guard point, it should try for it. Constantly.
			heavyShieldGuard.actionType = ActionState::Guard;
			heavyShieldGuard.cooldownTime = 30;
			heavyShieldGuard.requirement = 10;
			heavyShieldGuard.successRequirement = 20;
			heavyShieldGuard.isGeneric = false;
			heavyShieldGuard.uptime = std::numeric_limits<float>::infinity();
			heavyShieldGuard.bonus = 0;

			//If one isn't currently in this guard point, it should try for it. Constantly.
			lightShieldGuard.actionType = ActionState::Guard;
			lightShieldGuard.cooldownTime = 8;
			lightShieldGuard.requirement = 5;
			lightShieldGuard.successRequirement = 1;
			lightShieldGuard.isGeneric = false;
			lightShieldGuard.uptime = std::numeric_limits<float>::infinity();
			lightShieldGuard.bonus = 0;
		}
	}

	namespace StrikeSettings
	{
		inline StrikeSetting offenseEvadeSetting;
		inline StrikeSetting defenseEvadeSetting;
		inline StrikeSetting offenseHitSetting;
		inline StrikeSetting defenseHitSetting;

		inline void Initialize()
		{
			offenseHitSetting.minorThreshold = 0;
			offenseHitSetting.majorThreshold = 5;

			defenseHitSetting.minorThreshold = 4;
			defenseHitSetting.majorThreshold = 6;



			offenseEvadeSetting.minorThreshold = 0;
			offenseEvadeSetting.majorThreshold = 2;

			defenseEvadeSetting.minorThreshold = 0;
			defenseEvadeSetting.majorThreshold = 10;//Applied different, so the number is high

		}
	}

	namespace RecoverySettings
	{
		inline RecoverySetting poiseSetting;
		inline RecoverySetting staminaSetting;
		inline RecoverySetting magickaSetting;
		inline RecoverySetting speedSetting;

		inline void Initialize()
		{
			poiseSetting.regenValue.SetValue(RE::ActorValue::kHealRate, IncludeFlags::kMax);
			poiseSetting.recoveryMin = 5;//doesn't move like this heh
			poiseSetting.recoveryRate = 1.55f;//Damaging this for now.//This some how isn't enough.
			poiseSetting.isPool = false;
			
			staminaSetting.regenValue.SetValue(RE::ActorValue::KStaminaRate, IncludeFlags::kMax);
			staminaSetting.recoveryMin = 5;
			staminaSetting.recoveryRate = 5;
			staminaSetting.isPool = true;
		}
	}

	namespace General
	{
		inline GeneralSettings settings;
	}

}