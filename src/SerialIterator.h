#pragma once

#include "SerialConstructor.h"
#include "SerialArgument.h"

namespace Crucible
{
	//We need to be using this, so I can tell the iterator to through out the entry.
	template <class Entry>
	using EntryHandle = void(*)(Entry&, SerialArgument&, bool&);

	//Template class for serializing iterators like maps, lists, and vectors in a method acceptable
	// for serialization interface
	template<class Iterable, class Entry, void(*REPLACE_ENTRY_HANDLE)(Entry&, SerialArgument&, bool&) = nullptr, class CastIteratable = Iterable>
	class SerialIterator : public SerializationHandler, public OpenSerializer
	{
	protected:
		Iterable _iteratable;
	

	private:
		bool HandleEntry_INTERNAL(Entry& entry, SerialArgument& serializer)
		{
			bool success = true;

			if constexpr (REPLACE_ENTRY_HANDLE)
				REPLACE_ENTRY_HANDLE(entry, serializer, success);
			else
				HandleEntry(entry, serializer, success);

			return success;
		}


	public:

		virtual void HandleEntry(Entry& entry, SerialArgument& serializer, bool& success) = 0;

		//When deserializes function is used to add the new object at the back of the entry.
		virtual void EmplaceEntry(Entry& entry) = 0;//Doesn't really need a serializer, but keeping to remind that had it once.

		//This is mainly just incase there's some type out there I want to use that doesn't use size.
		virtual size_t GetSize() { return _iteratable.size(); }

		//Handles iteration. Simple as can be.
		void HandleIterator(SerialArgument& serializer)
		{
			size_t size{};

			if (serializer.IsSerializing() == true)
			{
				//Write the data,
				// iterate through each entry and handle the serialization
				size = GetSize();
				logger::info("size {}", size);
				serializer->WriteRecordData(size);

				for (Entry& entry : _iteratable) {
					this->HandleEntry_INTERNAL(entry, serializer);
				}
			}
			else if (serializer.IsDeserializing() == true)
			{
				//Get number of loops, make entry, let handle entry pull the records and create them and plug them, then emplace
				// for maps, the handle should point toward the left and right spots on the pair, making the created spot return to itself.

				//In loaded this is handled much differently.
				// Increase size to desired amount.
				// Then, instead of iterating on pairs we iterate on values, depending where each thing would go.
				//Maps are still weird, you would need to create then place as a key, then use it to get the reference.
				// The question is how do I set such a thing up without messing with handle iterator much.

				if (serializer->ReadRecordData(size) == false) {
					//_ERROR("Failed to load size!");
					return;
				}
				logger::info("size {}", size);
				for (int i = 0; i < size; ++i) {//needs checks
					Entry entry;
					
					if (this->HandleEntry_INTERNAL(entry, serializer) == true)
						EmplaceEntry(entry);
				}
			}
		}


		void HandleSerialize(SerialArgument& serializer, bool& success) override { HandleIterator(serializer); }


		constexpr CastIteratable& get() noexcept { return reinterpret_cast<CastIteratable&>(_iteratable); }

		constexpr CastIteratable* operator->() noexcept { return reinterpret_cast<CastIteratable*>(&_iteratable); }
		
		constexpr CastIteratable& operator*() noexcept { return reinterpret_cast<CastIteratable&>(_iteratable); }

		// implicit conversion
		//operator CastIteratable() const { return _valueSetting.data.f; }
		
		//Allow a beginning and end type of deal so I can just iterate through this.

	};

	/*

private:
	void HandleEntry_INTERNAL(Entry& entry, SerialArgument& serializer)
	{
		if (REPLACE_ENTRY_HANDLE) {
			REPLACE_ENTRY_HANDLE(entry, serializer);
		}
		else {
			HandleEntry(entry, serializer);
		}
	}

public:

	virtual void HandleEntry(Entry& entry, SerialArgument& serializer, bool& success) = 0;

	//When deserializes function is used to add the new object at the back of the entry.
	virtual void EmplaceEntry(Entry& entry) = 0;//Doesn't really need a serializer, but keeping to remind that had it once.

	//This is mainly just incase there's some type out there I want to use that doesn't use size.
	virtual size_t GetSize() { return _iteratable.size(); }

	//Handles iteration. Simple as can be.
	void HandleIterator(SerialArgument& serializer)
	{
		if (serializer.IsSerializing() == true)
		{
			//Write the data,
			// iterate through each entry and handle the serialization
			serializer->WriteRecordData(GetSize());

			for (Entry& entry : _iteratable) {
				this->HandleEntry_INTERNAL(entry, serializer);
			}
		}
		else if (serializer.IsDeserializing() == true)
		{
			//Get number of loops, make entry, let handle entry pull the records and create them and plug them, then emplace
			// for maps, the handle should point toward the left and right spots on the pair, making the created spot return to itself.

			//In loaded this is handled much differently.
			// Increase size to desired amount.
			// Then, instead of iterating on pairs we iterate on values, depending where each thing would go.
			//Maps are still weird, you would need to create then place as a key, then use it to get the reference.
			// The question is how do I set such a thing up without messing with handle iterator much.

			size_t size;
			if (serializer->ReadRecordData(size) == false) {
				//_ERROR("Failed to load size!");
				return;
			}

			for (int i = 0; i < size; ++i) {//needs checks
				Entry entry;
				this->HandleEntry(entry, serializer);
				EmplaceEntry(entry);
			}
		}
	}

	//*/
}