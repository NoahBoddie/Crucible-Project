#pragma once

#include "Utility.h"

namespace Crucible
{

//#define working_new_event_object
#ifdef working_new_event_object


	class EventTypeArgException : public std::exception {
	public:
		const char* what() {
			return "Attempting to access event parameter [temped out] with invalid type.";
		}
	};

	template <class Type>
	class event_parameter;

	struct event_parameter_base
	{
	public:
		template<class Type>
		Type& GetParameter()
		{
			auto* param = dynamic_cast<event_parameter<Type>*>(this);

			if (!param)
				throw EventTypeArgException();

			return param->GetValue();
		}

		template <class Type>
		void SetParameter(Type& value)
		{
			auto* param = dynamic_cast<event_parameter<Type>*>(this);

			if (!param)
				throw nullptr;//This should throw a different exception, because it's not something that should be trivally skipped.

			param->SetValue(value);
		}

	protected:
		virtual void release() = 0;//friend class this.

		friend class event_parameter_pointer;
	};

	template<class Type>//as a rule, this object will not take anything greater than a pointer.
	struct event_parameter : public event_parameter_base
	{
		//Why not hold a copy of the actual object?
		// the ownership of this thing really weird me out.
		Type _paramValue;

	public:
		Type& GetValue() { return _paramValue; }

		void SetValue(Type& value) { _paramValue = value; }


		event_parameter(Type& value) { SetValue(value); }

		void release() override { delete this; }
	};


	/*//Old version, used new pointers. Mucho bad.
	template<class Type>//as a rule, this object will not take anything greater than a pointer.
	struct event_parameter : public event_parameter_base
	{
		//Why not hold a copy of the actual object?
		// the ownership of this thing really weird me out.
		Type* _paramValue;

	public:
		Type& GetValue() { return *_paramValue; }

		void SetValue(Type& value)
		{
			if (_paramValue)
				delete _paramValue;

			_paramValue = new Type(value);
		}


		event_parameter(Type& value)
		{
			SetValue(value);
		}


		void release() override { if (_paramValue) delete _paramValue; delete this; }
	};
	//*/


	class event_parameter_pointer final
	{
		event_parameter_base* _parameter;

	public:
		template<class Type>
		Type& GetParameter()
		{
			if (!_parameter)
				throw nullptr;//This should also take a diffent exception.

			return _parameter->GetParameter<Type>();
		}

		template <class Type>
		void SetParameter(Type& value)
		{
			if (!_parameter)
				throw nullptr;//This should throw a different exception, because it's not something that should be trivally skipped.

			_parameter->SetParameter<Type>();
		}

		event_parameter_pointer(event_parameter_base* const param)
		{
			_parameter = param;
		}

		//nothing like this, at least for now. I'd like to later though.
		//event_parameter_pointer& operator= (event_parameter_base*&& arg)

		~event_parameter_pointer()
		{
			if (_parameter)
				_parameter->release();
		}
	};



	//Goes in detail, not to be used.
	template <class RefType>
	class EventReference final
	{
	public:
		using Type = RefType;





	private:
		Type* _reference;
		//static_assert(sizeof(SerialzingWrapper<RefType, REPLACE_WRAP_HANDLE>) != 0x0)

	public:



#pragma region Constructors_And_Assignment
		EventReference() = default;

		//Copy Constructor
		EventReference(RefType& other) { _reference = &other; }

		//Copy Constructor
		EventReference(RefType other) { _reference = &other; }


		//Copy Assignment
		EventReference& operator=(RefType& a_rhs) { _reference = &a_rhs; return *this; }

		//Copy Assignment with loose
		//EventReference& operator=(RefType a_rhs) { _reference = a_rhs; }

		//Explicit move assignment
		//EventReference& operator=(RefType&& a_rhs) { _reference = a_rhs; }


		//Copy Assignment
		template<std::convertible_to<RefType> TransType>
		EventReference& operator=(TransType& a_rhs) { _reference = &a_rhs;  return *this; }

