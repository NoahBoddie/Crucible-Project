#pragma once

namespace Crucible
{

	class CombatData;
	class CombatDataBehaviour;

	class IBehaviour;
	class HeadBehaviour;
	class TailBehaviour;
	inline std::unordered_map<HeadBehaviour*, std::vector<TailBehaviour*>> pendingBehaviourList;
	inline std::uint16_t _pendingCounter = 0;
	inline std::mutex _behaviourLock;// I completely don't believe this mutex is of any use what so ever.
	
	#define automatic_registry_enabled


	//Empty object that serves as the head of the behaviour pack
	class HeadBehaviour
	{
		


	public:

#ifdef automatic_registry_enabled

		HeadBehaviour()
		{
			std::lock_guard<std::mutex> behaviour_guard(_behaviourLock);

			logger::info(">Head Detected:{}\n==========================================="sv, address());

			_pendingCounter++;
			auto& just_initializing = pendingBehaviourList[this];
		}
#endif 
		std::uint64_t address() { return reinterpret_cast<std::uintptr_t>(this); }

		bool IsRelated(TailBehaviour* behaviour);

		void SetOwner(CombatData* new_owner, bool is_building);
	};

	 
	struct TailBehaviour
	{

		std::uint64_t address() { return reinterpret_cast<std::uintptr_t>(this); }


#ifdef automatic_registry_enabled

		TailBehaviour()
		{
			std::lock_guard<std::mutex> behaviour_guard(_behaviourLock);

			logger::info("*BehaviourChecking:{}\n"sv, address());

			for (auto& [key, entry_list] : pendingBehaviourList){
				if (key->IsRelated(this) == true) {
					entry_list.push_back(this);
					logger::info(">Finish \n==========================================="sv);
					return;
				}
			}

			throw nullptr;//I AM THE DEMAND
		}

#endif 

		IBehaviour* GetBehaviour();

		void SetOwner(CombatData* new_owner, bool is_building);
	};

	//An abstract class that CombatDataBehaviour inherents, made for the purpose of automatically registering a behaviour to the CombatData object
	class IBehaviour
	{
	private:
		//The behaviour header, who's constructor places the behaviour in registry without the need to implement a constructor each interation
		// of the class.
		TailBehaviour header;

	protected:
		virtual void SetOwner(CombatData* newOwner, bool isBuilding = true) = 0;

		friend class TailBehaviour;
	};
}
