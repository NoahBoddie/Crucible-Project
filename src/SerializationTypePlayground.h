#pragma once

#include "SerialArgument.h"
#include "SerializingWrapper.h"
#include "SerializableObject.h"
#include "SerializableMap.h"
#include "SerialComponent.h"



namespace Crucible
{
	//These belong in test field.
	//void TestInput(float a, int b, bool c){}

	//template <class... Args>
	//void TestFunc(void(*TestFunction)(Args...)){}

	//TestFunc(TestInput);

//#include "EventSystem.h"

//#define def_name1(sep, p1) \
    inline constexpr char const p1##_name[]{#p1};

#define FE_INDEX_INDEX_0(WHAT)
#define FE_INDEX_INDEX_1(WHAT, X) WHAT(X) 
#define FE_INDEX_INDEX_2(WHAT, X, ...) WHAT(X)FE_INDEX_1(WHAT, __VA_ARGS__)
#define FE_INDEX_INDEX_3(WHAT, X, ...) WHAT(X)FE_INDEX_2(WHAT, __VA_ARGS__)
#define FE_INDEX_INDEX_4(WHAT, X, ...) WHAT(X)FE_INDEX_3(WHAT, __VA_ARGS__)
#define FE_INDEX_INDEX_5(WHAT, X, ...) WHAT(X)FE_INDEX_4(WHAT, __VA_ARGS__)
//... repeat as needed

#define GET_MACRO(_0,_1,_2,_3,_4,_5,NAME,...) NAME 
#define FOR_EACH(action,...) \
  GET_MACRO(_0,__VA_ARGS__,FE_INDEX_5,FE_INDEX_4,FE_INDEX_3,FE_INDEX_2,FE_INDEX_1,FE_INDEX_0)(action,__VA_ARGS__)



	template <class A, class B>
	using CompressFunction = B(*)(A*);

	template <class A, class B>
	using RelinkerFunction = A * (*)(B, SerialArgument&);

	template <class PointerType, class RelinkType,
		CompressFunction<PointerType, RelinkType> COMPRESS,
		RelinkerFunction<PointerType, RelinkType> RELINK
	>requires (std::is_enum<RelinkType>::value || std::is_integral<RelinkType>::value)//Doesn't actually need to be integral or enum
		class RelinkPointer : public ISerialClassTrival
	{
		static_assert(COMPRESS, "Compression function is not allowed to be null.");
		static_assert(RELINK, "Relinking function is not allowed to be null.");
		//static_assert(RELINK, "Relinking function is not allowed to be null.");
		using Pointer = PointerType*;
		using Relink = RelinkType;

	protected:
		union//Use the struct pointer for this, so I can prevent acccessing an invalid pointer.
		{
			Pointer _ptr;
			Relink _linker;
		};

	public:

		SerializableBytes GetSerialized(bool& success)
		{
			//OnAction_INTERNAL(true);

			RelinkPointer new_relink_ptr(*this);

			new_relink_ptr._linker = COMPRESS(_ptr);

			return new_relink_ptr;
		}//Must be overloaded, but not overriden.

		void HandleSerialize(SerialArgument& serializer, bool& success, ISerializable* pre_process)
		{
			if (serializer.IsSerializing() == true)
				_linker = COMPRESS(_ptr);
			else
				_ptr = RELINK(_linker, serializer);
		}

		//void OnSerialize(SerialArgument& serializer, bool& success, ISerializable* pre_process) { _linker = COMPRESS(_ptr); }
		//void OnDeserialize(SerialArgument& serializer, bool& success, ISerializable* pre_serialized) { _ptr = RELINK(_linker, serializer); }


#pragma region Constructors_And_Assignment
		RelinkPointer() = default;

		//Copy Constructor
		RelinkPointer(Pointer& other) { _ptr = other; }


		//Copy Assignment
		RelinkPointer& operator=(Pointer& a_rhs) { _ptr = a_rhs; return *this; }

		//Copy Assignment with loose
		//RelinkPointer& operator=(Pointer a_rhs) { _ptr = a_rhs; }

		//Explicit move assignment
		RelinkPointer& operator=(Pointer&& a_rhs) { _ptr = a_rhs; return *this; }

		// implicit conversion
		operator Pointer() const { return _ptr; }

		constexpr Pointer operator->() { return _ptr; }
#pragma endregion
	};



	inline const SerializingWrapper<float> test_wrap(50);
	static_assert(sizeof(test_wrap) == sizeof(float));

