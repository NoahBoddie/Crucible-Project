#pragma once
#include "MagicStatController.h"
#include "Resources.h"
#include "CombatData.h"

namespace Crucible
{
	/*
	bool MagicStatController::EvaluateState(RE::ActorMagicCaster* caster, float delta)
	{
		//Feel like this needs to not go through if you're focusing.
		
		uMagicCastState state = caster->GetCastingState();
		
			
		//If we are in a time where we don't care about magicka change, don't force it. if not, force it.
		MagicCasterData* data = GetCreateCasterData(caster);//state == uMagicCastState::kNone ? GetCasterData(caster) : ;
		
		if (!data)
			return false;
			
		//Remove repeat testing for none state, it will only happen once.
		//char test = '1111';

		if (state != uMagicCastState::kNone)
			magicCombo.StopEndComboWindow();

		//This is only for the evaluation of stats and states.
		if ((state == uMagicCastState::kNone && storedEnum != state) || state > storedEnum)
		{
			storedEnum = state;

			switch (state)
			{
			case uMagicCastState::kNone:
				GoToState(MagicState::kNone);
				break;

			case uMagicCastState::kCastStart://Don't really care about this.
			case uMagicCastState::kCharging:
			case uMagicCastState::kCharged:
			case uMagicCastState::kReleased:
				if (Utility::GetMagicSkill(caster->currentSpell) == MagicSkill::kRitual)
					GoToState(MagicState::kRitualCharging);
				else
					GoToState(MagicState::kCharging);

				goto label;


			case uMagicCastState::kConcentrating:
				//figure out piece out ritual, dual, or normal.
				data->castTime = 0;
				GoToState(MagicState::kCasting);
				goto label;

			default:
			label://Do I have to do this here?
				if (caster->currentSpell && data->spellType == ItemType::kNone) {
					data->spellType = Utility::GetItemType(caster->currentSpell);
					data->skillLevel = Utility::GetMagicSkill(caster->currentSpell);
					//magicCombo.StopEndComboWindow();
					//magicCombo.input++;
					//Utility::DebugNotification(std::format("{} start input", magicCombo.input).c_str());

				}
			
			}

			//Utility::DebugMessageBox(std::format("State: {}, PureState: {}, match: {}, castHand: {}", (int)GetCurrentState(), (int)state, state == uMagicCastState::kCharged, (int)caster->GetCastingSource()));
		}


		if (state != data->previousState)
		{
			switch (state)
			{
			case uMagicCastState::kNone:
				if (data->usedFlag == false)
				{
					switch (data->previousState)
					{
					case uMagicCastState::kReleased:
					case uMagicCastState::kConcentrating:
						magicCombo.IncrementCombo(data);
					}
					
					//debug stuff
					if (data->previousState == uMagicCastState::kReleased)
					{
						Utility::DebugMessageBox(std::format("FireCast, Resource: {}, spellType: {}, skillLevel: {}",
							data->resourceStore, (int)data->spellType, (int)data->skillLevel));
					}
					//These 2 aren't needed, they are just tellers
					else if (data->previousState == uMagicCastState::kConcentrating)
					{
						Utility::DebugMessageBox(std::format("ConcCast, Resource: {}, spellType: {}, skillLevel: {}",
							data->resourceStore, (int)data->spellType, (int)data->skillLevel));
					}
					else if (data->resourceStore)
					{
						Utility::DebugMessageBox(std::format("Canceled, Resource: {}, spellType: {}, skillLevel: {}, time in: {}",
							data->resourceStore, (int)data->spellType, (int)data->skillLevel, data->castTime));
					}
				}
				
				data->spellType = ItemType::kNone;
				data->skillLevel = MagicSkill::kZero;
				data->resourceStore = 0;
				data->usedFlag = false;
				data->castTime = 0;
				magicCombo.StartEndComboWindow();//At a later point, have a function update and check around

				break;

			case uMagicCastState::kCharging:
				//This will do something once rituals do something.

			case uMagicCastState::kReleased:
				//This does something once some spells are released, and ritual spells are released.
				//Dodge point calc, right here.
				//Increment combo here, conc might not get here.

				//If released ready this for the next highest


				break;


			case uMagicCastState::kConcentrating:
				//Don't think there's even anything to do here ngl.
				break;
			}
		}

		data->previousState = state;

		switch (state)
		{
		case uMagicCastState::kNone:
		case uMagicCastState::kCastStart:
			return true;

		//case uMagicCastState::kConcentrating:
			//data->deltaTime = delta;
			//magicCombo.IncrementCombo(data);
			//SendCastData(data);//Concentration sends constantly, not on end.
			//break;
		
		//default:// I might do this, if I want to do something like overcharge
		//	data->castTime += delta;
		}
		data->castTime += delta;
		
		return true;//Won't care about this returning (anymore, it's not needed)
	}

	void MagicStatController::Initialize()
	{
		magicCombo.SetOwner(owner);
	}

	void MagicStatController::OnStateBegin()
	{
		//If I could get this to a point where it wouldn't be state none for like a single frame that would be tight.
		
		HandleChange(GetPreviousState(), false);
		HandleChange(GetCurrentState(), true);
	}

	//Currently, this cannot handle the concept of power attacks, or individuals
	// who are currently using completely different weapons.
	void MagicStatController::HandleChange(MagicState _state, bool assign)
	{
		std::int8_t mod = assign ? 1 : -1;

		switch (_state)
		{
		case MagicState::kCasting:
			owner->resist -= 2 * mod;
			break;
		case MagicState::kCharging:
			owner->resist %= -0.20f * mod;
			break;

		case MagicState::kRitualCharging:
			owner->resist += 2 * mod;
			break;
		}
	}
	//*/
}