		//Explicit move assignment
		template<std::convertible_to<RefType> TransType>
		EventReference& operator=(TransType&& a_rhs) { _reference = &a_rhs; return *this; }



		// implicit conversion
		operator RefType& () const { logger::info("trigger00"); return *_reference; }
		//operator RefType&() const { return _reference; }


		//template <class TypeFriend> requires(std::is_convertible<RefType, TypeFriend>::value)
		//operator TypeFriend() const { return _reference; }

		//explicit conversion
		explicit operator RefType* () const { return _reference; }

		constexpr RefType* operator->() { return _reference; }
#pragma endregion
	};

	template< class T > struct remove_event_ref { typedef T type; };
	template< class T > struct remove_event_ref<EventReference<T>> { typedef T type; };


	template< class T > using remove_event_ref_t = typename remove_event_ref<T>::type;

	template<class T> struct is_event_ref { typedef T type; };
	template<class T> struct is_event_ref<EventReference<T>> : std::true_type { };


	template<typename Test, template<typename...> class Ref>
	struct is_specialization_of : std::false_type {};

	template<template<typename...> class Ref, typename... Args>
	struct is_specialization_of<Ref<Args...>, Ref> : std::true_type {};

	//A note, the pointer tuple, most certainly does not need THIS in it. Maybe factor it out some?

	template <size_t SIZE = 0, size_t I = 1, class PointerTuple, class FunctionTuple>
	inline typename std::enable_if<(I == std::tuple_size_v<PointerTuple> && I == std::tuple_size_v<FunctionTuple>), void>::type
		TestFuncLoop(PointerTuple& ref_params, FunctionTuple& func_params)
	{
		int fake_exception = 0;

		if (I < std::tuple_size_v<PointerTuple> || I < std::tuple_size_v<PointerTuple>)
			throw fake_exception;//I forgot my exception all the way down there, whoops

		return;
	}

	template <size_t SIZE = 0, size_t I = 1, class PointerTuple, class FunctionTuple>
	inline typename std::enable_if<(I < std::tuple_size_v<PointerTuple>&& I < std::tuple_size_v<FunctionTuple>), void>::type
		TestFuncLoop(PointerTuple& ref_params, FunctionTuple& func_params)
	{
		int fake_exception = 0;


		using PointerType = std::tuple_element<I, PointerTuple>::type;
		using FunctionType = std::tuple_element<I, FunctionTuple>::type;

		//fake_object = ParameterType();

		PointerType& ref_arg = std::get<I>(ref_params);

		bool underlying_same = std::is_same_v<std::remove_pointer<PointerType>::type, remove_event_ref<FunctionType>::type>;

		if (is_specialization_of<FunctionType, EventReference>::value)
		{
			if (underlying_same == false)
				throw fake_exception;

			//This code is temp for now, just want to see if it will go through.
			//Unfortunately, it's just dereferenced for now. Later, send the pointer.
			std::get<I>(func_params) = *ref_arg;
		}
		else
		{
			//Might not have to do this, incompatibility might handle it.
			if (underlying_same == false)
				throw fake_exception;

			std::get<I>(func_params) = *ref_arg;
		}

		if (I < SIZE)
			TestFuncLoop<SIZE, I + 1>(ref_params, func_params);
	}



	//Tese are temp names.
	template <class Type>
	class EventArgTuple;

	struct EventArgBase
	{
	public:
		template<class TupleType>
		TupleType& GetTuple()
		{
			auto* tuple = dynamic_cast<EventArgTuple<TupleType>*>(this);

			if (!tuple)
				throw EventTypeArgException();

			return tuple->GetTuple();
		}

	protected:
		virtual void release() = 0;//friend class this.

		friend class event_arg_pointer;
	};

	template<class Tuple>//as a rule, this object will not take anything greater than a pointer.
	struct EventArgTuple : public EventArgBase
	{
		//Why not hold a copy of the actual object?
		// the ownership of this thing really weird me out.
		Tuple _tuple;

