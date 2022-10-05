#pragma once

#include "Utility.h"
#include "ItemStats.h"
#include <cmath>

namespace Crucible
{
	class ItemStatHandler
	{
		//This object should be responsible for getting things like baseStagger types and keep records for recover bases
		// and such. For now, get baseStagger and resist for weapons and spells.

	private:

#pragma region RaceFunctions
		static void LoadNpcRaceStats()
		{
			npcRaceStats.defaultStat.baseStagger = 3;
			npcRaceStats.defaultStat.basePrecision = 2;
			npcRaceStats.defaultStat.baseResist = 0;
			npcRaceStats.defaultStat.baseEvade = 3;

			npcRaceStats.defaultStat.basePoise = 40;
			npcRaceStats.defaultStat.poiseHealthMult = 0.1f;


			npcRaceStats.defaultStat.firstBlowCost = 8;
			npcRaceStats.defaultStat.baseStaminaCost = 15;
			npcRaceStats.defaultStat.staminaCostClimb = 10;
			npcRaceStats.defaultStat.staminaHitCostMult = 0.5f;

			npcRaceStats.defaultStat.firstBlowPowerRestore = 0.75f;
			npcRaceStats.defaultStat.swingPowerRestore = 0.5f;
			npcRaceStats.defaultStat.hitPowerRestore = 2;

			npcRaceStats.defaultStat.firstBlowPowerCooldown = 6;
			npcRaceStats.defaultStat.successPowerCooldown = 4;
			npcRaceStats.defaultStat.failurePowerCooldown = 8;
		}

		static void LoadSmallRaceStats()
		{
			//Small
			smallRaceStats.defaultStat.baseStagger = 2;
			smallRaceStats.defaultStat.basePrecision = 6;
			smallRaceStats.defaultStat.baseResist = 2;
			smallRaceStats.defaultStat.baseEvade = 6;


			smallRaceStats.defaultStat.firstBlowCost = 8;
			smallRaceStats.defaultStat.baseStaminaCost = 15;
			smallRaceStats.defaultStat.staminaCostClimb = 10;
			smallRaceStats.defaultStat.staminaHitCostMult = 0.5f;

			smallRaceStats.defaultStat.firstBlowPowerRestore = 0.75f;
			smallRaceStats.defaultStat.swingPowerRestore = 0.5f;
			smallRaceStats.defaultStat.hitPowerRestore = 2;

			smallRaceStats.defaultStat.firstBlowPowerCooldown = 6;
			smallRaceStats.defaultStat.successPowerCooldown = 4;
			smallRaceStats.defaultStat.failurePowerCooldown = 8;


			//Short
			shortRaceStats.defaultStat.baseStagger = 3;
			shortRaceStats.defaultStat.basePrecision = 4;
			shortRaceStats.defaultStat.baseResist = 3;
			shortRaceStats.defaultStat.baseEvade = 5;


			shortRaceStats.defaultStat.firstBlowCost = 8;
			shortRaceStats.defaultStat.baseStaminaCost = 15;
			shortRaceStats.defaultStat.staminaCostClimb = 10;
			shortRaceStats.defaultStat.staminaHitCostMult = 0.5f;

			shortRaceStats.defaultStat.firstBlowPowerRestore = 0.75f;
			shortRaceStats.defaultStat.swingPowerRestore = 0.5f;
			shortRaceStats.defaultStat.hitPowerRestore = 2;

			shortRaceStats.defaultStat.firstBlowPowerCooldown = 6;
			shortRaceStats.defaultStat.successPowerCooldown = 4;
			shortRaceStats.defaultStat.failurePowerCooldown = 8;
		}

		static void LoadMediumRaceStats()
		{
			//medium
			mediumRaceStats.defaultStat.baseStagger = 4;
			mediumRaceStats.defaultStat.basePrecision = 4;
			mediumRaceStats.defaultStat.baseResist = 4;
			mediumRaceStats.defaultStat.baseEvade = 3;


			mediumRaceStats.defaultStat.firstBlowCost = 8;
			mediumRaceStats.defaultStat.baseStaminaCost = 15;
			mediumRaceStats.defaultStat.staminaCostClimb = 10;
			mediumRaceStats.defaultStat.staminaHitCostMult = 0.5f;

			mediumRaceStats.defaultStat.firstBlowPowerRestore = 0.75f;
			mediumRaceStats.defaultStat.swingPowerRestore = 0.5f;
			mediumRaceStats.defaultStat.hitPowerRestore = 2;

			mediumRaceStats.defaultStat.firstBlowPowerCooldown = 6;
			mediumRaceStats.defaultStat.successPowerCooldown = 4;
			mediumRaceStats.defaultStat.failurePowerCooldown = 8;


			//Tall
			tallRaceStats.defaultStat.baseStagger = 5;
			tallRaceStats.defaultStat.basePrecision = 4;
			tallRaceStats.defaultStat.baseResist = 5;
			tallRaceStats.defaultStat.baseEvade = 3;


			tallRaceStats.defaultStat.firstBlowCost = 8;
			tallRaceStats.defaultStat.baseStaminaCost = 15;
			tallRaceStats.defaultStat.staminaCostClimb = 10;
			tallRaceStats.defaultStat.staminaHitCostMult = 0.5f;

			tallRaceStats.defaultStat.firstBlowPowerRestore = 0.75f;
			tallRaceStats.defaultStat.swingPowerRestore = 0.5f;
			tallRaceStats.defaultStat.hitPowerRestore = 2;

			tallRaceStats.defaultStat.firstBlowPowerCooldown = 6;
			tallRaceStats.defaultStat.successPowerCooldown = 4;
			tallRaceStats.defaultStat.failurePowerCooldown = 8;
		}

