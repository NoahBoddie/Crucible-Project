#pragma once

#include "ModSettings.h"

namespace Crucible
{
	enum class StrikeThreshold
	{
		offenseEvade,
		defenseEvade,
		offenseHit,
		defenseHit
	};

	struct StrikeSetting : public SettingTemplate<StrikeThreshold>
	{
		float minorThreshold;
		float majorThreshold;
	};

}