	public:
		Tuple& GetTuple() { return _tuple; }

		EventArgTuple(Tuple& value) { _tuple = value; }

		void release() override { logger::info("deleting tuple"); delete this; }
	};

	class event_arg_pointer final
	{
		EventArgBase* _tuple;

	private:
		void release()
		{
			if (_tuple)
				_tuple->release();

			_tuple = nullptr;
		}

	public:
		template<class TupleType>
		TupleType& GetTuple()
		{
			if (!_tuple)
				throw nullptr;//This should also take a diffent exception.

			return _tuple->GetTuple<TupleType>();
		}

		template<class... Args>
		void SetTuple(EventArgBase* const new_tuple)
		{
			release();

			_tuple = new_tuple;
		}

		event_arg_pointer() = default;

		event_arg_pointer(EventArgBase* const new_tuple)
		{
			_tuple = new_tuple;
		}

		operator bool() { return _tuple != nullptr; }


		//nothing like this, at least for now. I'd like to later though.
		//event_parameter_pointer& operator= (event_parameter_base*&& arg)

		~event_arg_pointer() { logger::info("release tuple pointer"); release(); }
	};



	//#define __PRIVATE_GET_EVENT_FUNCTION(type, a_name) auto event_function = &Type::a_name##_INTERNAL
	//the void is done manually, to look like a function
	// The first 2 parameters of internal additionally, are to be event object and control. This way, you still have impact on those.


#define EVENT(a_name, ...)									\
a_name(std::vector<event_parameter_pointer>& parameters)	\
{															\
	using RefType = decltype(*this);						\
	using Type = std::remove_reference<RefType>::type;		\
	auto event_function = &Type::a_name##_INTERNAL;			\
	TestFunc<Type>(this, event_function, parameters);		\
}															\
void a_name##_INTERNAL(__VA_ARGS__)							\

	using EventObject = LegacyEventParameter;
	enum EventControl;


	class LegacyEventParameter final
	{
	private:
		event_arg_pointer _argumentPack;

		std::vector<event_parameter_pointer>& _argumentList;






		template <size_t SIZE = 0, size_t I = 1, class... Ts>
		inline typename std::enable_if<I == sizeof...(Ts), void>::type
			UnpackTuple(std::tuple<Ts...>& send_params) { return; }

		template <size_t SIZE = 0, size_t I = 1, class... Ts>
		inline typename std::enable_if<(I < sizeof...(Ts)), void>::type
			UnpackTuple(std::tuple<Ts...>& send_params)
		{
			using TupleType = std::tuple<Ts...>;
			using ParameterType = remove_event_ref_t<std::tuple_element<I, TupleType>::type>;

			//fake_object = ParameterType();

			ParameterType& parameter = parameters[I - 1].GetParameter<ParameterType>();


			std::get<I>(send_params) = parameter;//fake_object;

			constexpr if (I < SIZE)
				UnpackTuple<SIZE, I + 1>(send_params, _argumentList);
		}

		template <class TupleType, class This>
		inline void UpdateArgumentPack(EventControl& control)
		{
			TupleType ref_params;

			using ParameterType = remove_wrapper_t<std::tuple_element<I, TupleType>::type>;

			std::get<1>(ref_params) = *this;
			std::get<1>(ref_params) = control;


			UnpackTuple<std::tuple_size_v<TupleType>>(ref_params, parameters);
			_argumentPack.SetTuple(new EventArgTuple<TupleType>(ref_params));
		}

