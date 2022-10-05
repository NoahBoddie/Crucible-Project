#pragma once

#include "ISerializer.h"

namespace Crucible
{
#define dddd
#ifdef dddd
	class ISerializable;
	class SerialArgument;
	class ISerialClassTrival;

	//A helper class to serializable object to remove the attachment to templates, to better suit different types
	// of inheritence. Additionally, this will stop caring about the typeid, and instead will only care about the size.
	// If this works out, I can remove the template stuff.
	class SerializableBytes
	{
#define Constraints (std::derived_from<SerialObject, ISerializable>)// && !std::is_same<SerialObject, ISerializable>::value)
#define Set_Bytes	bool class_relavent = !std::derived_from<SerialObject, ISerialClassTrival>;													\
					if (!class_relavent || typeid(ISerializable).hash_code() != typeid(SerialObject).hash_code())								\
					SetBytes(&a_rhs, sizeof(a_rhs), const_cast<std::type_info*>(class_relavent ? &typeid(a_rhs) : &typeid(ISerialClassTrival)))	\
		
	private:
		std::unique_ptr<std::byte[]> _bytes = nullptr;
		std::type_info* _byteSignature = nullptr;
		std::size_t _size = 1;

		void SetBytes(void* source, std::size_t size, std::type_info* info)
		{
			_size = size;
			_byteSignature = info;
			_bytes.reset(new std::byte[size]());
			std::memcpy(_bytes.get(), source, size);
		}

		
	public:

		//You may be wondering, Noah, why do you have the object have a return type, a reference AND a success reference?
		// I don't want to have to make a new object to use this function, mainly because I don't know what might happen in
		// deconstruction in the future. Similarly, if I have to return a serializable type, I want to return the original,
		// a properly working function version of the object. Lastly, if I'm gonna return the object outright, I'd like
		// to have some 3rd thing to tell success, because a completely 0 value could still be valid.
		template<class SerialObject>requires Constraints
		SerialObject GetSerializableObject(SerialObject& serial_object, bool& success)
		{
			//*
			if (!std::derived_from<SerialObject, ISerialClassTrival> && &typeid(SerialObject) != _byteSignature) {
				RE::DebugMessageBox(std::format("place is wrong, {} v {}", (int64_t)&typeid(SerialObject), (int64_t)_byteSignature));
				success = false;
				return serial_object;

			}
			else//*/
			if(sizeof(SerialObject) != _size) {
				RE::DebugMessageBox(std::format("size is wrong, {}", _size));
				success = false;
				return serial_object;

			}
			SerialObject return_object = serial_object;
			std::memcpy(&return_object, _bytes.get(), _size);
			return return_object;
			//return std::bit_cast<SerialObject>(*_bytes.get());
		}


		SerializableBytes() = default;

		//Copy Constructor
		template<class SerialObject>requires Constraints
		SerializableBytes(SerialObject& a_rhs) { Set_Bytes; }


		//Copy Assignment
		template<class SerialObject>requires Constraints
		SerializableBytes& operator=(SerialObject& a_rhs) { Set_Bytes; }

		//Copy Assignment with loose
		template<class SerialObject>requires Constraints
		SerializableBytes& operator=(SerialObject a_rhs) { Set_Bytes; }

		//Explicit move assignment
		template<class SerialObject>requires Constraints
		SerializableBytes& operator=(SerialObject&& a_rhs) { Set_Bytes; }


		// implicit conversion
		//template<class SerialObject>requires(std::derived_from<SerialObject, ISerializable>);
		//operator SerialObject() const { return _formID; }



		// explicit conversion
		//template<class SerialObject>requires(std::derived_from<SerialObject, ISerializable>);
		//explicit operator RE::FormID* () const { return (&_formID); }
	};

	
	//Make this a template type that takes a bool, merging it with Class Irrelevant. Better solution would be
	// making a binary enum to make it a bit more readable.
	class ISerializable
	{
	public:
		//The type is solely to resolve ambiguity, so it happens simply.
		template<std::derived_from<ISerializable> SerializableObject>
		inline SerializableObject* As(SerializableObject* type) { return static_cast<SerializableObject*>(this); }
		


		void HandleSerialize(SerialArgument& serializer, bool& success, ISerializable* pre_process) {}

		//I'm thinking of making this a single function with OnDeserialize
		void OnSerialize(SerialArgument& serializer, bool& success, ISerializable* pre_process) {}


		void OnDeserialize(SerialArgument& serializer, bool& success, ISerializable* pre_process) {}
	};
	
	//Marker class to say the class type data shouldn't matter, as long as it remains the same size. Used for stuff like relink
	// pointer where while it's giving the relink pointer object, it can and does get derived, but the data being the same size
	// shouldn't cause undefined behaviour
	class ISerialClassTrival : public ISerializable {};


	
#endif
	/*
	//This object is losing its identity atm
	//Deprecated, replaced by serializingWrapper and SerializingHandler
	template <class Type>
	class SerializableObject
	{
	public:
		virtual Type GetSerialized(bool& success) = 0;//Questioning whether this should be a template or not.
		//{
			//This isn't really how this goes, but basically it's the gist of what would happen when you use
			// get serialized on something that doesn't need it.
			//return *this;
		//}

		//An optional deserialize function, needs the interface, mainly exists to resolve 
		// form ids.
		virtual void OnDeserialize(SerialArgument& serializer) { }
	};
	//*/
}