		static void LoadLargeRaceStats()
		{
			//Large
			largeRaceStats.defaultStat.baseStagger = 6;
			largeRaceStats.defaultStat.basePrecision = 2;
			largeRaceStats.defaultStat.baseResist = 6;
			largeRaceStats.defaultStat.baseEvade = 2;


			largeRaceStats.defaultStat.firstBlowCost = 8;
			largeRaceStats.defaultStat.baseStaminaCost = 15;
			largeRaceStats.defaultStat.staminaCostClimb = 10;
			largeRaceStats.defaultStat.staminaHitCostMult = 0.5f;

			largeRaceStats.defaultStat.firstBlowPowerRestore = 0.75f;
			largeRaceStats.defaultStat.swingPowerRestore = 0.5f;
			largeRaceStats.defaultStat.hitPowerRestore = 2;

			largeRaceStats.defaultStat.firstBlowPowerCooldown = 6;
			largeRaceStats.defaultStat.successPowerCooldown = 4;
			largeRaceStats.defaultStat.failurePowerCooldown = 8;


			//Enormous
			enormousRaceStats.defaultStat.baseStagger = 6;
			enormousRaceStats.defaultStat.basePrecision = 1;
			enormousRaceStats.defaultStat.baseResist = 7;
			enormousRaceStats.defaultStat.baseEvade = 2;


			enormousRaceStats.defaultStat.firstBlowCost = 8;
			enormousRaceStats.defaultStat.baseStaminaCost = 15;
			enormousRaceStats.defaultStat.staminaCostClimb = 10;
			enormousRaceStats.defaultStat.staminaHitCostMult = 0.5f;

			enormousRaceStats.defaultStat.firstBlowPowerRestore = 0.75f;
			enormousRaceStats.defaultStat.swingPowerRestore = 0.5f;
			enormousRaceStats.defaultStat.hitPowerRestore = 2;

			enormousRaceStats.defaultStat.firstBlowPowerCooldown = 6;
			enormousRaceStats.defaultStat.successPowerCooldown = 4;
			enormousRaceStats.defaultStat.failurePowerCooldown = 8;
		}

		static void LoadExtraLargeRaceStats()
		{
			//extraLarge
			extraLargeRaceStats.defaultStat.baseStagger = 7;
			extraLargeRaceStats.defaultStat.basePrecision = 0.5f;
			extraLargeRaceStats.defaultStat.baseResist = 7;
			extraLargeRaceStats.defaultStat.baseEvade = 1;


			extraLargeRaceStats.defaultStat.firstBlowCost = 8;
			extraLargeRaceStats.defaultStat.baseStaminaCost = 15;
			extraLargeRaceStats.defaultStat.staminaCostClimb = 10;
			extraLargeRaceStats.defaultStat.staminaHitCostMult = 0.5f;

			extraLargeRaceStats.defaultStat.firstBlowPowerRestore = 0.75f;
			extraLargeRaceStats.defaultStat.swingPowerRestore = 0.5f;
			extraLargeRaceStats.defaultStat.hitPowerRestore = 2;

			extraLargeRaceStats.defaultStat.firstBlowPowerCooldown = 6;
			extraLargeRaceStats.defaultStat.successPowerCooldown = 4;
			extraLargeRaceStats.defaultStat.failurePowerCooldown = 8;


			//Massive
			massiveRaceStats.defaultStat.baseStagger = 8;
			massiveRaceStats.defaultStat.basePrecision = 0.5f;
			massiveRaceStats.defaultStat.baseResist = 8;
			massiveRaceStats.defaultStat.baseEvade = 1;


			massiveRaceStats.defaultStat.firstBlowCost = 8;
			massiveRaceStats.defaultStat.baseStaminaCost = 15;
			massiveRaceStats.defaultStat.staminaCostClimb = 10;
			massiveRaceStats.defaultStat.staminaHitCostMult = 0.5f;

			massiveRaceStats.defaultStat.firstBlowPowerRestore = 0.75f;
			massiveRaceStats.defaultStat.swingPowerRestore = 0.5f;
			massiveRaceStats.defaultStat.hitPowerRestore = 2;

			massiveRaceStats.defaultStat.firstBlowPowerCooldown = 6;
			massiveRaceStats.defaultStat.successPowerCooldown = 4;
			massiveRaceStats.defaultStat.failurePowerCooldown = 8;
		}
#pragma endregion RaceFunctions