	inline void foo(SerialArgument& argument)
	{
		argument.Serialize(test_wrap);
	}



	class TestObject : public ISerializable
	{
	public:
		float _value;

		TestObject() = default;

		TestObject(float set_value)
		{
			_value = set_value;
		}

		SerializableBytes GetSerialized(bool& success)
		{
			TestObject test = TestObject(*this);

			test._value *= (float)std::rand();
			RE::DebugMessageBox(std::format("{} test set.", test._value));
			return test;
		}

		void OnDeserialize(SerialArgument& serializer, bool& success, ISerializable* pre_serialized) 
		{ 
			auto* test_obj = pre_serialized->As(this);

			RE::DebugMessageBox(std::format("deserial {}", test_obj->_value));
		}

	};



	class TestMap : public SerializableMap<float, float*>
	{
	};



	class FakeCombatData : public SerializationHandler
	{
	public:
		//Test... args;

		//void Init_Friend() override {}


		bool attackControl;
		bool magicControl;
		bool guardControl;
		bool motionControl;
		bool poiseControl;
		bool actionPointControl;
		bool meleeCombo;
		bool recoverControl;
		bool strikeControl;
		bool* tester;
		void HandleSerialize(SerialArgument& serializer, bool& success) override
		{
			serializer.Serialize(attackControl);
			serializer.Serialize(magicControl);
			serializer.Serialize(guardControl);
			serializer.Serialize(motionControl);
			serializer.Serialize(poiseControl);
			serializer.Serialize(actionPointControl);
			serializer.Serialize(meleeCombo);
			serializer.Serialize(recoverControl);
			serializer.Serialize(strikeControl);


		}

		FakeCombatData(RE::Actor* a_this)
		{

		}
	};


	/*
	template<class A, class B>
	using Pair = std::pair<const A, B>;

	template<derived_from<SerialWrapperBase<RE::FormID>> A, derived_from<SerialWrapperBase<FakeCombatData*>> B>
	void HandleEntry(Pair<A, B>& entry, SerialArgument& serializer, bool& success) override
	{


		serializer.Serialize(entry.first);//Needs to be a particular type of object, serializable formID


		if (serializer.IsDeserializing() == true) {
			auto* actor = RE::TESForm::LookupByID<RE::Actor>(entry.first);

			entry.second = new FakeCombatData(actor);//, serializer);
		}

		//If this bit were to go awry, we would just loop until we found a signature that was a special disignated class.
		serializer.Serialize(*entry.second);
	}
	//*/

	/*
	class CombatDataMap : public SerializableMap<RE::FormID, FakeCombatData*>
	{
		//private:
		//using Pair = std::pair<const RE::FormID, FakeCombatData*>;
		using Pair = std::pair<const SerializingWrapper<RE::FormID>, SerializingWrapper<FakeCombatData*>>;

	public:

		void HandleEntry(Pair& entry, SerialArgument& serializer, bool& success) override
		{


			serializer.Serialize(entry.first);//Needs to be a particular type of object, serializable formID


			if (serializer.IsDeserializing() == true) {
				auto* actor = RE::TESForm::LookupByID<RE::Actor>(entry.first);

				entry.second = new FakeCombatData(actor);//, serializer);
			}

			//If this bit were to go awry, we would just loop until we found a signature that was a special disignated class.
			serializer.Serialize(*entry.second);
		}
	};
	//*/


	template<class A, class B>
	using Pair = std::pair<const A, B>;

	template<DerivedSerialWrapper<RE::FormID> A, DerivedSerialWrapper<FakeCombatData*> B>
	inline void HandleCombatMapEntry(Pair<A, B>& entry, SerialArgument& serializer, bool& success) 
	{
		serializer.Serialize(entry.first);//Needs to be a particular type of object, serializable formID


		if (success && serializer.IsDeserializing() == true) {
			auto* actor = RE::TESForm::LookupByID<RE::Actor>(entry.first);

			entry.second = new FakeCombatData(actor);//, serializer);
		}

		//If the pointer is null or the success is false, it will dump the data, and return unsuccessful.
		success = serializer.DumpIfFailure(entry.second, success);
	}

	using FakeCombatDataMap = SerializableMap<RE::FormID, FakeCombatData*, nullptr, nullptr, HandleCombatMapEntry>;


	/*
	class Derived :
		public SerialComponent<float>,
		public SerialComponent<int>,
		public SerialComponent<bool>
	{
		//Test... args;

		//void Init_Friend() override {}
	};
	//*/


