#include "CombatData.h"


namespace Crucible
{

	
	CombatData* CombatData::AddDataRecord(RE::FormID _id)
	{


		//At a later point, if this space is some how already taken, investigate if the object is relevant and if not
		// delete the entry and proceed.
		if (_id == 0) {// || _characterMap.contains(_id) == true){
			//throw nullptr;
			return nullptr;
		}

		RE::TESForm* loadedForm = RE::TESForm::LookupByID(_id);

		//if (loadedForm->GetFormType() != RE::FormType::ActorCharacter)
		//	return false;

		RE::Actor* actor = skyrim_cast<RE::Actor*>(loadedForm);

		return AddDataRecord(actor);
	}

	CombatData* CombatData::AddDataRecord(RE::Actor* _actor)
	{
		if (!_actor)
			return nullptr;

		CombatData* cData = new CombatData(_actor);


		//*cData = new CombatData(actor);

		//todo: ask if valid, if not delete it and return false
		//CombatData *cData = new CombatData(actor);

		_characterMap[_actor->GetFormID()] = cData;

		//delete cData;

		return cData;
	}



	bool CombatData::RemoveDataRecord(RE::FormID _id)
	{
		if (!_id)// || _characterMap.contains(_id) == false)
			return false;

		std::string display = std::format("[Unregister {} ]", _id);
		//needs an initializer part
		//It will need to search the left hand, the right hand
		RE::DebugNotification(display.c_str());

		CombatData* cData = _characterMap[_id];

		_characterMap.erase(_id);

		//delete *cData;
		delete cData;

		return true;
	}
	


	bool CombatData::HasDataRecord(RE::FormID _id)
	{
		if (!_id)// || _characterMap.contains(_id) == false)
			return false;
		//may check if pointer is valid, but don't think it's necessary right now
		return _characterMap.contains(_id);
	}


	//At a later point, I need a version of this function that will accept references. This version will return the single
	// a singleton CombatValueOwner. 
	
	//Might make operators to shorten this process some.
	CombatData* CombatData::GetDataRecord(RE::Actor* _actor) 
	{
		if (!_actor)
			return nullptr;

		return GetDataRecord(_actor->formID);
	}

	CombatData* CombatData::GetDataRecord(RE::FormID _id)
	{
		if (HasDataRecord(_id) == false)
			return nullptr;
		
		return _characterMap[_id];
	}

	void CombatData::RemoveDataRecords()
	{
		for (auto dataPair : _characterMap)
		{
			delete dataPair.second;
		}

		_characterMap.clear();
	}

}