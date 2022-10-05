#pragma once

#include "SerialIterator.h"
#include "SerializingWrapper.h"

namespace Crucible
{
#define radical_map
#ifdef radical_map
	
	template<class WrapType>
	using SideHandleFunction = void(*)(WrapType&, SerialArgument&, bool&);
	//REPLACE_ENTRY_HANDLE
	template<class A, class B, SideHandleFunction<A> HandleA, SideHandleFunction<A> HandleB>
	//requires (std::is_same<LeftHandleFunction<A>, HandleA>::value && std::is_same<RightHandleFunction<B>, HandleB>::value)
	using EntryHandleFunction = void(*)(std::pair<const SerializingWrapper<A, HandleA>, SerializingWrapper<B, HandleB>>&, SerialArgument&, bool&);
	
	

	//old
	//void(*REPLACE_PAIR_B_HANDLE)(SerializingWrapper<PairB, REPLACE_PAIR_B_HANDLE>&, SerialArgument&) = nullptr,


	
	template<class PairA, class PairB,//Use a macro that takes types to make this shit.
		SideHandleFunction<PairA> REPLACE_PAIR_A_HANDLE = nullptr,
		SideHandleFunction<PairB> REPLACE_PAIR_B_HANDLE = nullptr,
		//EntryHandleFunction<PairA, PairB, REPLACE_PAIR_A_HANDLE, REPLACE_PAIR_B_HANDLE> REPLACE_ENTRY_HANDLE = nullptr
		void(*REPLACE_ENTRY_HANDLE)(std::pair<const SerializingWrapper<PairA, REPLACE_PAIR_A_HANDLE>, SerializingWrapper<PairB, REPLACE_PAIR_B_HANDLE>>&, SerialArgument&, bool&) = nullptr
	>
		//class SerializableMap : public SerialIterator<std::map<PairA, PairB>, std::pair<const PairA, PairB>, REPLACE_ENTRY_HANDLE>
	class SerializableMap :
		public SerialIterator<
		std::map<SerializingWrapper<PairA, REPLACE_PAIR_A_HANDLE>, SerializingWrapper<PairB, REPLACE_PAIR_B_HANDLE>>,
		std::pair<const SerializingWrapper<PairA, REPLACE_PAIR_A_HANDLE>, SerializingWrapper<PairB, REPLACE_PAIR_B_HANDLE>>,
		//std::pair<const SerializingWrapper<PairA, REPLACE_PAIR_A_HANDLE>, SerializingWrapper<PairB, REPLACE_PAIR_B_HANDLE>>,
		REPLACE_ENTRY_HANDLE, 
		std::map<PairA, PairB>
	>
	{
	protected:
		//using Pair = std::pair<const PairA, PairB>;
		using Pair = std::pair<const SerializingWrapper<PairA, REPLACE_PAIR_A_HANDLE>, SerializingWrapper<PairB, REPLACE_PAIR_B_HANDLE>>;


	public:

		void EmplaceEntry(Pair& entry) override
		{
			this->_iteratable.emplace(entry);
		}
		
		//*
		void HandleEntry(Pair& entry, SerialArgument& serializer, bool& success) override
		{
			//serializer.Serialize(entry);
			serializer.Serialize(entry.first);
			serializer.Serialize(entry.second);
			

			//The first entry may be an issue, since it's const. Messes with serializer.
			//serializer.Serialize(entry.first);//This much should be all that's necessary.
			//serializer.Serialize(entry.second);

			//this->HandleLeft(entry.first, serializer);
			//this->HandleRight(entry.second, serializer);

		}
		//*/

		PairB& operator[](const PairA& key)
		{
			auto& _map = this->get();
			return _map[key];
		}
//#ifdef ddddddddddddddddd
		template<class LikeA>
		PairB& operator[](const LikeA& key)
		{
			//Check for invalid conversions.
			auto& _map = this->get();

			//static_assert(std::is_same_v<LikeA, SerialFormID>, "llll");
			//return _map[static_cast<const PairA&>(*&key)];
			//Try this if you can convert this.
			//PairA pair_test = key;
			PairA pair_test(key);
			//Desperate.
			return _map[pair_test];
		}
//#endif
	};