	//This will not always need to be used. It will actually probably never be used. 
	// The best place for it to be used is actually in lists and maps where serializing specific elements will be a pain. 
	//SerializableForm however, is
	// something that will be far more useful.
	// WHEN that gets created, it will need to be an event member, and when created it will look for FF forms,
	// upon detecting it should set its event to a global event for form deletion. Of course, such a thing can only exist when
	// we are allowed to have accountability for what events go where, because since events will no longer be local, they need
	// to be removed.

	//inline void handle_function(SerialArgument& serializer) {}

	//Note, this object would be FAR better suited as a serial wrapper that comes with an aliased type. No need for this
	// or a serializaiton handler.
	//template<void(*function)(SerialArgument& serializer) = nullptr>
	struct SerializableFormID : public ISerializable
	{
	private:
		RE::FormID _formID;

	public:

		constexpr RE::FormID GetID() { return _formID; }


		//Not needed
		//void OnSerialize(SerialArgument& serializer, bool& success, ISerializable* pre_serialized) { }


		void OnDeserialize(SerialArgument& serializer, bool& success, ISerializable* pre_serialized) 
		{
			success = serializer.ResolveFormID(_formID);
		}

		SerializableFormID() = default;

		//Copy Constructor
		SerializableFormID(RE::FormID& other) { _formID = other; }


		//Copy Assignment
		SerializableFormID& operator=(RE::FormID& a_rhs) { _formID = a_rhs; }

		//Copy Assignment with loose
		SerializableFormID& operator=(RE::FormID a_rhs) { _formID = a_rhs; }

		//Explicit move assignment
		SerializableFormID& operator=(RE::FormID&& a_rhs) { _formID = a_rhs; }


		// implicit conversion
		operator RE::FormID() const { return _formID; }

		// explicit conversion
		//explicit operator RE::FormID* () const { return (&_formID); }
	};

	//inline SerializableFormID test_serial_form;
	//inline SerializableFormID<handle_function> test_serial_form2;




	template<class Form>requires(std::derived_from<Form, RE::TESForm>)
	inline RE::FormID form_to_id(Form* form)
	{
		return form ? form->GetFormID() : 0x0;
	}
	template<class Form>requires(std::derived_from<Form, RE::TESForm>)
	inline Form* id_to_form(RE::FormID id, SerialArgument& serializer)
	{
		if (!id)
			return nullptr;

		Form* form = nullptr;

		serializer.ResolveFormID(id, form);

		return form;
	}
	//template<class Form = RE::TESForm>requires(std::derived_from<Form, RE::TESForm>)
	//using SerializableFormTest = RelinkPointer<Form, RE::FormID, form_to_id, id_to_form>;

	//RE::TESForm* test_form = nullptr;

	//using SerializableForm = SerializableForm<RE::TESForm>;
	//SerializableFormTest reg_form(test_form);

	/*
	template<std::derived_from<RE::TESForm> Form = RE::TESForm>
	class SerialForm final : public RelinkPointer<Form, RE::FormID, form_to_id, id_to_form>
	{
#define pointer_specific_bits 0xFFFFFFFF00000000
	public:
		bool IsForm(){ return (static_cast<uintptr_t>(RelinkPointer::_ptr) & pointer_specific_bits) == 0; }
	
	};
	//*/

	template<std::derived_from<RE::TESForm> Form = RE::TESForm>
	using SerialForm = RelinkPointer<Form, RE::FormID, form_to_id, id_to_form>;

	using SerialTESForm = SerialForm<RE::TESForm>;

	class SerialFormID final : public ISerializable
	{
	private:
		RE::FormID _formID;

	public:
		constexpr RE::FormID GetFormID() { return _formID; }

		//The original one is just fine.
		//SerializableBytes GetSerialized(bool& success);

		void OnDeserialize(SerialArgument& serializer, bool& success, ISerializable* pre_serialized) 
		{ 
			auto old_id = _formID;

			success = serializer.ResolveFormID(_formID); 

			logger::info("SerialFormID Resolution: {}, {:08X} -> {:08X}", success, old_id, _formID);
		}
	

		#pragma region Constructors_And_Assignment
		SerialFormID() = default;

		//Copy Constructor
		SerialFormID(const RE::FormID& other) { _formID = other; }
		SerialFormID(const RE::FormID&& other) { _formID = other; }
		SerialFormID(RE::FormID& other) { _formID = other; }
		SerialFormID(RE::FormID&& other) { _formID = other; }