		static void LoadRaceStats()
		{
			//At a later point, I would like to have options to make changes for specific creatures, attaching different stats to them.
			//This could be useful for different types of creatures that I may want to attach this to. I think it will have to be based
			// on specific reference, but to avoid multiples, perhaps you can make one, and then just label a race is "proxy" to another,
			// giving them the same Stats.defaultStat.

			//Contained in "RaceFunctions
			LoadNpcRaceStats();
			SharedStatContainer<RaceStat>::ShareContainer(smallRaceStats,
				shortRaceStats,
				mediumRaceStats,
				tallRaceStats,
				largeRaceStats,
				enormousRaceStats,
				extraLargeRaceStats,
				massiveRaceStats);

			LoadSmallRaceStats();
			LoadMediumRaceStats();
			LoadLargeRaceStats();
			LoadExtraLargeRaceStats();
		}

		static void LoadNonWeaponStats()
		{
			//logger::info("OFFSET: {}", sizeof(StatBase) + offsetof(MinimumStat, MinimumStat::nemesis));
			//shieldStats.defaultStat.header.SendOwner(&shieldStats.defaultStat, "shieldStats");
			//RE::DebugMessageBox(std::format("shield name: {}", result, shieldStats.defaultStat.header.alt_size, sizeof(MinimumSt));

			//shieldStats.defaultStat.baseStagger = 4;
			//shieldStats.defaultStat.basePrecision = 2;

			SharedStatContainer<ShieldStat>::ShareContainer(shieldStats, heavyShieldStats, lightShieldStats);
			//SharedStatContainer<ShieldStat>::ShareContainer(heavyShieldStats, lightShieldStats);
		}

		static void LoadUnarmedStats()
		{
			/*
			auto& name = unarmedStats.defaultStat.my_besto_furendo._valueSetting.name;
			auto& test_name = unarmedStats.defaultStat.my_besto_furendo.test_Name;

			//unarmedStats.defaultStat.SetName("Unarmed");
			auto* setting = RE::GameSettingCollection::GetSingleton()->GetSetting(unarmedStats.defaultStat.my_besto_furendo._valueSetting.name);

			RE::DebugMessageBox(std::format("{}, {}, setting:{}  == test{} <=> {}", setting->name, setting->data.f, reinterpret_cast<uintptr_t>(setting->name), reinterpret_cast<uintptr_t>(unarmedStats.defaultStat.my_besto_furendo.test_Name),
				reinterpret_cast<uintptr_t>(setting->name) == reinterpret_cast<uintptr_t>(unarmedStats.defaultStat.my_besto_furendo.test_Name)));
			//*/


			unarmedStats.defaultStat.baseStagger = 3.5f;
			unarmedStats.defaultStat.basePrecision = 3;
			unarmedStats.defaultStat.firstBlowCost = 8;
			unarmedStats.defaultStat.baseStaminaCost = 15;
			unarmedStats.defaultStat.staminaCostClimb = 10;
			unarmedStats.defaultStat.staminaHitCostMult = 0.5f;
			unarmedStats.defaultStat.firstBlowPowerRestore = 4;
			unarmedStats.defaultStat.swingPowerRestore = 2;
			unarmedStats.defaultStat.hitPowerRestore = 6;

			unarmedStats.defaultStat.firstBlowPowerCooldown = 6;
			unarmedStats.defaultStat.successPowerCooldown = 4;
			unarmedStats.defaultStat.failurePowerCooldown = 6;
		}

		//NOTE, the stamina climb values are too low I think. They're tailored around 100 stamina.


		//1 handed general rule (seemingly) hits dont matter as much for stamina consumption
		static void LoadDaggerStats()
		{
			daggerStats.defaultStat.baseStagger = 3;
			daggerStats.defaultStat.basePrecision = 6;
			daggerStats.defaultStat.firstBlowCost = 3;//I'm thinking, the base should be low, and inc high.
			daggerStats.defaultStat.baseStaminaCost = 4;
			daggerStats.defaultStat.staminaCostClimb = 8;
			daggerStats.defaultStat.staminaHitCostMult = 0.85f;
			daggerStats.defaultStat.firstBlowPowerRestore = 2.5f;
			daggerStats.defaultStat.swingPowerRestore = 2;
			daggerStats.defaultStat.hitPowerRestore = 3;

			//But because of this, it's combo times should be longer perhaps?
			daggerStats.defaultStat.firstBlowPowerCooldown = 4;//Dagger power attacks are to be very good openers.
			daggerStats.defaultStat.successPowerCooldown = 6;
			daggerStats.defaultStat.failurePowerCooldown = 10;
		}

		static void LoadSwordStats()
		{
			//Swords are more practical. Less committed because of their lack of weight.


			swordStats.defaultStat.baseStagger = 4;
			swordStats.defaultStat.basePrecision = 4.5f;
			swordStats.defaultStat.firstBlowCost = 3.f;
			swordStats.defaultStat.baseStaminaCost = 5.5f;//alt 8
			swordStats.defaultStat.staminaCostClimb = 10.f;//was 8.5
			swordStats.defaultStat.staminaHitCostMult = 0.75f;
			swordStats.defaultStat.firstBlowPowerRestore = 4;
			swordStats.defaultStat.swingPowerRestore = 2;
			swordStats.defaultStat.hitPowerRestore = 6;

			swordStats.defaultStat.firstBlowPowerCooldown = 10;
			swordStats.defaultStat.successPowerCooldown = 8;
			swordStats.defaultStat.failurePowerCooldown = 15;
		}