	public:
		template <class This, class... Args>
		void UnpackEvent(This* a_this, void(This::* event_function)(Args...), EventControl& event_control)
		{
			//CONFUTE, this needs to be loaded with an event object ref type, and an event control.
			//using RefTuple = std::tuple<This*, EventReference<std::remove_reference_t<remove_event_ref_t<Args>>>...>;
			//We're not gonna remove event reference, as it's not really going to be visible in a bit.
			//Also move i back 1.

			using FirstElement = std::tuple_element<1, std::tuple<Args...>>::type;
			using SecondElement = std::tuple_element<2, std::tuple<Args...>>::type;

			static_assert(std::is_same_v<EventObject, std::remove_reference<FirstElement>::type>,
				"First argument of event is not an LegacyEventParameter.");

			static_assert(std::is_same_v<EventObject&, FirstElement>,
				"First argument LegacyEventParameter must be a reference.");


			static_assert(std::is_same_v<EventControl, std::remove_reference<SecondElement>::type>,
				"First argument of event is not an LegacyEventParameter.");

			static_assert(std::is_same_v<EventControl&, SecondElement>,
				"Second argument EventControl must be a reference.");



			using RefTuple = std::tuple<EventReference<std::remove_reference_t<Args>>...>;

			if (!_argumentPack)
				UpdateArgumentPack<RefTuple>(a_this, event_control);

			//If the types don't match, it will send an exception, forcing that event to fail.
			auto ref_args = _argumentPack.GetTuple<RefTuple>();

			std::apply(TestFunction, ref_args);
		}




		//I'm deleting the copy constructors, for the main reason if I never, EVER, want the data in here to be replicated.

	};

#endif

//#define enable_legacy_event_token183183
#ifdef enable_legacy_event_token183183




	//Revisions that I was going to do was something along the lines of this
	// Event parameters hold a void pointer, and a type hash. Effectively, the 
	// ability to prevent it from going hay wire is gonna be something like checking the
	// type hashcode to see if I should load it. Basically, it can tell you what type it's supposed
	// to be, and to tell if the third, fourth, whatever is, you have to submit a type.
	//You can optionally, also submit the actual thing you want to send stuff to, which will then auto fill
	// IF I CAN, I'd like it so that you can submit a series of variables, and if one of them fails to be gotten, it will
	// say NO. and you can cancel your event.

	//This probably shouldn't handled template types.

	//Last thing, for sending events, it would be nice to have a function I can just use template types
	// and parameter packs for the fucking event object, cause loading that shit is a pain and it looks ugly.
	// but if you change that bit, make it have legacy support.


	//To validate the purpose of this, I need it for the event system to work with the serialization system, unless I want this shit to
	// be annoyingly specific. THIS bit, will need a sort of global set up. So, it only goes from global to combat data, not further than that.
	
	//To sum up the pros, if templates aren't used parameter validation would be very simple and more accurate, I could use a wider variety of 
	// parameters, it can finally work with serialization, and lastly, should I implement the template types, pulling and setting
	// would likely be a lot easier.

	//Lastly, once I make a global one, please please please PLEASE. Do exception checking for functions not successfully firing
	// in the event.



	class EventObject;

	struct LegacyEventParameter
	{
	private:
		enum ParameterValidation
		{
			kInvalid = 0,
			kForm = 1,
			kBool = 2,
			kFloat = 3,
			kInteger = 4,
			kEnum = 5,
			kString = 6
		};
		//Combine these, it saves space (i think)
		bool _locked = false;

		bool _dirty = false;//Something is marked dirty if it's been set once, and is edited after that again.

		friend class EventObject;
	

	private:
		void MarkDirty() { if (flag != kInvalid) _dirty = true; }

	public:

		ParameterValidation flag;

		std::string stringValue;//32 bits



		//What I'm going to do here is this object should be able to hold pointers to things.
		// THEN the conversion type to that pointer is stored along with it. It will increase the size, but I think I can just get rid of parameter 
		// validation in that case. Give it a test run and see if it breaks down.
		union
		{
			RE::FormID		formValue;	//32 bits
			bool			boolValue;	//1 bit
			float			floatValue;	//32 bits
			std::int32_t	intValue;	//32 bits, same as form id, doubles as enum val
		};
		


