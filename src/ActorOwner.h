#pragma once
namespace Crucible
{
	struct QueryContext;

	//As it says, it belongs to objects that own actor, recording their form in particular.
	class ActorOwner
	{
	public:
		RE::Actor* GetOwner();

		RE::TESForm* CheckAttackForm(RE::TESForm* form);

		RE::TESForm* CheckSource(QueryContext& context);

		bool IsPlayerRef() { return _ownerID == 0x00000014; }

	protected:
		RE::FormID _ownerID{ 0 };

		RE::TESRace* ownerRace;

	};

}