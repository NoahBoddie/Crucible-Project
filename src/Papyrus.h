#pragma once
#include "Utility.h"
#include "CombatData.h"

namespace Crucible
{
	class Papyrus
	{
	public:
		static bool Install(SkyrimVM* a_vm)
		{
			a_vm->RegisterFunction("SetEffectRegister", "_CRC_Effect_Base", SetEffectRegister);

			return true;
		}

		//registers active effects to be watched, they must have a special id to do so though.
		static inline bool SetEffectRegister(SkyrimVM* a_vm, RE::VMStackID a_stackID, RE::ActiveEffect* a_effect, bool apply)
		{
			if (!a_effect)
			{
				return false;
			}

			if (a_effect->usUniqueID == 0)
			{
				//cannot use non unique effects.
				return false;
			}

			auto actor = skyrim_cast<RE::Actor*>(a_effect->target);

			if (!actor)
			{
				Utility::DebugMessageBox("No actor for effect register");
				return false;
			}

			auto data = CombatData::GetDataRecord(actor->GetFormID());

			if (!data)
				return false;

			if (apply && data->currentSpecialID == 0)
			{
				data->currentSpecialID = a_effect->usUniqueID;
			}
			else if (!apply && data->currentSpecialID == a_effect->usUniqueID)
			{
				data->currentSpecialID = 0;
			}
			else
				return false;

			return true;
		}

	};
	

}