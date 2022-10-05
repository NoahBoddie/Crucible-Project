#pragma once

namespace Crucible
{

	enum class StrikeCategory
	{
		kNone = 0, //DO NOT FUCKING USE plz. Convient for clearing, not supposed to be reserved.
		kEvadeDefend = 1 << 0,
		kResistDefend = 1 << 1,
		kResistAttack = 1 << 2,
		kEvadeAttack = 1 << 3
	};

	enum class StrikeResult
	{
		none = 0,
		minorAttack = 1 << 0,
		majorAttack = 1 << 1,
		minorDefend = 1 << 2,
		majorDefend = 1 << 3,
		allAttack = minorAttack | majorAttack,
		allDefend = minorDefend | majorDefend,
		allMinor = minorAttack | minorDefend,
		allMajor = majorAttack | majorDefend,
		exceptMinorAttack = majorDefend | majorAttack | minorDefend,
		exceptMajorAttack = majorDefend | minorAttack | minorDefend,
		exceptMinorDefend = minorAttack | majorAttack | majorDefend,
		exceptMajorDefend = minorAttack | majorAttack | minorDefend,
		total = minorAttack | majorAttack | minorDefend | majorDefend
	};

}