		static void LoadWarAxeStats()
		{
			//out of all weapons, THIS version of the axe should be really aggressive, and have a low climb (the other version
			// should value hits more)

			//comparitively, it's first blow is far less important. But can be used more moderately than the 2 handed variant (maybe?)

			//The cost to swing however, is quite high even while the increment isnt

			warAxeStats.defaultStat.baseStagger = 5;
			warAxeStats.defaultStat.basePrecision = 1.5f;
			warAxeStats.defaultStat.firstBlowCost = 5;
			warAxeStats.defaultStat.baseStaminaCost = 7;
			warAxeStats.defaultStat.staminaCostClimb = 2.f;
			warAxeStats.defaultStat.staminaHitCostMult = 0.9f;
			warAxeStats.defaultStat.firstBlowPowerRestore = 4;
			warAxeStats.defaultStat.swingPowerRestore = 7;
			warAxeStats.defaultStat.hitPowerRestore = 8;


			warAxeStats.defaultStat.firstComboTime = 0.65f;//I'm gonna play with the combo end times.
			warAxeStats.defaultStat.followComboTime = 0.85f;//This one should have a longer first blow window, but a shorter follow up.

			warAxeStats.defaultStat.penaltyComboTime = 2;//More in line with what axe is like.

			warAxeStats.defaultStat.firstBlowPowerCooldown = 15;
			warAxeStats.defaultStat.successPowerCooldown = 15;
			warAxeStats.defaultStat.failurePowerCooldown = 20;
		}

		static void LoadMaceStats()
		{
			//Maces are very power attack centric, and as such, there is a great importance on whether that power attack lands or not.
			// Though, this personality might be more 2 handed?

			//If so, this is still focused on power attacking, being less patient than it's 2 handed version
			// power attacks are better openers than before, and the first blow is far more focused on recover than anything else.
			// miss is more forgiving that hamme (who should be very patient).

			//To sum up, hammer of all things. Great reward for patience and better first blow usage for non-power attacking stuff,
			// but more penalty for missing.
			//Mace is less patient, and incurs less weaknesses for missing, but suffers from recovery in general, while still being heavily focused on
			// on power attacking.

			//^All this on mace is void for right now, I'm going to try the personality documents suggestions.



			//Here's how i interpret the pers document.
			// High stamina cost, but high power attack reduction. Don't play with the times. YET. Swing should be good on
			// power attack restoration, BUT not stamina. Be harsh on stamina on miss. First blow, is a bit weak, but shouldn't be
			// as weak as axes. 
			//Power attacks as first blow should be good.
			// subsequent attacks make the power attacks cost less, note, 2hand should be more because it's deal is "it's more patient"



			//Counter point, maces, are shit right now compared to great swords. On paper it looks like.
			maceStats.defaultStat.baseStagger = 6;
			maceStats.defaultStat.basePrecision = 1;
			maceStats.defaultStat.firstBlowCost = 8;
			maceStats.defaultStat.baseStaminaCost = 12;
			maceStats.defaultStat.staminaCostClimb = 15;
			maceStats.defaultStat.staminaHitCostMult = 0.55f;
			maceStats.defaultStat.firstBlowPowerRestore = 4;
			maceStats.defaultStat.swingPowerRestore = 7;
			maceStats.defaultStat.hitPowerRestore = 10;

			maceStats.defaultStat.powerAttackCostRate = 20;//Out of ALL the weapons, this is the one I want to experiment on this the most with.

			//Perhaps make this things first blow longer?
			maceStats.defaultStat.firstComboTime = 0.8f;//Additionally, I'm gonna play with the combo end times.

			//maceStats.defaultStat.firstComboTime = 0.65f;//Additionally, I'm gonna play with the combo end times.
			//maceStats.defaultStat.followComboTime = 1;//Maces end about half a second quicker. I'm not sure how to feel about this option though.


			maceStats.defaultStat.firstBlowPowerCooldown = 20;
			maceStats.defaultStat.successPowerCooldown = 18;
			maceStats.defaultStat.failurePowerCooldown = 25;//was 30, do want it to be high though.
		}

		//The stamina stuff for two handers should be harsher, because these are supposed to be harder to use.


