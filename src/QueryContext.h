#pragma once
namespace Crucible
{
//I spelled it wrong, too lazy to change.
#define queryAggressor QueryFunction::queryAgressor
//#define queryPostAggressorPoise QueryFunction::queryPostAgressorPoise

	//Want a query type that basically says no conflict.
	enum QueryFunction
	{
		queryRaw,				//This is generally false by default. But the presence of this is generally for visuals.
		querySource,			//This is for run on's with conditions, doesn't replace manual form placing.
		queryProjectile,		//If there is a projectile, this is where it will be stored. Probably.
		queryGuardSource,		//The defending object, casted ward/wards, shield, fists, etc.
		queryAgressor,
		queryDefender,
		queryBash,				//If the attack was a bash attack, this will be filled out.
		queryTimed,				//If the bash is timed, this will be true.
		queryCounter,				//If it was a bash, and it was counter viable on land, it's a counter.
		queryBlocked,
		querySneakAttack,
		queryPowerAction,
		queryDistance,			//Will most likely be used for effective range, but is also used for action calculation.
		queryAggressorRange,	//The range of the Aggressor. Currently unused.
		queryDefenderRange,		//The range of the Defender. Currently unused.
		//Post Queries			//<!> The values assigned during or after a query context has been processed to avoid repeat calls.
		queryPostStagger,
		queryPostPrecision,
		queryPostResist,
		queryPostEvade,
		//queryPostPoiseDamage,	//Is the amount of poise damage before calculation. This will be set on completion.
		//queryPostHealthDamage,	//Similar to the above, this handles the value for damage, before and after.
		//queryPostMagicDamage,	//The magic weakness value. Inc for resist, dec for weakness.
		//queryPostReflectDamage,	//Sent on hit to symbolize reflect damage.

		//queryPostPoiseMult,	//Same as above, but instead is a mult used after the above to apply to 
		//queryPostDamageMult,	//When the query is involved in damaging action, this mult judges how much damage should be done.
		//queryPostResultValue,	//The resulting value, minor percent, or the major difference associated with the action performed.
		kTemp
	};

	enum class QueryValidation
	{
		formQuery = 1,
		boolQuery = 2,
		floatQuery = 3,
		intQuery = 4
	};
	
	struct QueryParameter
	{
	public:

		QueryValidation flag;

		union
		{
			RE::FormID		formValue;	//32 bits
			bool			boolValue;	//1 bit
			float			floatValue;	//32 bits
			std::int32_t	intValue;	//32 bits, same as form id

		};
	};

	class QueryContext
	{
	private:

		std::map<QueryFunction, QueryParameter> functionMap;
		bool _isAggressorActor = false;
		bool _playerInvolved = false;

	public:

		bool empty() { return functionMap.size() == 0; }
		
		bool const& isPlayerInvolved = _playerInvolved;
		bool const& isAggressorActor = _isAggressorActor;

		void SetFloat(QueryFunction function, float value)
		{
			auto entry = functionMap.find(function);

			if (functionMap.end() != entry)
			{
				if (entry->second.flag != QueryValidation::floatQuery)
					return;
			}

			QueryParameter parameter;

			parameter.flag = QueryValidation::floatQuery;
			parameter.floatValue = value;

			functionMap[function] = parameter;
		}

		void SetBool(QueryFunction function, bool value)
		{
			auto entry = functionMap.find(function);

			if (functionMap.end() != entry)
			{
				if (entry->second.flag != QueryValidation::boolQuery)
					return;
			}

			QueryParameter parameter;

			parameter.flag = QueryValidation::boolQuery;
			parameter.boolValue = value;

			functionMap[function] = parameter;
		}

		void SetInt(QueryFunction function, std::int32_t value)
		{
			auto entry = functionMap.find(function);

			if (functionMap.end() != entry)
			{
				if (entry->second.flag != QueryValidation::intQuery)
					return;
			}

			QueryParameter parameter;

			parameter.flag = QueryValidation::intQuery;
			parameter.intValue = value;

			functionMap[function] = parameter;
		}

		void SetForm(QueryFunction function, RE::TESForm* value)
		{
			RE::FormID formID = value ? value->GetFormID() : 0;
			
			SetForm(function, formID);
		}

		void SetForm(QueryFunction function, RE::FormID value)
		{
			if (!value) {
			
				//Utility::DebugMessageBox("A1");
				return;
			
			}
			auto entry = functionMap.find(function);

			if (functionMap.end() != entry)
			{
				if (entry->second.flag != QueryValidation::formQuery) {
					//Utility::DebugMessageBox("A2");
					return;
				}
			}

			QueryParameter parameter;

			parameter.flag = QueryValidation::formQuery;
			parameter.formValue = value;

			functionMap[function] = parameter;

			switch (function)
			{
			case QueryFunction::queryAgressor:
			{
				auto aggressorForm = RE::TESForm::LookupByID(value);
				_isAggressorActor = aggressorForm ? aggressorForm->GetFormType() == RE::FormType::ActorCharacter : _isAggressorActor;
			}
			case QueryFunction::queryDefender:
				if (value == 0x00000014)//IE, if we are setting one and ist's the player
					_playerInvolved = true;
			}
		}



		
		float GetFloat(QueryFunction function, bool force = false)
		{
			if (functionMap.contains(function) == false)
				return 0.f;

			QueryParameter& param = functionMap[function];

			if (!force && param.flag != QueryValidation::floatQuery)
				return 0.f;

			return param.floatValue;
		}
		
		bool GetBool(QueryFunction function, bool force = false)
		{
			if (functionMap.contains(function) == false)
				return false;

			QueryParameter& param = functionMap[function];

			if (!force && param.flag != QueryValidation::boolQuery)
				return false;

			return param.boolValue;
		}

		std::int32_t GetInt(QueryFunction function, bool force = false)
		{
			if (functionMap.contains(function) == false)
				return 0;

			QueryParameter& param = functionMap[function];

			if (!force && param.flag != QueryValidation::floatQuery)
				return 0;

			return param.floatValue;
		}

		RE::FormID GetFormID(QueryFunction function, bool force = false)
		{
			if (functionMap.contains(function) == false)
				return 0;

			QueryParameter& param = functionMap[function];

			if (!force && param.flag != QueryValidation::formQuery)
				return 0;

			return param.formValue;
		}

		RE::TESForm* GetForm(QueryFunction function, bool force = false)
		{
			RE::FormID formID = GetFormID(function, force);

			return RE::TESForm::LookupByID(formID);
		}

		template <class T>
		T* GetForm(QueryFunction function)
		{
			auto form = GetForm(function);

			return form ? form->As<T>() : nullptr;
		}

		bool HasValue(QueryFunction function)
		{
			return functionMap.contains(function);
		}
	};

}