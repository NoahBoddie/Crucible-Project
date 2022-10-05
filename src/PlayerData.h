#pragma once
#include "CombatData.h"

namespace Crucible
{	
	/// <summary>
	/// This class is in charge of holding onto player specific data, and unlike the maps for other combat data, this one has a singleton
	/// one can pull should they need operation instead.
	/// </summary>
	class PlayerData : public CombatData
	{
	public:
		static PlayerData* GetSingleton();
	};


	
}