#include "ReadableValue.h"


#include "Utility.h"
#include "CombatData.h"
#include "CombatResource.h"
#include "CombatResourceOwner.h"

namespace Crucible
{

	RE::ActorValue ReadableValue::AsActorValue()
	{
		return static_cast<RE::ActorValue>(_value);
	}

	CombatResource ReadableValue::AsResource()
	{
		return static_cast<CombatResource>(_value);
	}

	void ReadableValue::SetValue(std::int32_t new_value)
	{
		//int function_value = (new_value >> 27);//Was gonna XOR the function bit but fuck it amirite?

		if ((new_value & function_bit) != 0)
		{
			_isFunction = true;

			_includeDamage = new_value & damage_bit;
			_includeTemporary = new_value & temporary_bit;
			_includePermenant = new_value & permenant_bit;
			_isActorValue = new_value & actorvalue_bit;
			new_value &= ~extra_bits;
		}
		else
		{
			_isFunction = false;
			_includeDamage = false;
			_includeTemporary = false;
			_includePermenant = false;
		}

		_value = new_value;
	}


	std::int32_t ReadableValue::FlagsToInt(IncludeFlags flags)
	{

		int iFlags = static_cast<int>(flags);

		constexpr int first_bit = 27;


		return iFlags << 27;



		int result = 0;

		int i = 1;

		while (i <= iFlags)
		{
			if ((i & iFlags) != 0)
				result += i;

			i << 1;
		}

		//for (int i = 1; i <= iFlags; i * 2){}

		return result;
	}

	void ReadableValue::SetValue(CombatResource cr, IncludeFlags flags)
	{
		auto to_send = static_cast<std::int32_t>(cr);

		return SetValue(FlagsToInt(flags) | to_send | sign_bit);

	}

	void ReadableValue::SetValue(RE::ActorValue av, IncludeFlags flags)
	{
		auto to_send = static_cast<std::int32_t>(av);

		return SetValue(FlagsToInt(flags) | to_send | sign_bit | actorvalue_bit);
	}


	//It's viable for these 2 to just exist here
	float ReadableValue::GetActorValue(RE::Actor* actor)
	{
		float result = 0.f;

		if (!actor)
			return result;

		auto av = AsActorValue();

		result = actor->GetBaseActorValue(av);

		if (_includeDamage)
			result += actor->GetActorValueModifier(AVModifier::kDamage, av);

		if (_includeTemporary)
			result += actor->GetActorValueModifier(AVModifier::kTemporary, av);

		if (_includePermenant)
			result += actor->GetActorValueModifier(AVModifier::kPermanent, av);

		return result;
	}
	float ReadableValue::GetCombatResource(CombatData* data)
	{
		float result = 0.f;

		if (!data)
			return result;

		auto cr = AsResource();

		result = data->GetResourceBase(cr);

		if (_includeDamage)
			result += data->GetResourceDamage(cr);

		if (_includeTemporary)
			result += data->GetResourceModifier(cr);

		//if (_includePermenant)//This has no permenant value, rather this is merged with temp.
		//	result += actor->GetActorValueModifier(AVModifier::kPermanent, av);

		return result;
	}



	float ReadableValue::GetValue(RE::Actor* actor)
	{
		if (!_isFunction)
			return _value;

		if (IsActorValue() == true)
		{
			return GetActorValue(actor);
		}
		else
		{
			auto data = CombatData::GetDataRecord(actor);
			return GetCombatResource(data);
		}
	}

	float ReadableValue::GetValue(CombatData* data)
	{
		if (!_isFunction)
			return _value;

		if (!data)
			return 0.f;

		if (IsCombatResource() == true)
			return GetCombatResource(data);
		else
			return GetValue(data->GetOwner());
	}


	bool ReadableValue::ModDistantValue(RE::Actor* actor, float value)
	{
		if (!actor)
			return false;

		if (IsActorValue() == true)
		{
			actor->RestoreActorValue(AVModifier::kDamage, AsActorValue(), -value);
		}
		else
		{
			auto data = CombatData::GetDataRecord(actor);
			return ModDistantValue(data, value);
		}

		return true;
	}

	bool ReadableValue::ModDistantValue(CombatData* data, float value)
	{
		if (!data)
			return false;

		if (IsActorValue() == true)
		{
			ModDistantValue(data->GetOwner(), value);
		}
		else
		{
			data->RestoreResource(AsResource(), value);
		}

		return true;
	}
}

