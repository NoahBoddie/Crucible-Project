#pragma once

#include "Hooks.h"
#include "Events.h"
#include "Papyrus.h"
#include "Resources.h"
#include "CombatData.h"
#include "StatedObject.h"
#include "TimeManager.h"
#include "ItemStatHandler.h"
#include "ModSettingHandler.h"
#include "ActionStatHandler.h"
#include "GlobalEffectHolder.h"
#include "CollisionIgnoreHandler.h"

#include "RecoveryController.h"

//#include "TestField.h"
#include "ItemStats.h"
#include "SerializableMap.h"
#include "SerializableList.h"
#include "SerializableObject.h"
#include "SerializationTypePlayground.h"
#include "Utility.h"
//#include "debugapi.h"

#include <map>
#include <functional>


//I 100% NEED a main header to store some of the stuff from here.


//animation events to use
//moveStop
//moveStart
//SprintSart, BlockBashSprint

/// <summary>
/// This object is (supposed to be) the object that holds and manages references to data that we control.
/// Things like magic effects or keywords and such are held here for easy reference. 
/// </summary>
//class Crucible::DataManager
//{
//};
//#define old
void InitializeLog_DD(SKSE::PluginInfo* a_info)
{
#ifndef old


	auto path = logger::log_directory();
	if (!path) {
		SKSE::stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= "Crucible.log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%l] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);


#else

#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		//return false;
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::warn);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
	//*/
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

#endif
}


void InitializeLog()
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format("{}.log"sv, Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
	const auto level = spdlog::level::trace;
#else
	const auto level = spdlog::level::info;
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);
}


extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	/*
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		 return false;
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::warn);
#endif

	spdlog::set_default_logger(std::move(log));
	//spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);
	spdlog::set_pattern("[%l] %v"s);
	
	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
	
	//*/

	InitializeLog();
	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;


	//*

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	

	return true;
}


enum class PluginState {kPreload, kLoaded, kSuspended };

PluginState _crucibleState = PluginState::kPreload;

Crucible::SerializableMap<string_hash, Crucible::SerializableList<int32_t>> serialTestMap;

Crucible::TestObject tester(50);

//*/
class Derived :
	public Crucible::SerialComponent<float>,
	public Crucible::SerialComponent<int>,
	public Crucible::SerialComponent<bool>
{
public:
	void print()
	{
		RE::DebugMessageBox(std::format("f:{}, b:{}, i:{}",
			*GetDataPtr<float>(),
			*GetDataPtr<int>(),
			*GetDataPtr<bool>()));
	}
	
	void SerializeData(Crucible::SerialArgument& serializer, bool& success) override
	{ 
		if (serializer.IsSerializing() == true)
		{
			float& derived_float = *GetDataPtr<float>();
			int& derived_int = *GetDataPtr<int>();
			bool& derived_bool = *GetDataPtr<bool>();


			derived_float *= 10;
			derived_int = std::rand();
			derived_bool = !derived_bool;

			RE::DebugMessageBox("Serialization Ready Component");
		}
	}
};
//*/

Derived testDerived;





