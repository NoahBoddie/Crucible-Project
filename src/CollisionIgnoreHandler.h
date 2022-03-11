#pragma once
namespace Crucible
{
	class CollisionIgnoreHandler
	{
		static inline std::map<RE::FormID, std::list<RE::FormID>> ignoreMap;
	public:

		//may need to include object refs for melee too
		static bool IsIgnoring(RE::TESForm* collider, RE::TESObjectREFR* refr)
		{
			if (!collider || !refr)
				return false;


			auto id = collider->GetFormID();

			auto pair = ignoreMap.find(id);

			if (ignoreMap.end() != pair)
			{
				auto& ignoreList = pair->second;

				auto success = std::find(ignoreList.begin(), ignoreList.end(), refr->GetFormID());

				if (success != ignoreList.end())
				{
					return true;
				}
			}

			return false;
		}

		static void IgnoreReference(RE::TESForm* collider, RE::TESObjectREFR* refr)
		{
			if (!collider || !refr)
				return;


			auto id = collider->GetFormID();

			auto pair = ignoreMap.find(id);
			///ignoreMap.end() != pair ? 


			if (ignoreMap.end() != pair)
			{
				std::list<RE::FormID>& ignoreList = pair->second;
				ignoreList.push_front(refr->GetFormID());
			}
			else
			{
				ignoreMap[id] = { refr->GetFormID() };
			}

		}

		static void ClearIgnoreList(RE::TESForm* collider)
		{
			if (!collider)
				return;

			auto id = collider->GetFormID();

			ignoreMap.erase(id);
		}

		static void ClearIgnoreMap()
		{
			ignoreMap.clear();
			//This will need to happen at some point.
		}

	};

}