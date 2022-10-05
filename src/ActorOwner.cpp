#pragma once
#include "ActorOwner.h"
#include "QueryContext.h"

namespace Crucible
{
	RE::Actor* ActorOwner::GetOwner()
	{
		RE::Actor* actor = RE::TESForm::LookupByID<RE::Actor>(_ownerID);

		return actor;
	}

	RE::TESForm* ActorOwner::CheckAttackForm(RE::TESForm* form)
	{
		//auto weapTest = form ? form->As<RE::TESObjectWEAP>() : nullptr;

		if (form)// && (!weapTest || weapTest->GetWeaponType() != RE::WEAPON_TYPE::kHandToHandMelee) == true)
			return form;
		else {
			RE::Actor* actor = GetOwner();
			return actor ? actor->race : nullptr;
		}
	}

	RE::TESForm* ActorOwner::CheckSource(QueryContext& context)
	{
		auto source = context.GetForm(querySource);

		if (!source)
		{
			//Utility::DebugMessageBox("error2");
			source = CheckAttackForm(source);
			context.SetForm(querySource, source);
		}

		return source;
	}

}