	//Removes the need for serialFormID (it's hard to use in maps).
	//template<DerivedSerialWrapper<RE::FormID> WrappedFormID>
	inline void SerializeFormHandle(RE::FormID& form_id, SerialArgument& argument, bool& success)
	{
		success = argument.Serialize(form_id);

		if (success && argument.IsDeserializing() == true) {
			success = argument.ResolveFormID(form_id);
		}
	}

	//template<class ValueType, SideHandleFunction<ValueType> RightHandle = nullptr,
	//	void(*EntryHandle)(std::pair<const SerializingWrapper<RE::FormID, SerializeFormHandle>, SerializingWrapper<ValueType, SideHandleFunction<ValueType>>>&, SerialArgument&, bool&) = nullptr>
	//	using SerialFormIDMap = SerializableMap<RE::FormID, ValueType, SerializeFormHandle, RightHandle, EntryHandle>;
	//inline SerialFormIDMap<float> test_tttt;



	template<class PairA, class PairB,//Use a macro that takes types to make this shit.
		SideHandleFunction<PairA> REPLACE_PAIR_A_HANDLE = nullptr,
		SideHandleFunction<PairB> REPLACE_PAIR_B_HANDLE = nullptr,
		//EntryHandleFunction<PairA, PairB, REPLACE_PAIR_A_HANDLE, REPLACE_PAIR_B_HANDLE> REPLACE_ENTRY_HANDLE = nullptr
		void(*REPLACE_ENTRY_HANDLE)(std::pair<const SerializingWrapper<PairA, REPLACE_PAIR_A_HANDLE>, SerializingWrapper<PairB, REPLACE_PAIR_B_HANDLE>>&, SerialArgument&, bool&) = nullptr
	>
		//class SerializableMap : public SerialIterator<std::map<PairA, PairB>, std::pair<const PairA, PairB>, REPLACE_ENTRY_HANDLE>
		class SerialUnorderedMap :
		public SerialIterator<
		std::unordered_map<SerializingWrapper<PairA, REPLACE_PAIR_A_HANDLE>, SerializingWrapper<PairB, REPLACE_PAIR_B_HANDLE>>,
		std::pair<const SerializingWrapper<PairA, REPLACE_PAIR_A_HANDLE>, SerializingWrapper<PairB, REPLACE_PAIR_B_HANDLE>>,
		//std::pair<const SerializingWrapper<PairA, REPLACE_PAIR_A_HANDLE>, SerializingWrapper<PairB, REPLACE_PAIR_B_HANDLE>>,
		REPLACE_ENTRY_HANDLE,
		std::unordered_map<PairA, PairB>
		>
	{
	protected:
		//using Pair = std::pair<const PairA, PairB>;
		using Pair = std::pair<const SerializingWrapper<PairA, REPLACE_PAIR_A_HANDLE>, SerializingWrapper<PairB, REPLACE_PAIR_B_HANDLE>>;


	public:

		void EmplaceEntry(Pair& entry) override
		{
			this->_iteratable.emplace(entry);
		}

		//*
		void HandleEntry(Pair& entry, SerialArgument& serializer, bool& success) override
		{
			//serializer.Serialize(entry);
			serializer.Serialize(entry.first);
			serializer.Serialize(entry.second);


			//The first entry may be an issue, since it's const. Messes with serializer.
			//serializer.Serialize(entry.first);//This much should be all that's necessary.
			//serializer.Serialize(entry.second);

			//this->HandleLeft(entry.first, serializer);
			//this->HandleRight(entry.second, serializer);

		}
		//*/

		PairB& operator[](const PairA& key)
		{
			auto& _map = this->get();

			return _map[key];
		}
};



#else
	//Serializable map.
	template<class PairA, class PairB>
	class SerializableMap : public SerialIterator<std::map<PairA, PairB>, std::pair<const PairA, PairB>>
	{
	protected:
		using Pair = std::pair<const PairA, PairB>;

	public:

		void EmplaceEntry(Pair& entry) override
		{
			this->_iteratable.emplace(entry);
		}

		void HandleEntry(Pair& entry, SerialArgument& serializer) override
		{
			//The first entry may be an issue, since it's const. Messes with serializer.
			serializer.Serialize(entry.first);//This much should be all that's necessary.
			serializer.Serialize(entry.second);
		}

		PairB& operator[](const PairA& key)
		{
			return this->_iteratable[key];
		}
	};
#endif
}