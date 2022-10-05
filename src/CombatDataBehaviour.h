#pragma once

#include "EventSystem.h"
#include "IBehaviour.h"
namespace Crucible
{
	class ActionPointController;
	class CombatData;
	/*Wanted to mirror base form component, a central
	class BaseCombatDataComponent{
	public:
		virtual CombatData* GetOwner();
		virtual RE::FormID	GetID();
		virtual bool		IsPlayerRef();
		virtual RE::Actor*	GetActorOwner();
	};
	//*/

	//#define IMPLEMENT(class_name) class_name() { int first = 01;/*This is the place we do the THIS thing*/ }

	class CombatDataBehaviour :
		public IBehaviour,
		public EventMember
	{
		//note, source defined template functions are not fucking allowed.

	public:
		void			SetOwner(CombatData* newOwner, bool isBuilding = true) override;
		CombatData*		GetOwner();
		RE::FormID		GetID();
		bool			IsPlayerRef();
		RE::Actor*		GetActorOwner();
		RE::TESForm*	CheckAttackForm(RE::TESForm* form);//Gives parameter if exists and isn't a fist, else returns race.
		RE::TESForm*	GetBlockingForm(bool check = true);
		template<class... Args> requires(sizeof...(Args) != 0)
		void			SendOwnerEvent(std::string eventName, Args&&... args);
		void			SendOwnerEvent(std::string eventName);


		//ActionPointController& temp_GetActionPointController();

		//CombatDataBehaviour* GetSiblingBehaviour(std::string name);
		
		//Isn't it sorta dumb I still have to specify name when doing this?
		//template<std::derived_from<CombatDataBehaviour> DerivedBehaviour>
		//DerivedBehaviour* GetSiblingBehaviour(std::string name);
		//CombatDataBehaviour() = delete;//Used to force deriving classes to manually implement the default constructor.


		void* TrySibling(const type_info* a_type);

		template<class ReturnType>
		ReturnType* TrySibling() { void* sibling = TrySibling(&typeid(ReturnType)); return reinterpret_cast<ReturnType*>(sibling); }

		
		template<class ReturnType>
		ReturnType& GetSibling() { return *TrySibling<ReturnType>(); }


	protected:
		EventHandler* GetHandler() override;


		virtual void	Initialize() {}
		virtual void	Initialize_INTERNAL() { Initialize(); }
		virtual void	Build() {}
		CombatData* owner;
	private:


		bool _init = false;
	};
}