		//Copy Assignment
		//SerialFormID& operator=(RE::FormID& a_rhs) { _formID = a_rhs; }

		//Copy Assignment with loose
		SerialFormID& operator=(RE::FormID a_rhs) { _formID = a_rhs;  return *this; }

		//Explicit move assignment
		//SerialFormID& operator=(RE::FormID&& a_rhs) { _formID = a_rhs; }

		// implicit conversion
		operator RE::FormID() const { return _formID; }

		//explicit conversion
		explicit operator RE::FormID*() { return (&_formID); }

		constexpr bool operator < (const SerialFormID& _Right) const {
			return _formID < _Right._formID;
		}
		#pragma endregion
	};
	


	template<typename R, typename... Args>
	struct FunctionTraitsBase
	{
		using RetType = R;
		using ArgTypes = std::tuple<Args...>;
		static constexpr std::size_t ArgCount = sizeof...(Args);
		template<std::size_t N>
		using NthArg = std::tuple_element_t<N, ArgTypes>;
	};

	template<typename F> struct FunctionTraits;

	template<typename R, typename... Args>
	struct FunctionTraits<R(*)(Args...)>
		: FunctionTraitsBase<R, Args...>
	{
		using Pointer = R(*)(Args...);
	};



	inline void f()
	{
		using T = void(*)();
		using R = FunctionTraits<T>::RetType; // error
	}



	//Initializer is having a rework, basically, it will work how it works now, is it will store everything it needs to initialize the class, then
	// when a transfer happens, it will run it's function

	//I think I can try to find a way to have it built in.

	/*This can and should have a set up where it will take a tuple for the init function parameters first, and parameter pack args for the classes constructor.
	//This initializer set up would allow some serious control.
	template <class StartType, class... InitParams>
	class Initializer
	{
	public:
		using Type = StartType;
		using InitFunc = void(StartType&, InitParams...);
	private:
		mutable StartType _target;
		//static_assert(sizeof(SerialzingWrapper<WrapType, REPLACE_WRAP_HANDLE>) != 0x0)
		//This will have to be initialized, but basically, all initializers need their goto location.
		// They are not objects that are allowed to persist. When they die is when they filfill their role.
		//StartType& _destination;
	public:
		constexpr StartType& GetTarget() { return _target; }

#pragma region Constructors_And_Assignment


		void Init(InitFunc* func, std::tuple<InitParams...>& args)
		{
			if (func) {
				auto send_args = std::tuple_cat(std::make_tuple(_target), args);
				std::apply(func, send_args);
			}
		}

		void Init(InitFunc* func, InitParams... args)
		{
			if (func)
				func(_target, args...);
		}

		Initializer(InitFunc* func)
		{
			static_assert(sizeof...(InitParams) == 0, "Default constructor cannot be used on an initializer without empty");

			Init(func);
		}

		template <class... ConstructorArgs>
		Initializer(InitFunc* func, std::tuple<InitParams...> init_args, ConstructorArgs... construct_args) : _target(construct_args...)
		{
			Init(func, init_args);
		}




		//I have no need for copy constructors.
		//Copy Constructor
		Initializer(InitFunc* func, InitParams... args)
		{
			static_assert(sizeof...(InitParams) != 0, "Typed constructor cannot be used on an initializer without being empty");

			Init(func, args...);
		}


		//Copy Constructor
		Initializer(StartType& other, InitFunc* func)
		{
			static_assert(sizeof...(InitParams) == 0, "Default constructor cannot be used on an initializer without empty");

			_target = other;
			Init(func);
		}


		//Copy Assignment
		//SerializingWrapper& operator=(WrapType& a_rhs) { _wrapObject = a_rhs; }

		//Copy Assignment with loose
		//SerializingWrapper& operator=(WrapType a_rhs) { _wrapObject = a_rhs; }

		//Explicit move assignment
		//SerializingWrapper& operator=(WrapType&& a_rhs) { _wrapObject = a_rhs; }


		//Copy Assignment
		template<std::convertible_to<StartType> TransType>
		Initializer& operator=(TransType& a_rhs) { _target = a_rhs;  return *this; }

		//Explicit move assignment
		template<std::convertible_to<StartType> TransType>
		Initializer& operator=(TransType&& a_rhs) { _target = a_rhs; return *this; }



		// implicit conversion
		operator StartType() const { return _target; }
		//operator WrapType&() const { return _wrapObject; }


		//template <class TypeFriend> requires(std::is_convertible<WrapType, TypeFriend>::value)
		//operator TypeFriend() const { return _wrapObject; }

		//explicit conversion
		explicit operator StartType* () const { return (&_target); }

		constexpr StartType* operator->() { return &_target; }
#pragma endregion
	};
	/*/

#define INITIALIZE(mc_reference, mc_function, ...) mc_reference = Initializer(mc_reference, mc_function, __VA_ARGS__)