		static void LoadGreatswordStats()
		{

			//This should likely prioritize first hits not caring and being very throw away, while subsequent hits are
			// expensive misses that leave you exposed, while their hits are cost effective.
			greatSwordStats.defaultStat.baseStagger = 6;
			greatSwordStats.defaultStat.basePrecision = 4;
			greatSwordStats.defaultStat.firstBlowCost = 5;
			greatSwordStats.defaultStat.baseStaminaCost = 8;
			greatSwordStats.defaultStat.staminaCostClimb = 10;
			greatSwordStats.defaultStat.staminaHitCostMult = 0.65f;//was 0.5
			greatSwordStats.defaultStat.firstBlowPowerRestore = 6;
			greatSwordStats.defaultStat.swingPowerRestore = 5;
			greatSwordStats.defaultStat.hitPowerRestore = 8;

			//An experiment on speed and recovery
			greatSwordStats.defaultStat.firstComboTime = 0.15f;
			greatSwordStats.defaultStat.followComboTime = 1.0f;


			greatSwordStats.defaultStat.firstBlowPowerCooldown = 20;
			greatSwordStats.defaultStat.successPowerCooldown = 18;
			greatSwordStats.defaultStat.failurePowerCooldown = 25;
		}

		static void LoadBattleAxeStats()
		{
			battleAxeStats.defaultStat.baseStagger = 7;
			battleAxeStats.defaultStat.basePrecision = 0.75f;
			battleAxeStats.defaultStat.firstBlowCost = 8;
			battleAxeStats.defaultStat.baseStaminaCost = 10;
			battleAxeStats.defaultStat.staminaCostClimb = 20;
			battleAxeStats.defaultStat.staminaHitCostMult = 0.35f;
			battleAxeStats.defaultStat.firstBlowPowerRestore = 4;
			//I want someone to keep swinging after missing to keep their momentum going. This is for that.
			battleAxeStats.defaultStat.swingPowerRestore = 7;
			battleAxeStats.defaultStat.hitPowerRestore = 8;

			//battleAxeStats.defaultStat.staminaLossRateClimb = 18.5f;//Testing this having an awful conversion rate.

			battleAxeStats.defaultStat.firstBlowPowerCooldown = 30;
			battleAxeStats.defaultStat.successPowerCooldown = 25;
			battleAxeStats.defaultStat.failurePowerCooldown = 35;
		}

		static void LoadWarhammerStats()
		{
			//From what I can tell, this weapons should be more patient, have a have a longer combo time on general, with
			// a rather standard first combo time.
			//Additionally, it's conversion rate is really good. But it's stamina costs are quite high.

			//Power attack first strikes are less good for first strikes, but in general and
			// proportionally are better than maces.

			//Additionally, hits you can wait out a lot more, and cure first blows with hits.
			// The misses can be cured with hits as well, but at a less rate.

			warhammerStats.defaultStat.baseStagger = 8;
			warhammerStats.defaultStat.basePrecision = 0.5f;
			warhammerStats.defaultStat.firstBlowCost = 5;
			warhammerStats.defaultStat.baseStaminaCost = 10;
			warhammerStats.defaultStat.staminaCostClimb = 15;
			warhammerStats.defaultStat.staminaHitCostMult = 0.45f;
			warhammerStats.defaultStat.firstBlowPowerRestore = 8;
			warhammerStats.defaultStat.swingPowerRestore = 4;
			warhammerStats.defaultStat.hitPowerRestore = 12;

			//warhammerStats.defaultStat.firstComboTime = 0.65f;
			warhammerStats.defaultStat.followComboTime = 2.35f;
			warhammerStats.defaultStat.penaltyComboTime = 4.75f;//Quite lengthy.

			warhammerStats.defaultStat.powerAttackCostRate = 20;//This ones will be the same as it's lesser, dont know what I'll do with it though.
			warhammerStats.defaultStat.staminaLossRateClimb = 5.f;//Much lower than most. Temp likely, but giving it a go.

			//I'm finding it tou
			warhammerStats.defaultStat.firstBlowPowerCooldown = 25;
			warhammerStats.defaultStat.successPowerCooldown = 15;
			warhammerStats.defaultStat.failurePowerCooldown = 30;

			//Less expensive, I'll alter it over time, but it's a big who cares for now.
			//Thinking of generally lowering the cooldown, to match it's patient play style, being able to wait out mostly anything,
			// to provoke a reaction. This sort of if you miss style of thing is going to battle axe I believe.
		}

		static void LoadBowStats()
		{
			bowStats.defaultStat.baseStagger = 6;//8;
			bowStats.defaultStat.basePrecision = 3;
			bowStats.defaultStat.baseStaminaCost = 10;
			bowStats.defaultStat.staminaCostClimb = 2;
		}

		static void LoadCrossbowStats()
		{
			crossbowStats.defaultStat.baseStagger = 8;
			crossbowStats.defaultStat.basePrecision = 2;
			crossbowStats.defaultStat.baseStaminaCost = 20;
			crossbowStats.defaultStat.staminaCostClimb = 0;
		}

