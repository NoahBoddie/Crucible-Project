#pragma once

#include "QueryContext.h"
#include "CombatValue.h"



namespace Crucible
{
	class CombatData;

	//Query context should ALWAYS come with the target actor of the query. Either as the attacker or defender.

	//One central one?
	class CombatValueOwner
	{
	public:
		static CombatValueOwner* GetObjectValueOwner()
		{
			static CombatValueOwner value_owner_object;
			return &value_owner_object;
		}

		virtual float GetStagger(RE::TESForm* a_form);

		virtual float GetPrecision(RE::TESForm* a_form);

		virtual float GetResist();

		virtual float GetEvade();


		//For these apply lists, I think a reference might be a good idea.

		virtual float GetStagger(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr);

		virtual float GetPrecision(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr);

		virtual float GetResist(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr);

		virtual float GetEvade(QueryContext& context, std::list<CombatValueEffect*>* applyValueList = nullptr);

		virtual CombatData* AsCombatData() { return nullptr; }
		virtual std::list<CombatValueEffect*>* GetPortList() { return nullptr; }//Function for run on enemy effects.



		virtual void* GetBehaviourChild(const type_info* a_type) { return nullptr; }

		template<class ReturnType>
		void* GetBehaviourChild() { return GetBehaviourChild(&typeid(ReturnType)); }

		template<class ReturnType>
		ReturnType* TryBehaviour() { auto* behaviour = GetBehaviourChild<ReturnType>(); return reinterpret_cast<ReturnType*>(behaviour); }

		template<class ReturnType>
		ReturnType& GetBehaviour() { return *TryBehaviour<ReturnType>(); }
	};


	
}