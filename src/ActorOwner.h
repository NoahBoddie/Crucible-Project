#pragma once
namespace Crucible
{
	struct QueryContext;

	//As it says, it belongs to objects that own actor, recording their form in particular.
	class ActorOwner
	{
	public:
		RE::Actor* GetOwner();

		RE::FormID GetOwnerID() { return _ownerID; }

		RE::TESForm* CheckAttackForm(RE::TESForm* form);

		RE::TESForm* CheckSource(QueryContext& context);

		bool IsPlayerRef() { return _ownerID == 0x00000014; }

	protected:
		RE::FormID _ownerID{ 0 };//While I do want to keep this, I want to use nipointers at a later point.

		RE::TESRace* ownerRace;

	};

}