void MessageReciever(SKSE::MessagingInterface::Message* a_msg)
{
	if (_crucibleState == PluginState::kSuspended)
		return;//Should we have this suspended, no functionality should work, crucible is deactivated.
	else if (_crucibleState == PluginState::kPreload && a_msg->type != SKSE::MessagingInterface::kDataLoaded)
		return;//Want to block anything from SOME HOW activating pre data load.
	

	switch (a_msg->type)
	{
	case SKSE::MessagingInterface::kDataLoaded:
		//proper load query
		{
			auto dataHandler = RE::TESDataHandler::GetSingleton();
			auto cruciblePlugin = dataHandler->LookupLoadedModByName("Crucible.esp");

			if (!cruciblePlugin){
				logger::info("Crucible.esp not found. SKSE Plugin will not load.");
				_crucibleState = PluginState::kSuspended;
			}
			else {
				logger::info("Crucible.esp found, loading mods");
				
				_crucibleState = PluginState::kLoaded;
				Crucible::Resources::Initialize();
				Crucible::ActionStatHandler::Initialize();
				//Crucible::ItemStatHandler::Initialize();
				Crucible::ModSettingHandler::Initialize();
				Crucible::GlobalEffectHolder::Initialize();
				Crucible::CrucibleEvent::Install();
				Crucible::CrucibleHook::Install();
				
				auto papyrus = SKSE::GetPapyrusInterface();
				if (!papyrus || !papyrus->Register(Crucible::Papyrus::Install)) {
					//report error
				}
			}
		
		}

		break;
	case SKSE::MessagingInterface::kPostLoadGame:
	{
		/*break;
		auto playerObject = RE::PlayerCharacter::GetSingleton();
		RE::FormID _id = playerObject->GetFormID();


		Crucible::CombatData* c_data = nullptr;
		if (Crucible::CombatData::HasDataRecord(_id) == false)//Remove it if it exists instead. WE WILL GET a new one.
			c_data = Crucible::CombatData::AddDataRecord(_id);

		Crucible::CombatData::AddDataRecord(_id);
		//*/
	}
		break;
	case SKSE::MessagingInterface::kPreLoadGame:
		Crucible::TimeManager::ClearAll();
		Crucible::CombatData::RemoveDataRecords();
		Crucible::CollisionIgnoreHandler::ClearIgnoreMap();
		break;

	//case SKSE::MessagingInterface::kNewGame:
	//case account for going back to main menu
		
	}

	
	//return;
	
	//auto manager = Crucible::CrucibleEvent::GetSingleton();
	//auto playerObject = RE::PlayerCharacter::GetSingleton();
	//playerObject->AddAnimationGraphEventSink(manager);
}

#include "Timer.h"

void OnTimerFinish(Crucible::Timer* timer_source, float total_time)
{
	RE::DebugMessageBox("FINISH TIME!");
}

Crucible::TimeStamp timeStamp;
Crucible::Timer test_timer = Crucible::Timer::Local();

void SaveCallback(SKSE::SerializationInterface* a_intfc)
{
	if (_crucibleState != PluginState::kLoaded)
		return;
	//return;

	Crucible::SerialArgument argument(a_intfc, Crucible::SerializingState::Serializing);

	//argument.serialInterface = a_intfc;
	//argument.serialState = Crucible::SerializingState::Serializing;

	Crucible::CombatData* c_data = Crucible::CombatData::GetPlayerRecord();
	//if (Crucible::CombatData::HasDataRecord(_id) == false)//Remove it if it exists instead. WE WILL GET a new one.
	//	c_data = Crucible::CombatData::AddDataRecord(_id);


	if (!c_data)
	{
		RE::DebugMessageBox("couldn't get player record");
		return;
	}


	int player_signature = 72;

	argument.Serialize(player_signature);

	if (argument.Serialize(*c_data) == false)
	{
		RE::DebugMessageBox("Serialization unsuccessful");
		return;
	}

	return;

	test_timer.Start(60); 
	timeStamp.Start(true);

	argument.Serialize(tester);

	
	//int f = 4;
	//bool b = true;
	//argument.Serialize(f);
	//argument.Serialize(b);
	//std::list<int> lister;
	//auto test = bit_cast<void*>(&lister);
	//argument.Serialize(lister);

	argument.Serialize(serialTestMap);

	argument.Serialize(testDerived);
	
	argument.Serialize<Crucible::TimeStamp>(timeStamp);
	argument.Serialize(test_timer);

}