		static void LoadMagicStats()
		{
			//note, the shout has to do damage to be considered, with
			//I'm going with 6, as with 2 more it will 



			shoutMagicStats.defaultStat.baseStagger = 6;


			SharedStatContainer<MagicSpellStat>::ShareContainer(noviceMagicStats,
				apprenticeMagicStats,
				adeptMagicStats,
				expertMagicStats,
				masterMagicStats,
				ritualMagicStats);


			noviceMagicStats.defaultStat.baseStagger = 3;
			apprenticeMagicStats.defaultStat.baseStagger = 4;
			adeptMagicStats.defaultStat.baseStagger = 5;
			expertMagicStats.defaultStat.baseStagger = 6;
			masterMagicStats.defaultStat.baseStagger = 8;
			ritualMagicStats.defaultStat.baseStagger = 12;


			//basePrecision is in alpha, values are not final
			magicProjectileStats.defaultStat.concMagicPrecision = 0.5f;//Was 1
			magicProjectileStats.defaultStat.beamMagicPrecision = 3.5f;//Was 4
			magicProjectileStats.defaultStat.missileMagicPrecision = 2;//same
			magicProjectileStats.defaultStat.coneMagicPrecision = 1.5f;//was 4 or some shit.
			magicProjectileStats.defaultStat.arrowMagicPrecision = 2.5f;//was 3
			magicProjectileStats.defaultStat.lobberMagicPrecision = 100;
		}


		static StatType TranslateRequest(RE::MagicItem* a_magic, StatType request)
		{
			if (!a_magic)
				return request;

			switch (request)
			{
			case basePrecision:
			case concMagicPrecision:
			case beamMagicPrecision:
			case missileMagicPrecision:
			case lobberMagicPrecision:
			case coneMagicPrecision:
			case arrowMagicPrecision:
			{
				ProjectileType proj;
				if (Utility::GetProjectileType(a_magic, proj) == true)
				{
					switch (proj)
					{
					case ProjectileType::kMissile:
						return missileMagicPrecision;
					case ProjectileType::kGrenade:
						return lobberMagicPrecision;
					case ProjectileType::kBeam:
						return beamMagicPrecision;
					case ProjectileType::kFlamethrower:
						return concMagicPrecision;
					case ProjectileType::kCone:
						return coneMagicPrecision;
						//case ProjectileType::kBarrier:
					case ProjectileType::kArrow:
						return arrowMagicPrecision;
					}
				}

				break;
			}

			case baseStagger:
				//In the future cases of asking for stagger, I will want this to evaluate the magic item,
				// asking if it's a shout and evaluating it via that method. This might go somewhere else.
			default:
				return request;
			}


			return request;
			//if the request is anything but 
		}

	public:
		static void Initialize()
		{
			LoadRaceStats();
			LoadNonWeaponStats();
			LoadUnarmedStats();
			LoadDaggerStats();
			LoadSwordStats();
			LoadWarAxeStats();
			LoadMaceStats();
			LoadGreatswordStats();
			LoadBattleAxeStats();
			LoadWarhammerStats();
			LoadBowStats();
			LoadCrossbowStats();
			LoadMagicStats();
		}

		//The race data is associated with the size of the creature, as well as their base mass.
		// if their base mass is greater than their size setting + 2 they are given the stats of a race higher.
		// If their weight falls below a certain amount, then they will get the smallest weight instead. (0.5 seems to be it)
		//More notes, race data is also associated with how input from unarmed is treated.
		static inline StatContainer<RaceStat> npcRaceStats = "Humanoid";//Default for all humanoid races

		static inline SharedStatContainer<RaceStat> smallRaceStats = "Small";
		static inline SharedStatContainer<RaceStat> shortRaceStats = "Short";

		static inline SharedStatContainer<RaceStat> mediumRaceStats = "Medium";
		static inline SharedStatContainer<RaceStat> tallRaceStats = "Tall";

		static inline SharedStatContainer<RaceStat> largeRaceStats = "Large";
		static inline SharedStatContainer<RaceStat> enormousRaceStats = "Enormous";

		//Very Large, massive will likely never be used.
		static inline SharedStatContainer<RaceStat> extraLargeRaceStats = "ExtraLarge";
		static inline SharedStatContainer<RaceStat> massiveRaceStats = "Massive";


		//This will get split into a few different ones, but the precision data stays grouped.
		//Perhaps include a space for explosions, if I ever plan to make them dodgable.
		static inline SharedStatContainer<MagicSpellStat> noviceMagicStats = "Novice";
		static inline SharedStatContainer<MagicSpellStat> apprenticeMagicStats = "Apprentice";
		static inline SharedStatContainer<MagicSpellStat> adeptMagicStats = "Adept";
		static inline SharedStatContainer<MagicSpellStat> expertMagicStats = "Expert";
		static inline SharedStatContainer<MagicSpellStat> masterMagicStats = "Master";
		static inline SharedStatContainer<MagicSpellStat> ritualMagicStats = "Ritual";

		static inline StatContainer<MagicSpellStat> shoutMagicStats = "Shout";
		static inline StatContainer<MagicProjStat> magicProjectileStats = "MagicProjectile";


		//thinking of splitting this quite a bit.

		//static inline ShieldStat someShitAboutWards;
		
		static inline SharedStatContainer<ShieldStat> lightShieldStats = "LightShield";
		static inline SharedStatContainer<ShieldStat> heavyShieldStats = "HeavyShield";
		static inline SharedStatContainer<ShieldStat> shieldStats = "NonArmorShield";
		//static inline StatContainer<ShieldStat> lightShieldStats;
		//static inline StatContainer<ShieldStat> heavyShieldStats;
		//static inline StatContainer<ShieldStat> shieldStats;//Delete me for detection.