		bool AsFloat(float& value)
		{
			if (flag != kFloat)
				return false;

			value = floatValue;
			return true;
		}

		bool AsBool(bool& value)
		{
			if (flag != kBool)
				return false;

			value = boolValue;
			return true;
		}

		bool AsInt(std::int32_t& value)
		{
			if (flag != kInteger)
				return false;

			value = intValue;
			return true;

		}

		bool AsString(std::string& value)
		{
			if (flag != kString)
				return false;

			value = stringValue;
			return true;

		}

		bool AsFormID(RE::FormID& value)
		{
			if (flag != kForm) {

				return false;
			}

			value = formValue;

			return true;
		}

		bool AsForm(RE::TESForm*& form)
		{
			RE::FormID formID;

			if (AsFormID(formID) == false) {

				return false;
			}

			form = RE::TESForm::LookupByID(formID);

			return true;
		}

		template <class T>
		bool AsFormType(T*& form)
		{
			RE::TESForm* form_obj = nullptr;

			if (AsForm(form_obj) == false) {
				
				return false;
			}

			T* form_as = form_obj ? form_obj->As<T>() : nullptr;

			if (!form_as) {
				
				return false;
			}
			form = form_as;

			return true;
		}


		template <class Enumeration>
		bool AsEnum(Enumeration& value)
		{
			static_assert(std::is_enum<Enumeration>::value, "Provided type is not an enumeration.");

			if (flag != kEnum)
				return false;

			value = static_cast<Enumeration>(intValue);
			return true;
		}




		float AsFloat()
		{
			float value;
			
			if (AsFloat(value) == false)
				return 0;

			return value;
		}

		bool AsBool()
		{
			bool value;

			if (AsBool(value) == false)
				return false;

			return value;
		}

		std::int32_t AsInt()
		{
			std::int32_t value;

			if (AsInt(value) == false)
				return 0;

			return value;
		}

		std::string AsString()
		{
			std::string value;

			if (AsString(value) == false)
				return "";

			return value;

		}

		RE::FormID AsFormID()
		{
			RE::FormID value;

			if (AsFormID(value) == false)
				return 0;

			return value;
		}

		RE::TESForm* AsForm()
		{
			RE::TESForm* value;

			if (AsForm(value) == false)
				return nullptr;

			return value;
		}

		template <class T>
		T* AsFormType()
		{
			T* value;

			if (AsFormType(value) == false)
				return nullptr;

			return value;
		}


		template <class Enumeration>
		Enumeration AsEnum()
		{
			Enumeration value;

			if (AsEnum(value) == false)
				return (Enumeration)0;

			return value;
		}


		bool IsInvalid()
		{
			return flag == kInvalid;
		}

		bool IsDirty() { return _dirty; }


		void SetForm(const RE::TESForm* a_rhs) {
			if (!_locked)
			{

				if (!a_rhs)
					formValue = 0x0;
				else
					formValue = a_rhs->GetFormID();

				MarkDirty();
			
				flag = kForm;
			}
		}
		//*/

		//*

		void SetBool(const bool& a_rhs) 
		{
			if (!_locked)
			{
				MarkDirty();
				boolValue = a_rhs;
				flag = kBool;
			}
		}

		
		void SetFloat(const float& a_rhs) {
			if (!_locked)
			{
				MarkDirty();
				floatValue = a_rhs;
				flag = kFloat;
			}
		}


		void SetInteger(const std::int32_t& a_rhs) {
			if (!_locked)
			{
				MarkDirty();
				intValue = a_rhs;
				flag = kInteger;
			}
		}

		//*/
		void SetString(const std::string& a_rhs) {
			if (!_locked)
			{
				MarkDirty();
				stringValue = a_rhs.c_str();
				flag = kString;
			}
		}