	template <class StartType, class... InitParams>
	class Initializer
	{
	public:
		using Type = StartType;
		using InitializerType = Initializer<StartType, InitParams...>;
		using InitFunc = void(StartType&, InitParams...);
		using TupleType = std::tuple<StartType&, InitParams...>;//If I could build the destination in, I'd like to.
	private:
		mutable StartType _target;
		//static_assert(sizeof(SerialzingWrapper<WrapType, REPLACE_WRAP_HANDLE>) != 0x0)
		//This will have to be initialized, but basically, all initializers need their goto location.
		// They are not objects that are allowed to persist. When they die is when they filfill their role.
		// And if you never see it say it was deleted, then it didn't handle properly.

		InitFunc* _initFunction = nullptr;

		TupleType parameters;

		//StartType& _destination;//May not be needed.

	public:
		constexpr StartType& GetTarget() { return _target; }

#pragma region Constructors_And_Assignment

		constexpr Initializer(StartType& dest, InitFunc* func, InitParams... args) : _initFunction(func), parameters(dest, args...) {}

		constexpr Initializer(StartType& dest, InitFunc* func, TupleType args) : _initFunction(func), parameters(dest, args...) {}

		template <class... ConstructorArgs>
		constexpr Initializer(StartType& dest, InitFunc* func, TupleType args, ConstructorArgs... ctor_args) : _target(ctor_args...), _initFunction(func), parameters(dest, args...) {}

		~Initializer()
		{
			if (_initFunction) {
				std::apply(_initFunction, parameters);
				logger::info("{} successfully initialized from {}.", typeid(StartType).name(), typeid(InitializerType).name());
			}
			else {
				logger::info("{} unable to initialize from {}, init function not present.", typeid(StartType).name(), typeid(InitializerType).name());
			}
		}



		// implicit conversion. THis is the only thing I'll actually need.
		constexpr operator StartType() const { return _target; }
		//operator WrapType&() const { return _wrapObject; }


		//template <class TypeFriend> requires(std::is_convertible<WrapType, TypeFriend>::value)
		//operator TypeFriend() const { return _wrapObject; }

		//explicit conversion
		explicit operator StartType* () const { return (&_target); }

		constexpr StartType* operator->() { return &_target; }
#pragma endregion
	};
	//*/


	class IEncapsulateSerialDepthBase { constexpr int GetEncapsuleDepth() { return 0; } };


	class IChildDepthPrimaryBase { constexpr int GetPrimaryDepth() { return 0; } };


	template <std::uint8_t ENCAP_DEPTH>
	class IEncapsulateSerialDepth : public IEncapsulateSerialDepthBase { constexpr int GetEncapsuleDepth() { return ENCAP_DEPTH; } };

	template <std::uint8_t PRIME_DEPTH>
	class IChildDepthPrimary : public IChildDepthPrimaryBase { constexpr int GetPrimaryDepth() { return PRIME_DEPTH; } };

	//Primaries have to be dealt with differently. I think how I'll handle it is after loading the zero header and we're going through
	// primary serializers will push the depth to 1. Also all data that's placed will be put on 1, and any additional data will be stacked onto the
	// 1 slot of serialization data.
	class PrimarySerializer : public SerializationHandler//, public IEncapsulateSerialDepthBase, public IChildDepthPrimaryBase
	{
		//This shit is basically used to iterface with that dogshit serialConstructor,
		//Cause it's a template, we have no fucking way to get what its entry type is, hence, this weird shit.
	protected:
		SerializationHandler* _serializer{};
		SerializingFlag _flags{};
		//std::uint8_t _primeDepth;
		//std::uint8_t _encapDepth;
		//VersionID _incompatibleVersion;
		//void intro version//There is none.
		//int GetEncapsuleDepth() { return _encapDepth; }

		//int GetPrimaryDepth() { return _primeDepth; }

	public:
		//I actually think this could get away with just an ISerializer because I won't need a pointer to this.
		void HandleSerialize(SerialArgument& buffer, bool& success) override
		{
			constexpr int intro_version = 0;//There is none,
			logger::info("flags : {}", (unsigned int)_flags);
			success = buffer.Serialize(_serializer, intro_version, 0, _flags);
		}

