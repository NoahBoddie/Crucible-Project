#pragma once

#include "CombatData.h"
#include "CombatValueOwner.h"

namespace Crucible
{

	//I want to start keeping the records with the map, so I can just clear, since it has it's own data handling in that regard.
	// Though, if I go the other route, I can have this thing clean up after itself.
	CombatData* CombatData::AddDataRecord(RE::FormID _id)
	{
		//At a later point, if this space is some how already taken, investigate if the object is relevant and if not
		// delete the entry and proceed.
		if (_id == 0 || _characterMap->contains(_id) == true){
			//throw nullptr;
			return nullptr;//Should probably give instead?
		}

		RE::Actor* actor = RE::TESForm::LookupByID<RE::Actor>(_id);

		return AddDataRecord_Internal(actor);
	}

	CombatData* CombatData::AddDataRecord(RE::Actor* _actor)
	{
		if (!_actor)
			return nullptr;

		RE::FormID _id = _actor->GetFormID();
		
		//At a later point, if this space is some how already taken, investigate if the object is relevant and if not
		// delete the entry and proceed.
		if (_id == 0 || _characterMap->contains(_id) == true) {
			//throw nullptr;
			return nullptr;//Should probably give instead?
		}

		return AddDataRecord_Internal(_actor);
	}

	CombatData* CombatData::AddDataRecord_Internal(RE::Actor* _actor)
	{
		CombatData* cData = new CombatData(_actor);

		logger::info("new creationed {}/{:08X}", _actor->GetName(), _actor->formID);
		//*cData = new CombatData(actor);

		//todo: ask if valid, if not delete it and return false
		//CombatData *cData = new CombatData(actor);

		_characterMap[_actor->GetFormID()] = cData;

		//delete cData;

		return cData;
	}


	bool CombatData::InsertDataRecord(CombatData* data)
	{
		if (!data || _characterMap->contains(data->_ownerID) == true)
			return false;

		_characterMap[data->_ownerID] = data;
	}


	bool CombatData::RemoveDataRecord(RE::FormID _id)
	{
		if (!_id || _characterMap->contains(_id) == false)
			return false;

		//needs an initializer part
		//It will need to search the left hand, the right hand
		logger::info("[Unregister {} ]", _id);


		CombatData* cData = _characterMap[_id];

		_characterMap->erase(_id);

		//delete *cData;
		delete cData;

		return true;
	}
	


	bool CombatData::HasDataRecord(RE::FormID _id)
	{
		if (!_id)// || _characterMap->contains(_id) == false)
			return false;
		//may check if pointer is valid, but don't think it's necessary right now
		return _characterMap->contains(_id);
	}


	//At a later point, I need a version of this function that will accept references. This version will return the single
	// a singleton CombatValueOwner. 
	
	//Might make operators to shorten this process some.
	CombatData* CombatData::GetDataRecord(RE::Actor* _actor, bool force)
	{
		if (!_actor)
			return nullptr;

		if (HasDataRecord(_actor->formID) == false) {
			if (!force) {
				return nullptr;
			}
			else {
				logger::info("used on Actor {}/{:08X}, forcing records", _actor->GetName(), _actor->formID);
				return AddDataRecord(_actor);
			}
		}

		return _characterMap[_actor->formID];
	}

	CombatData* CombatData::GetDataRecord(RE::FormID _id, bool force)
	{
		if (HasDataRecord(_id) == false) {
			if (!force) {
				return nullptr;
			}
			else{
				logger::info("used on FormID {:08X}", _id);
				return AddDataRecord(_id);
			}
		}

		return _characterMap[_id];
	}
	

	CombatValueOwner* CombatData::GetValueOwner(RE::TESObjectREFR* refr)
	{
		if (!refr || refr->GetFormType() != RE::FormType::ActorCharacter)
			return CombatValueOwner::GetObjectValueOwner();

		return GetDataRecord(refr->formID);
	}

	CombatValueOwner* CombatData::GetValueOwner(RE::FormID _id)
	{
		//Technically could just look up and see if returns null.
		// I have nothing else to say.
		RE::TESObjectREFR* refr = RE::TESForm::LookupByID<RE::TESObjectREFR>(_id);

		return GetValueOwner(refr);
	}


	void CombatData::RemoveDataRecords()
	{
		for (auto dataPair : *_characterMap)
		{
			delete dataPair.second;
		}

		_characterMap->clear();
	}

}