		//push enum isn't going to work. Not like this. Needs more manual of a deal.
		template <typename Enumeration> requires (std::is_enum<Enumeration>::value)
		void SetEnum(const SKSE::stl::enumeration<Enumeration, std::int32_t>& a_rhs) 
		{
			if (!_locked)
			{
				MarkDirty();
				//SKSE::stl::enumeration<Enumeration, std::int32_t> enumValue = a_rhs;
				intValue = a_rhs.underlying();//static_cast<std::int32_t>(a_rhs);//(int)a_rhs;//
				flag = kEnum;
			}
		}





		
	};

	class EventObject
	{
	private:
		using StringHash = uint32_t;

		std::vector<LegacyEventParameter> _paramList;
		
		LegacyEventParameter _end;

		//std::map<StringHash, LegacyEventParameter> parameterMap;
		//int size;// The size of the indexed objects, IF psuedo list is done.

		//Effectively makes the parameters constants.
		bool _locked = false;

		std::string _eventName;
	public:
	
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		/*
		//I will keep this, however I seek to use lists for now. I may switch to psuedo list if I start to use 
		// indeterminate amounts of values. This would effectively remove _x as a value allowed to be used.
		// basically, if the submitted object is indeterminate, I could make it switch to effectively have a map system
		// for specific values.
		LegacyEventParameter& operator[] (std::string& param)
		{
			auto s_hash = hash(param.c_str(), param.size());

			//If this is locked, making new parameters is forbidden, so make it ask if it contains or not.
			
			auto result = parameterMap.find(s_hash);
			if (_locked && parameterMap.end() == result)
			{
				auto emptyHash = ""_h;
				auto ep = parameterMap[emptyHash];
				ep._locked = true;
				return ep;
			}

			auto parameter = result->second;//parameterMap[s_hash];

			parameter._locked = _locked;

			return parameter;
		}
		//*/

		LegacyEventParameter& operator[] (int index)
		{
			//If this is locked, making new parameters is forbidden, so make it ask if it contains or not.
			if (index >= _paramList.size() || index < 0)
			{
				RE::DebugMessageBox("END HIT");

				_end._locked = true;
				return _end;
			}

			auto& parameter = _paramList[index];

			parameter._locked = _locked;

			return parameter;
		}


		void PushFloat(float value)
		{
			if (_locked)
				return;

			LegacyEventParameter parameter;
			//parameter = value;
			parameter.SetFloat(value);
			_paramList.push_back(parameter);
		}

		void PushBool(bool value)
		{
			if (_locked)
				return;

			LegacyEventParameter parameter;
			//parameter = value;
			parameter.SetBool(value);
			_paramList.push_back(parameter);
		}

		void PushInt(std::int32_t value)
		{
			if (_locked)
				return;

			LegacyEventParameter parameter;
			//parameter = value;
			parameter.SetInteger(value);
			_paramList.push_back(parameter);
		}

		void PushString(std::string value)
		{
			if (_locked)
				return;

			LegacyEventParameter parameter;
			//parameter = value;
			parameter.SetString(value);
			_paramList.push_back(parameter);
		}

		/*Unused.
		void PushFormID(RE::FormID value)
		{
			if (_locked)
				return;

			LegacyEventParameter parameter;
			parameter = value;
			_paramList.push_back(parameter);
		}
		//*/

		void PushForm(RE::TESForm* value)
		{
			if (_locked)
				return;

			LegacyEventParameter parameter;
			//parameter = value;
			parameter.SetForm(value);
			_paramList.push_back(parameter);
		}

		template <class Enumeration>
		void PushEnum(Enumeration& value)
		{
			if (_locked)
				return;
			SKSE::stl::enumeration<Enumeration, std::int32_t> enumValue = value;

			LegacyEventParameter parameter;
			//parameter = enumValue;
			parameter.SetEnum(enumValue);
			_paramList.push_back(parameter);
		}


		void SetEventName(std::string new_name)
		{
			if (_eventName == "")
				_eventName = new_name;
		}

		void LockEvent()
		{
			_locked = true;
		}
	};

#endif
}