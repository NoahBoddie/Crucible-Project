#include "IBehaviour.h"
#include "CombatData.h"
#include "CombatDataBehaviour.h"

namespace Crucible
{
	IBehaviour* TailBehaviour::GetBehaviour()
	{

		//static bool once = true;

		//auto offset = offsetof(CombatData, CombatData::header) / 8;

		//CombatData* data_ptr = reinterpret_cast<CombatData*>(this - offset);
		//Utility::DebugMessageBox(std::format("TEST: {}, {} - {} = {}, {}", newOwner == data_ptr, (std::uintptr_t)this, offsetof(CombatData, CombatData::header), (std::uintptr_t)data_ptr, (std::uintptr_t)newOwner), once);

		auto offset = offsetof(IBehaviour, IBehaviour::header);// / 8;
		auto ptr = reinterpret_cast<IBehaviour*>(this - offset);
		//Utility::DebugMessageBox(std::format("TEST: {}, {}, {}",(std::uintptr_t)this, offset, (std::uintptr_t)ptr), once);
		//once = false;

		return ptr;
	}

	void TailBehaviour::SetOwner(CombatData* new_owner, bool is_building)
	{
		IBehaviour* behaviour = GetBehaviour();
		behaviour->SetOwner(new_owner, is_building);
	}



	bool HeadBehaviour::IsRelated(TailBehaviour* behaviour)
	{
		std::uintptr_t head_addr = address();// - sizeof(HeadBehaviour);//Don't really need this last bit.
		std::uintptr_t be_addr = reinterpret_cast<std::uintptr_t>(behaviour);
		std::uintptr_t range = head_addr + sizeof(CombatData);

		logger::info(" -head:{} < behaviour:{} < end:{}"sv, head_addr, be_addr, range);

		return head_addr < be_addr && be_addr < range;
	}

	void HeadBehaviour::SetOwner(CombatData* new_owner, bool is_building)
	{
#ifdef automatic_registry_enabled

		std::lock_guard<std::mutex> behaviour_guard(_behaviourLock);


		{
			auto& behaviour_entries = pendingBehaviourList[this];

			for (auto& behaviour : behaviour_entries) {
				behaviour->SetOwner(new_owner, is_building);
			}
		}

		_pendingCounter--;

		if (!_pendingCounter) {
			//pendingBehaviourList.clear();
			logger::info("Clear");
		}
		pendingBehaviourList.erase(this);
#endif 
	}
}