void LoadCallback(SKSE::SerializationInterface* a_intfc)
{
	if (_crucibleState != PluginState::kLoaded)
		return;
	
	//return;

	//return;

	//Crucible::SerialArgument argument;

	//argument.serialInterface = a_intfc;
	//argument.serialState = Crucible::SerializingState::Deserializing;

	Crucible::SerialArgument argument(a_intfc, Crucible::SerializingState::Deserializing);


	int signature = 0;

	if (argument.Serialize(signature) == true && signature == 72)
	{
		//Core idea is to use a useless data type as a header object, to signify what I'm actually about to deserialize, that 
		// way, if creation of the object is important, and the serialization was botched, I can just continue on, no harm no fowl.
		// I'd like a better way to make headers than this though.
		auto playerObject = RE::PlayerCharacter::GetSingleton();
		RE::FormID _id = playerObject->GetFormID();

		//Need a proper way to handle this dog, cause if it's false i'm sure it misses a few things.
		Crucible::CombatData* c_data = new Crucible::CombatData(playerObject, false);
		//if (Crucible::CombatData::HasDataRecord(_id) == false)//Remove it if it exists instead. WE WILL GET a new one.
		//	c_data = Crucible::CombatData::AddDataRecord(_id);

		if (argument.Serialize(*c_data) == false)
		{
			RE::DebugMessageBox("Deserialization unsuccessful");
			delete c_data;
			return;
		}


		if (Crucible::CombatData::InsertDataRecord(c_data) == false)
		{
			RE::DebugMessageBox("Insertion unsuccessful");
			delete c_data;
			return;
		}
	}
	else
	{
		RE::DebugMessageBox("Non-valid signature detected.");
	}
	return;

	if (argument.Serialize(tester) == false)
	{
		Crucible::Utility::DebugMessageBox("Deserialize Failure: TestObject", true);
		return;
	}
	
	RE::DebugMessageBox(std::format("{} value", tester._value));
	

	serialTestMap->clear();

	if (argument.Serialize(serialTestMap) == false)
	{
		Crucible::Utility::DebugMessageBox("Deserialize Failure: Map", true);

		return;
	}

	if (argument.Serialize(testDerived) == false)
	{
		Crucible::Utility::DebugMessageBox("Deserialize Failure: Derived", true);
	}
	else
	{
		testDerived.print();
	}
	

	if (argument.Serialize<Crucible::TimeStamp>(timeStamp) == false)
	{
		Crucible::Utility::DebugMessageBox("Deserialize Failure: TimeStamp", true);
		//timeStamp.Start(true);
	}
	else
	{
		RE::DebugMessageBox(std::format("{} time spent", timeStamp.GetTime()));
	}

	//if (1!=1)
	if (argument.Serialize(test_timer) == false)
	{
		Crucible::Utility::DebugMessageBox("Deserialize Failure: Timer", true);
		
	}
	else
	{
		RE::DebugMessageBox(std::format("{} time elapsed", test_timer.GetTimeElapsed()));
	}



	//if (argument.Serialize(test_timer) == false){test_timer.Start(60);}
	



	bool happen = false;

	for (auto& [key, value] : serialTestMap.get())
	{
		happen = true;

		//Forgive me for this doo doo awfulness please lord have mercy
		std::string head;
		

		head = std::format("key: {}, \n", key);

		int i = 1;
		for (auto& iterValue : value.get())
		{
			head.append(std::format("value({}): {}, \n", i, iterValue));
			i++;
		}

		Crucible::Utility::DebugMessageBox(head, true);

	}

	Crucible::Utility::DebugMessageBox("No happen", !happen);
}








extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	auto messaging = SKSE::GetMessagingInterface();
	
	messaging->RegisterListener(MessageReciever);

	//TeSt
	std::srand(std::time(nullptr));

	int random_variable = std::rand();

	auto appendNumbers = [](Crucible::SerializableList<int32_t>& list)
	{
		int size = 5;
		for (int i = 0; i < 5; i++)
		{
			list->push_back(std::rand());
		}
	};


	
	appendNumbers(serialTestMap["_A"_h]);
	appendNumbers(serialTestMap["_B"_h]);
	appendNumbers(serialTestMap["_C"_h]);
	appendNumbers(serialTestMap["_D"_h]);
	appendNumbers(serialTestMap["_E"_h]);
	appendNumbers(serialTestMap["_f"_h]);
	//testEND

	*testDerived.GetDataPtr<float>() = 69.f;
	*testDerived.GetDataPtr<int>() = 420;
	*testDerived.GetDataPtr<bool>() = true;

	test_timer.SetTimerFinishCallback(OnTimerFinish);

	Crucible::MainSerializer::Initialize('CRUC');
	//auto serialization = SKSE::GetSerializationInterface();
	//serialization->SetUniqueID('CRUC');
	//serialization->SetSaveCallback(SaveCallback);
	//serialization->SetLoadCallback(LoadCallback);

	Crucible::ItemStatHandler::Initialize();

	return true;
}