		bool SetSerializer(SerializationHandler& prime_serializer, SerializingFlag flags)
		{
			if (_serializer) {
				logger::info("Prime Serializer is already full.");
				return false;
			}

			_serializer = &prime_serializer;
			_flags = flags;

			return true;
		}
	};



	enum class PrimaryRecordType : uint16_t//While one can use this, they don't actually have to.
	{
		CombatDataMap,
		CollisionMap
	};



	//inline InitializerTest<float, HandlePrimarySerializer2> test2;

	//inline SerialForm class_form;

	//static_assert(sizeof(SerialFormID) == sizeof(RE::FormID));
	//SerialFormID form_id = 0;
	//inline SerialForm<RE::TESObjectWEAP> weapon;
	//static_assert(sizeof(SerialForm<RE::TESObjectWEAP>) == sizeof(RE::TESObjectWEAP*));

	//While this is a class, I really only need a clump place to store data. Well, the private is useful too.
	//Also move this to main temporarily.
	struct MainSerializer
	{
	public:

		static bool Initialize(uint32_t serial_id)
		{
			if (_initialized)
				return true;

			if (auto serialization = SKSE::GetSerializationInterface())
			{
				serialization->SetUniqueID(serial_id);
				serialization->SetSaveCallback(SaveCallback);
				serialization->SetLoadCallback(LoadCallback);

				_initialized = true;

				logger::info("Serialization ID {} initialized. size {}.", serial_id, _primarySerializerMap.size());
				return true;
			}

			return false;
		}

		template <std::derived_from<SerializationHandler> DerivedHandler>
		static inline bool AddPrimarySerializer(size_t entry, DerivedHandler& handler)
		{
			//Needs more checks.
			auto& prime_entry = _primarySerializerMap[entry];

			SerializingFlag flags = SerializingFlag::None;

			flags |= is_open_serializer<DerivedHandler> ? SerializingFlag::OpenSerializing : SerializingFlag::None;

			return prime_entry.SetSerializer(handler, flags);
		}
	private:
		inline static bool _initialized = false;


		//inline static SerializableMap<size_t, PrimarySerializer> _primarySerializerMap;
		inline static std::map<size_t, PrimarySerializer> _primarySerializerMap;



		static void SaveCallback(SKSE::SerializationInterface* a_intfc)
		{
			//if (_crucibleState != PluginState::kLoaded)//I need this outside of main.
			//	return;
			//return;

			SerialArgument buffer(a_intfc, SerializingState::Serializing);

			//Serialize(argument);

			for (auto& [id, serializer] : _primarySerializerMap)
			{
				buffer.Serialize(id);
				buffer.Serialize(serializer);
			}
		}

		static void LoadCallback(SKSE::SerializationInterface* a_intfc)
		{
			//if (_crucibleState != PluginState::kLoaded)
			//	return;

			SerialArgument buffer(a_intfc, SerializingState::Deserializing);

			//Serialize(argument);
			//This makes me believe the id should be something special.
			//This way, no mistakes can occur when it's being set.
			size_t id = 0;

			while (buffer.IsFinished() == false)
			{
				if (buffer.Serialize(id) == false) {
					logger::error("ID failed to deserialize properly.");
					continue;
				}

				auto serial_entry = _primarySerializerMap.find(id);

				if (serial_entry == _primarySerializerMap.end()) {
					logger::error("ID {:X} is invalid.", id);
					continue;
				}

				buffer.Serialize(serial_entry->second);
			}
		}
		

	};




	//template<std::derived_from<IEncapsulateSerialDepth> Serializer> requires (s)
	template<std::derived_from<SerializationHandler> Serializer, class OverarchingType> requires(std::is_enum<OverarchingType>::value)
		inline void HandlePrimarySerializer(Serializer& serializer, OverarchingType type)
	{
		size_t entry = static_cast<size_t>(type);

		MainSerializer::AddPrimarySerializer(entry, serializer);
	}



	//inline CombatDataMap test_init = Initializer(test_init, HandlePrimarySerializer, PrimaryRecordType::CombatDataMap);
	//inline CombatDataMap INITIALIZE(test_init, HandlePrimarySerializer, PrimaryRecordType::CombatDataMap);

#define CreatePrimarySerializer(mc_reference, mc_record_type) INITIALIZE(mc_reference, HandlePrimarySerializer, mc_record_type);

}