		//Remove this.
		static inline StatContainer<MeleeStat> unarmedStats = "Unarmed";
		static inline StatContainer<MeleeStat> daggerStats = "Dagger";

		static inline StatContainer<MeleeStat> swordStats = "Sword";
		static inline StatContainer<MeleeStat> warAxeStats = "Waraxe";
		static inline StatContainer<MeleeStat> maceStats = "Mace";

		static inline StatContainer<MeleeStat> greatSwordStats = "Greatsword";
		static inline StatContainer<MeleeStat> battleAxeStats = "Battleaxe";
		static inline StatContainer<MeleeStat> warhammerStats = "Warhammer";

		static inline StatContainer<RangedStat> bowStats = "Bow";
		static inline StatContainer<RangedStat> crossbowStats = "Crossbow";




		static StatBase* GetMagicStatBase(RE::MagicItem* magItem)
		{
			if (!magItem)
				return nullptr;

			switch (Utility::GetMagicSkill(magItem))
			{
			case MagicSkill::kNovice:
				return noviceMagicStats.GetStatBase(magItem);
			case MagicSkill::kApprentice:
				return apprenticeMagicStats.GetStatBase(magItem);
			case MagicSkill::kAdept:
				return adeptMagicStats.GetStatBase(magItem);
			case MagicSkill::kExpert:
				return expertMagicStats.GetStatBase(magItem);
			case MagicSkill::kMaster:
				return masterMagicStats.GetStatBase(magItem);
			case MagicSkill::kRitual:
				return ritualMagicStats.GetStatBase(magItem);
			}
		}

		static StatBase* GetWeaponStatBase(RE::TESObjectWEAP* weap)
		{
			if (!weap)
				return nullptr;

			auto type = Utility::GetWeaponType(weap);

			switch (type)
			{
			default:
			case WeaponType::kHandToHandMelee:
				//I'm debating this because of brawl fists. perhaps measure their weight first.
				//That or, it muse have a use skills
				if (weap->GetPlayable() == false)
					return npcRaceStats.GetStatBase(weap);//If not playable, treat it 
				else
					return unarmedStats.GetStatBase(weap);;

			case WeaponType::kOneHandSword:
				return swordStats.GetStatBase(weap);
			case WeaponType::kOneHandDagger:
				return daggerStats.GetStatBase(weap);
			case WeaponType::kOneHandAxe:
				return warAxeStats.GetStatBase(weap);
			case WeaponType::kOneHandMace:
				return maceStats.GetStatBase(weap);
			case WeaponType::kTwoHandSword:
				return greatSwordStats.GetStatBase(weap);
			case WeaponType::kTwoHandAxe:
				return battleAxeStats.GetStatBase(weap);
			case WeaponType::kBow:
				return bowStats.GetStatBase(weap);
			case WeaponType::kStaff:
				return GetMagicStatBase(weap->formEnchanting);
			case WeaponType::kCrossbow:
				return crossbowStats.GetStatBase(weap);
			case WeaponType::kTwoHandMace:
				return warhammerStats.GetStatBase(weap);
			}
		}

		static StatBase* GetRaceStatBase(RE::TESRace* race)
		{
#define force_small_epsilon 0.5f


			if (!race)
				return nullptr;
			//at a later point, this MIGHT take actorbase, so it can see what the sex is, and use that value instead
			// In doing this, races that have much large females or males can have variation. Not for npcs though.
			// Will either use this, or BodyPartData, but I think I may do it based on project instead.
			if (race->HasKeywordString("ActorTypeNPC") == true)
			{
				return npcRaceStats.GetStatBase(race);
			}
			else
			{
				float weight = fmin(race->data.weight[0], race->data.weight[1]);

				if (weight <= force_small_epsilon)//Chickens are fucking weird. So idk sorry.
					return smallRaceStats.GetStatBase(race);

				RE::RACE_SIZE size = *race->data.raceSize;
				float mass = race->data.baseMass;//fmax(race->weight[0], race->weight[1]);
				bool plusClass = mass >= fmin(static_cast<int>(size) + 2, 4);//mainly for mammoth and dragoonz

				switch (size)
				{
				case RE::RACE_SIZE::kSmall:
					return !plusClass ? smallRaceStats.GetStatBase(race) : shortRaceStats.GetStatBase(race);
				case RE::RACE_SIZE::kMedium:
					return !plusClass ? mediumRaceStats.GetStatBase(race) : tallRaceStats.GetStatBase(race);
				case RE::RACE_SIZE::kLarge:
					return !plusClass ? largeRaceStats.GetStatBase(race) : enormousRaceStats.GetStatBase(race);
				case RE::RACE_SIZE::kExtraLarge:
					return !plusClass ? extraLargeRaceStats.GetStatBase(race) : massiveRaceStats.GetStatBase(race);
				}
			}

			return nullptr;
		}

#ifdef should_should_get_containers
		static StatBase* GetMagicStatBase(RE::MagicItem* magItem)
		{
			if (!magItem)
				return nullptr;

			switch (Utility::GetMagicSkill(magItem))
			{
			case MagicSkill::kNovice:
				return &noviceMagicStats;
			case MagicSkill::kApprentice:
				return &apprenticeMagicStats;
			case MagicSkill::kAdept:
				return &adeptMagicStats;
			case MagicSkill::kExpert:
				return &expertMagicStats;
			case MagicSkill::kMaster:
				return &masterMagicStats;
			case MagicSkill::kRitual:
				return &ritualMagicStats;
			}
		}

