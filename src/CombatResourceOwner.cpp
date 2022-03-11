#pragma

#include "ItemStatHandler.h"
#include "CombatResourceOwner.h"
#include "FormulaHandler.h"

namespace Crucible
{
	float CombatResourceOwner::GetResource(CombatResource resource)
	{
		float result = GetResourceMax(resource);

		float dif = result - GetResourceDamage(resource);

		result = dif > result ?
			result : dif < 0 ?
			0 : dif;
		return result;
	}

	float CombatResourceOwner::GetResourceBase(CombatResource resource)
	{
		//*
		auto& source = resourceContainer[resource];
		auto& baseValue = source.baseValue;
		if (baseValue.IsCombatResource() && baseValue.AsResource() == resource) {
			//report stack overflow
			return -1;
		}

		float value = baseValue.GetValue(GetOwner());

		return FormulaHandler::UseFormular(source.formular, value, GetOwner());
		//*/
		//return resourceContainer[resource].GetBase(GetOwner());
	}
	
	float CombatResourceOwner::GetResourceMax(CombatResource resource)
	{
		return GetResourceBase(resource) + GetResourceModifier(resource);
	}

	float CombatResourceOwner::GetResourceModifier(CombatResource resource)
	{
		return resourceContainer[resource].modifierValue;
	}

	float CombatResourceOwner::GetResourceDamage(CombatResource resource)
	{
		return resourceContainer[resource].damageValue;
	}


	//Since you could direct access these, these should be convienience functions to prevent them from passing a point
	// if relevant.
	void CombatResourceOwner::DamageResource(CombatResource resource, float damage) 
	{
		resourceContainer[resource].damageValue += damage;
	}
	void CombatResourceOwner::RestoreResource(CombatResource resource, float restore)
	{
		resourceContainer[resource].damageValue -= restore;

		if (0 > resourceContainer[resource].damageValue)
		{
			resourceContainer[resource].damageValue = 0;
		}
	}
	void CombatResourceOwner::ModResource(CombatResource resource, float mod)
	{
		resourceContainer[resource].modifierValue += mod;
	}
}