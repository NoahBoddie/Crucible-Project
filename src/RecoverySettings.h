#pragma once

#include "ModSettings.h"
#include "ReadableValue.h"

namespace Crucible
{
	enum class RecoverValue : std::uint8_t
	{
		kGeneric, //There is currently none.
		kPoise,
		kStamina,
		kMagicka,
		kSpeed,
		//recovery value for the recovery resource, aint that a tongue twister?
		kStaminaRecover,
		kMagickaRecover
	};

	struct RecoverySetting : public SettingTemplate<RecoverValue>
	{
	public:
		ReadableValue regenValue;
		float recoveryRate;
		float recoveryMin;
		bool isPool;
		bool pauseClear = true;//All things will want to do this for now, I'm too lazy to set it up speciifically.
	};
}