		static StatBase* GetWeaponStatBase(RE::TESObjectWEAP* weap)
		{
			if (!weap)
				return nullptr;

			auto type = Utility::GetWeaponType(weap);

			switch (type)
			{
			default:
			case WeaponType::kHandToHandMelee:
				//I'm debating this because of brawl fists. perhaps measure their weight first.
				//That or, it muse have a use skills
				return &npcRaceStats;
				//return &unarmedStats;//There is a specific branch for unarmed anim weapons.
			case WeaponType::kOneHandSword:
				return &swordStats;
			case WeaponType::kOneHandDagger:
				return &daggerStats;
			case WeaponType::kOneHandAxe:
				return &warAxeStats;
			case WeaponType::kOneHandMace:
				return &maceStats;
			case WeaponType::kTwoHandSword:
				return &greatSwordStats;
			case WeaponType::kTwoHandAxe:
				return &battleAxeStats;
			case WeaponType::kBow:
				return &bowStats;
			case WeaponType::kStaff:
				return GetMagicStatBase(weap->formEnchanting);
			case WeaponType::kCrossbow:
				return &crossbowStats;
			case WeaponType::kTwoHandMace:
				return &warhammerStats;
			}
		}

		static StatBase* GetRaceStatBase(RE::TESRace* race)
		{
#define force_small_epsilon 0.5f


			if (!race)
				return nullptr;
			//at a later point, this MIGHT take actorbase, so it can see what the sex is, and use that value instead
			// In doing this, races that have much large females or males can have variation. Not for npcs though.
			// Will either use this, or BodyPartData, but I think I may do it based on project instead.
			if (race->HasKeywordString("ActorTypeNPC") == true)
			{
				return &npcRaceStats;
			}


			else
			{
				float weight = fmin(race->data.weight[0], race->data.weight[1]);

				if (weight <= force_small_epsilon)//Chickens are fucking weird. So idk sorry.
					return &smallRaceStats;

				RE::RACE_SIZE size = *race->data.raceSize;
				float mass = race->data.baseMass;//fmax(race->weight[0], race->weight[1]);
				bool plusClass = mass >= fmin(static_cast<int>(size) + 2, 4);//mainly for mammoth and dragoonz

				switch (size)
				{
				case RE::RACE_SIZE::kSmall:
					return !plusClass ? &smallRaceStats : &shortRaceStats;
				case RE::RACE_SIZE::kMedium:
					return !plusClass ? &mediumRaceStats : &tallRaceStats;
				case RE::RACE_SIZE::kLarge:
					return !plusClass ? &largeRaceStats : &enormousRaceStats;
				case RE::RACE_SIZE::kExtraLarge:
					return !plusClass ? &extraLargeRaceStats : &massiveRaceStats;
				}
			}

			return nullptr;
		}
#endif

		static StatBase* GetStatBase(RE::TESForm* a_form)
		{
			//you can keep one of these around if you'd like, they don't go anywhere and it may be easier to just access the thing
			// and keep it around for a process.

			if (!a_form)
				return nullptr;

			switch (a_form->GetFormType())
			{
			case RE::FormType::Weapon:
				return GetWeaponStatBase(a_form->As<RE::TESObjectWEAP>());

			case RE::FormType::Spell:
			case RE::FormType::Scroll:
			case RE::FormType::Enchantment:
				return GetMagicStatBase(a_form->As<RE::MagicItem>());

			case RE::FormType::ActorCharacter:
				a_form = a_form->As<RE::Actor>()->GetRace();

			case RE::FormType::Race:
				return GetRaceStatBase(a_form->As<RE::TESRace>());

			case RE::FormType::Armor:
				if (auto shield = a_form->As<RE::TESObjectARMO>(); shield->IsShield() == true) {
					if (shield->IsHeavyArmor() == true)
						return heavyShieldStats.GetStatBase(a_form);
					else if (shield->IsLightArmor() == true)
						return lightShieldStats.GetStatBase(a_form);
					else
						return shieldStats.GetStatBase(a_form);
				}
					

			}

			return nullptr;
		}

		template<std::derived_from<StatBase> StatBaseType>
		static inline StatBaseType* GetStatBase(RE::TESForm* a_form)
		{
			auto* stat_base = GetStatBase(a_form);

			if (stat_base)
				return dynamic_cast<StatBaseType*>(stat_base);

			return nullptr;
		}


		static float GetStatValue(RE::TESForm* a_form, StatType type)
		{
			if (!a_form)
				return -1;

			auto statBase = GetStatBase(a_form);

			if (!statBase)
				return -1;

			if (a_form)
				type = TranslateRequest(a_form->As<RE::MagicItem>(), type);

			return statBase->GetValue(type);
		}

	};
}