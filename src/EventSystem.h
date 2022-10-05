#pragma once

#include "Utility.h"
//#include "EventObject.h"//Disabled for the time being.

namespace Crucible
{

#pragma region NewEventSystem

	class EventTypeArgException : public std::exception {
	public:
		const char* what() {
			return "Attempting to access event parameter [temped out] with invalid type.";
		}
	};


	template <class RefType>
	class RefWrapper final
	{
	public:
		using Type = RefType;





	private:
		Type* _reference;
		//static_assert(sizeof(SerialzingWrapper<RefType, REPLACE_WRAP_HANDLE>) != 0x0)

	public:



#pragma region Constructors_And_Assignment
		RefWrapper() = default;

		//Copy Constructor
		RefWrapper(RefType& other) { _reference = &other; }

		//I feel like this would cause this to be in error.
		// I believe I maybe wanted this object to have ownership or something, there was a reason that couldn't happen though.
		//Copy Constructor
		//RefWrapper(RefType other) { _reference = &other; }


		//Copy Assignment
		RefWrapper& operator=(RefType& a_rhs) { _reference = &a_rhs; return *this; }

		//Copy Assignment with loose
		//RefWrapper& operator=(RefType a_rhs) { _reference = a_rhs; }

		//Explicit move assignment
		//RefWrapper& operator=(RefType&& a_rhs) { _reference = a_rhs; }


		//Copy Assignment
		template<std::convertible_to<RefType> TransType>
		RefWrapper& operator=(TransType& a_rhs) { _reference = &a_rhs;  return *this; }

		//Explicit move assignment
		template<std::convertible_to<RefType> TransType>
		RefWrapper& operator=(TransType&& a_rhs) { _reference = &a_rhs; return *this; }



		// implicit conversion
		operator RefType& () const { return *_reference; }
		//operator RefType&() const { return _reference; }


		//template <class TypeFriend> requires(std::is_convertible<RefType, TypeFriend>::value)
		//operator TypeFriend() const { return _reference; }

		//explicit conversion
		explicit operator RefType* () const { return _reference; }

		constexpr RefType* operator->() { return _reference; }
#pragma endregion
	};




	enum class RequireFlags
	{
		None = 0,
		IsReference = 1 << 0,
		RequiresNot = 1 << 1,
		RefRequiresNot = IsReference | RequiresNot
	};



	//Might replace this with a ref wrapper
	template <class RefType, RequireFlags Flags = RequireFlags::None>
	class EventRequirement final
	{
	public:
		static inline constexpr bool Is_Reference = enum_contains<RequireFlags, Flags, RequireFlags::IsReference>;
		static inline constexpr bool Is_Pointer = std::is_pointer_v<RefType>;
		static inline constexpr RequireFlags OppositeRefFlag = Flags ^ RequireFlags::IsReference;

		using Type = std::conditional_t<Is_Reference, RefType*, RefType>;
		using PointerType = std::remove_pointer_t<RefType>;





	private:
		Type _value;
		//static_assert(sizeof(SerialzingWrapper<RefType, REPLACE_WRAP_HANDLE>) != 0x0)

		RefType& GetValue() {
			if constexpr (Is_Reference)
				return *_value;
			else
				return _value;
		}



		void SetValue(RefType& other) {
			if constexpr (Is_Reference)
				_value = &other;
			else {
				_value = other;
			}
		}

		friend EventRequirement<RefType, RequireFlags::None>;
		friend EventRequirement<RefType, RequireFlags::IsReference>;
		friend EventRequirement<RefType, RequireFlags::RequiresNot>;
		friend EventRequirement<RefType, RequireFlags::RefRequiresNot>;
		
	public:



#pragma region Constructors_And_Assignment
		//In order to use default it needs not be a reference, so it can just default create the object
		EventRequirement()requires(!Is_Reference) = default;

		//Copy Constructor
		EventRequirement(RefType& other) : _value(other) { SetValue(other); }
		EventRequirement(RefType&& other) requires(!Is_Reference) : _value(other) { SetValue(other); }

		template<RequireFlags OtherFlags>
		EventRequirement(EventRequirement<RefType, OtherFlags>& other) { SetValue(other.GetValue()); }


		//I feel like this would cause this to be in error.
		// I believe I maybe wanted this object to have ownership or something, there was a reason that couldn't happen though.
		//Copy Constructor
		//EventRequirement(RefType other) { _reference = &other; }


		//Copy Assignment
		EventRequirement& operator=(RefType&& a_rhs)requires(!Is_Reference) { SetValue(a_rhs); return *this; }

		//Copy Assignment with loose
		//EventRequirement& operator=(RefType a_rhs) { _reference = a_rhs; }

		//Explicit move assignment
		//EventRequirement& operator=(RefType&& a_rhs) { _reference = a_rhs; }


		//Copy Assignment
		template<std::convertible_to<RefType> TransType>
		EventRequirement& operator=(TransType& a_rhs) { SetValue(a_rhs);  return *this; }

		//Explicit move assignment
		template<std::convertible_to<RefType> TransType>
		EventRequirement& operator=(TransType&& a_rhs) { SetValue(a_rhs); return *this; }



		// implicit conversion
		operator RefType& () { return GetValue(); }
		operator RefType& () const { return GetValue(); }
		//operator RefType&() const { return _reference; }
		RefType& operator*() { return GetValue(); }

		operator bool() const requires(Is_Reference) { return _value != nullptr; }


		//template <class TypeFriend> requires(std::is_convertible<RefType, TypeFriend>::value)
		//operator TypeFriend() const { return _reference; }

		//explicit conversion
		explicit operator PointerType* () const { if constexpr (Is_Pointer)return GetValue(); else return &GetValue(); }


		constexpr PointerType* operator->() noexcept { if constexpr (Is_Pointer)return GetValue(); else return &GetValue(); }
#pragma endregion
	};


	template <class RefType, bool Is_Reference = false>
	using RequireReference = EventRequirement<RefType, Is_Reference ? RequireFlags::IsReference : RequireFlags::None>;

	template <class RefType, bool Is_Reference = false>
	using DisallowReference = EventRequirement<RefType, Is_Reference ? RequireFlags::RefRequiresNot : RequireFlags::RequiresNot>;



	template <class RefType, bool Is_Reference = false>
	using RequireReference = EventRequirement<RefType, Is_Reference ? RequireFlags::IsReference : RequireFlags::None>;

	template <class RefType, bool Is_Reference = false>
	using DisallowReference = EventRequirement<RefType, Is_Reference ? RequireFlags::RefRequiresNot : RequireFlags::RequiresNot>;
	
	//template< class T >requires(requires_reference_v<T>) struct make_disallow_ref { typedef T type; }
	//template< class T >requires(!requires_reference_v<T>) struct make_disallow_ref { typedef DisallowReference<T, true> type; };




	template< class T > struct remove_ref_wrapper { typedef T type; };
	template< class T > struct remove_ref_wrapper<RefWrapper<T>> { typedef T type; };


	template< class T > using remove_ref_wrapper_t = typename remove_ref_wrapper<T>::type;



	template< class T > struct remove_require_ref { typedef T type; };
	//template< class T > struct remove_require_ref<RequiresReference<T, true>> { typedef T type; };
	//template< class T > struct remove_require_ref<RequiresReference<T, false>> { typedef T type; };
	template< class T > struct remove_require_ref<EventRequirement<T, RequireFlags::None>> { typedef T type; };
	template< class T > struct remove_require_ref<EventRequirement<T, RequireFlags::IsReference>> { typedef T type; };
	template< class T > struct remove_require_ref<EventRequirement<T, RequireFlags::RequiresNot>> { typedef T type; };
	template< class T > struct remove_require_ref<EventRequirement<T, RequireFlags::RefRequiresNot>> { typedef T type; };
	


	template< class T > using remove_require_ref_t = typename remove_require_ref<T>::type;



	template <class T> struct is_requirement { static inline constexpr bool value = !std::is_same_v<remove_require_ref<T>, T>; };
	template <class T> inline constexpr bool is_requirement_v = is_requirement<T>::value;


	template <class T> struct requires_reference : std::false_type {};
	//template <class T> struct requires_reference<RequiresReference<T, true>> : std::true_type {};
	//template <class T> struct requires_reference<RequiresReference<T, false>> : std::true_type {};
	template< class T > struct requires_reference<EventRequirement<T, RequireFlags::None>> : std::true_type {};
	template< class T > struct requires_reference<EventRequirement<T, RequireFlags::IsReference>> : std::true_type {};
	template< class T > struct requires_reference<EventRequirement<T, RequireFlags::RequiresNot>> : std::false_type {};
	template< class T > struct requires_reference<EventRequirement<T, RequireFlags::RefRequiresNot>> : std::false_type {};

	template <class T> inline constexpr bool requires_reference_v = requires_reference<T>::value;


	template <class T> struct disallows_reference : std::false_type {};
	//template <class T> struct requires_reference<RequiresReference<T, true>> : std::true_type {};
	//template <class T> struct requires_reference<RequiresReference<T, false>> : std::true_type {};
	template< class T > struct disallows_reference<EventRequirement<T, RequireFlags::None>> : std::false_type {};
	template< class T > struct disallows_reference<EventRequirement<T, RequireFlags::IsReference>> : std::false_type {};
	template< class T > struct disallows_reference<EventRequirement<T, RequireFlags::RequiresNot>> : std::true_type {};
	template< class T > struct disallows_reference<EventRequirement<T, RequireFlags::RefRequiresNot>> : std::true_type {};
	//template <class T> struct disallows_reference { static inline constexpr bool value = (is_requirement_v<T> && !requires_reference_v<T>); };
	template <class T> inline constexpr bool disallows_reference_v = disallows_reference<T>::value;



	//template <class T> struct requires_reference<T&&> : std::true_type {};
	


	template <class... Args>
	class EventParameter;

	class EventArgument;

	using RequireCode = std::uint64_t;


	struct EventParamBase
	{
	public:

		template<class FirstArg, class... Args> requires(std::is_same_v<std::remove_reference_t<FirstArg>, EventArgument>)
		auto& GetParameters()
		{
			//The EventArument version

			static_assert(std::is_same<FirstArg, EventArgument&>::value, "The first optional value must be an EventArgument&.");
			//if (!std::is_same<FirstArg, EventArgument&>::value) {}

			return GetParameters<Args...>();
		}
		//The idea is, here we remove the reference stuff but requires reference still remains
		// The problem is, that the specification would be in parameters, and the call pack is the thing that requests 
		// So what would happen is the call pack most certainly would not have that version.
		// If this is the case, I do the reverse. I don't remove references, I "add" them.
		//I make a helper struct that will do nothing to normal types, but if it's a reference the type it will submit will be
		// one that requires references. If any references (of course outside the optional) are detected, it will do a reference version first,
		// if this fails it will do the regular one first which will then perform it's exception.
		// A HUGE note, event parameters should be able to say that they have required reference types.
		template<class... Args>
		auto& GetParameters()
		{
			//The non-optional version
			

			if (HasRequirement() == true)
			{
				RequireCode call_code = GetRequireCode<std::is_reference_v<Args>...>();
				
				const char* error_message = nullptr;

				if (IsRequirementValid(call_code, error_message) == false) {
					logger::info("A type (or types) require a reference to be used,{}. Types:{}", error_message, typeid(std::tuple<Args...>).name());
					PrintParameters();
					throw EventTypeArgException();
				}
			}


			using EventParamType = EventParameter<std::remove_reference_t<Args>...>;
			using EventTupleType = std::tuple<RefWrapper<std::remove_reference_t<Args>>...>;

			auto comp_code = typeid(EventTupleType).hash_code();

			if (GetTypeCode() != comp_code) {
				logger::info("DDDDDD {}", typeid(EventParamType).name());
				PrintParameters();
				throw EventTypeArgException();
			}


			EventParamType* event_param = static_cast<EventParamType*>(this);

			return event_param->GetParameters();
		}




		//play with the template of this function, see what I can make a constexpr or not.
		template <bool... bits>
		constexpr RequireCode GetRequireCode()
		{
			static_assert(sizeof...(bits) <= sizeof(RequireCode), "GetRequireCode requires equal to or less parameters than sizeof(RequireCode).");

			std::array values{ bits... };

			RequireCode code = 0;

			for (RequireCode i = 0; i < values.size(); i++)
			{
				if (values[i] == true)
					code |= (1 << i);
			}

			return code;

		}

		virtual bool HasRequirement() { return false; }

		virtual std::size_t GetTypeCode() = 0;

		virtual bool IsRequirementValid(RequireCode require_code, const char*& error_string) { return true; }
		
		virtual void PrintParameters() = 0;

		virtual ~EventParamBase() = default;

		//friend class EventParamPointer_OLD;
	};



	template<class... Args>
	struct EventParameter : public EventParamBase
	{
	public:
		//using TupleType = std::tuple<RefWrapper<remove_require_ref<Args>>...>;//we are still using this
		//using Self = std::tuple<RefWrapper<Args>...>;
		using TupleType = std::tuple<RefWrapper<Args>...>;
	
	public:
		TupleType _tuple;

		TupleType& GetParameters() { return _tuple; }
		

		std::size_t GetTypeCode() override { return typeid(_tuple).hash_code(); }

		void PrintParameters() override { logger::info(typeid(std::tuple<Args...>).name()); }

		EventParameter(Args&... args) { _tuple = std::make_tuple<RefWrapper<Args>...>(args...); }

		//EventParameter(std::tuple<Args...>& value) { __super::_tuple = value; }
		~EventParameter() override = default;
	};
	

	template<class... Args>
	struct RequiredEventParameter final : public EventParameter<remove_require_ref_t<Args>...>
	{
		using BaseParameter = EventParameter<remove_require_ref_t<Args>...>;
	private:
		//I can dynamically type this information with different integrals,  dynamically adjusting how much data is take up for this.
		// I can 
		RequireCode _eventRequireCode{ 0 };
		RequireCode _eventRequireNotCode{ 0 };
	public:
		RequiredEventParameter(Args&... args) : BaseParameter(args...)
		{
			_eventRequireCode = EventParamBase::GetRequireCode<requires_reference_v<Args>...>();
			_eventRequireNotCode = EventParamBase::GetRequireCode<disallows_reference_v<Args>...>();
			//__super::_hasRequirement = true;
		}


		bool HasRequirement() override { return true; }

		bool IsRequirementValid(RequireCode call_require_code, const char*& error_string) override
		{ 
			RequireCode and_code = _eventRequireCode & call_require_code;
			
			if (and_code != _eventRequireCode) {
				error_string = "Required Reference not found";
				return false;
			}

			RequireCode x_and_code = _eventRequireNotCode & call_require_code;

			if (x_and_code != 0) {

				logger::info("{} vs {} == {}",_eventRequireNotCode, call_require_code, x_and_code);
				error_string = "Disallowed Reference detected";
				return false;
			}

			return true;
		}

		//EventParameter(std::tuple<Args...>& value) { __super::_tuple = value; }
		~RequiredEventParameter() override = default;
	};


	template <class... Args>requires(all_true<!is_requirement_v<Args>...>::value)
	EventParameter<Args...> CreateEventParameter(Args&... args)
	{
		return EventParameter(args...);
	}


	template <class... Args>
	RequiredEventParameter<Args...> CreateEventParameter(Args&... args)
	{
		return RequiredEventParameter(args...);
	}
	

	struct ZeroArgEventParameter final : public EventParamBase
	{
		//As it's name implies, it's null. This is used if the value is just something that utterly doesn't require the use of
		// any packed parameters.
	public:
		void PrintParameters() { logger::info("No Parameters"); }

		std::size_t GetTypeCode() override { return 0; }

		~ZeroArgEventParameter() override = default;
	};

	//In order to get the thing I need, I need to create a function to execute this, that will naturally enforce all the references I wish.
	//The only issue is that the call parameter purges it's references for match, how can I pick the ones I want and leave the ones I don't without knowing which are
	// which?


	//Not needed
	using EventParamPointer = std::unique_ptr<EventParamBase>;

	class EventParamPointer_OLD final
	{
		EventParamBase* _params;

	private:
		void release()
		{
			//if (_params)
			//	_params->release();

			_params = nullptr;
		}

	public:
		template<class... Args>
		auto& GetParameters()
		{
			if (!_params)
				throw nullptr;//This should also take a diffent exception.

			return _params->GetParameters<Args...>();
		}

		void SetParameters(EventParamBase* const new_params)
		{
			release();

			_params = new_params;
		}

		EventParamPointer_OLD() = default;

		EventParamPointer_OLD(EventParamBase* const new_params)
		{
			_params = new_params;
		}

		operator bool() { return _params != nullptr; }


		//nothing like this, at least for now. I'd like to later though.
		//event_parameter_pointer& operator= (event_parameter_base*&& arg)

		//~EventParamPointer_OLD() { logger::info("release param pointer"); release(); }
	};

	//Event control, as the name on the tin
	enum class EventControl
	{
		kContinue,													//Continue the loop. Simple.
		kContinueLocal,												//Disallow the event to reach global.
		kContinueGlobal,											//Finishes the local events and skips to the global.
		kStop,														//Stop the event outright.
		kGroupStopSpecific = kStop << 1,							//Stops ALL specific events, setting this event value for all of them past and performing general.
		kGroupStopGeneral = kStop << 2,								//Stops the General event but continues specific ones
		kGroupStopMain = kGroupStopSpecific | kGroupStopGeneral,	//Stops ALL events in that main name
		kGroupStopAll = kGroupStopMain | kStop,						//Stops ALL events in the whole group
		kError,														//An error was encountered, event didn't resolve properly
		kNoEvent													//No actual event was detected
	};


	//Event initialization flags control the starting parameters of an event, whether like whether it should start locked
	// or if it's first come first served on events.
	enum class EventInitFlags
	{
		None = 0,					//Does nothing smile
		Locked = 1 << 0,			//Prevents events from changing parameters without forcing const.
		FirstRetPriority = 1 << 1	//This value prioritizes the first return value, and prevents any further ones from being loaded.
	};
	
	
	struct GroupEventControl
	{
		using GroupEventControlMap = std::vector<std::pair<std::string, EventControl>>;

		//I'll likely need to move onto paired vectors, but for now I'm gonna give this system a test to see if it functions properly
	private:
		GroupEventControlMap _controlResultMap;

		constexpr GroupEventControlMap& GetMap() { return _controlResultMap; }

		friend class EventHandler;
	public:
		EventControl& operator [] (unsigned int index) { return _controlResultMap[index].second; }

		EventControl& operator [] (std::string&& control_name) {
			for (auto& [key, control] : _controlResultMap) {
				if (key == control_name)
					return control;
			}
			//Intentional bad reference. I'm sleepy fuck you.
			throw std::bad_cast();
		}


		//Since only a portion of control is a flag, this is primarily made for that, and as such is better suited a bool check
		inline friend bool operator & (GroupEventControl& a_lhs, EventControl a_rhs)
		{
			for (auto& [key, control] : a_lhs._controlResultMap) {
				if ((a_rhs & control) != static_cast<EventControl>(0))
					return true;
			}

			return false;
		}
		inline friend bool operator & (EventControl a_lhs, GroupEventControl& a_rhs) { return a_rhs & a_lhs; }

		inline friend bool operator==(const GroupEventControl& a_lhs, const EventControl& a_rhs)
		{
			//For this, if it's asking if it's equal to this value, it only look for an instance of, so be careful on that.
			//This is mostly useful (and roughly only useful) when checking for failure, particularly a form of absolute failure like GroupstopAll
			for (auto& [key, control] : a_lhs._controlResultMap) {
				if (a_rhs == control)
					return true;
			}

			return false;
		}



		inline friend bool operator==(const EventControl& a_lhs, const GroupEventControl& a_rhs) { return a_rhs == a_lhs; }

		inline friend bool operator!=(const GroupEventControl& a_lhs, const EventControl& a_rhs) { return !(a_lhs == a_rhs); }

		inline friend bool operator!=(const EventControl& a_lhs, const GroupEventControl& a_rhs) { return !(a_rhs == a_lhs); }

		GroupEventControl() = default;

		//Redesign to make this more useful, it can have multiple main names, and the loops for however many main names it has.
		GroupEventControl(std::string& main_name, std::vector<std::string>& sub_names, EventControl starting_value = EventControl::kContinue)
		{
			std::size_t size = sub_names.size();
			logger::info("group event control size {}", size);

			_controlResultMap.resize(size);

			for (int i = 0; i <= size; i++) {//auto& main_name : main_names) {
				std::string total_name = i != size ?
					std::format("{}_{}", main_name, sub_names[i]) : main_name;

				_controlResultMap.push_back(std::make_pair(total_name, starting_value));
			}
		}

		GroupEventControl(std::vector<std::string>& main_names, std::vector<std::string>& sub_names, EventControl starting_value = EventControl::kContinue)
		{
			auto main_size = main_names.size();
			auto sub_size = sub_names.size();

			std::size_t size = (main_size * sub_size) + main_size;
			logger::info("group event control size {}", size);
			_controlResultMap.resize(size);

			for (int x = 0; x < main_size; x++) {//auto& main_name : main_names) {
				for (int y = 0; y <= sub_size; y++) {//auto& main_name : main_names) {
					std::string total_name = y != sub_size ?
						std::format("{}_{}", main_names[x], sub_names[y]) : main_names[x];

					_controlResultMap.push_back(std::make_pair(total_name, starting_value));
				}
			}
		}
	};
	


	class EventArgument
	{
	private:
		bool _locked = false;
		//Actually, this object probably won't be needed, as I can just have a pointer to the event param base, and have the function calling
		// the entire mess be the thing that owns the tuple, and owns the EventParameter
		EventParamPointer_OLD old_params;

		//Doesn't even need this either.
		EventParamBase* params{};
		//There are however things I would like, primarily, something to check if the values are dirty or not.
		// Something the old one was able to do. Perhaps something along the lines of only if the variables
		// has a reference type.
		//To get if the values are the same we're gonna have to do the loop deloops, luckily we won't really want to do index of type stuff,
		// we just want to adjust a vector of bools to say which once has been made dirty or not.
		//RefWrappers could hold onto their original values as well, and they could be the thing that controls whether it's seen as dirty or not.
		//An idea is that the tuple when set to it's own address it will confirm whether it's individual values have changed.
		// Or perhaps I'll have a helper class that I can pore RefWrapper into, that sets a value on the wrappers.

		EventHandler* _sender{};

		std::string _name{};

	public:

		EventControl control = EventControl::kContinue;//maybe make this the first property if I can.

		template <class Type>
		inline Type* GetSender()
		{
			if constexpr (std::derived_from<Type, EventHandler>)
			{
				Type* return_obj = dynamic_cast<Type*>(_sender);

				if (return_obj)
					return return_obj;
			}
			
			throw EventTypeArgException();
		}

		template <class Type>
		inline Type* GetSender(Type*& return_sender)
		{
			if constexpr (std::derived_from<Type, EventHandler>)
			{
				return_sender = dynamic_cast<Type*>(_sender);

				if (return_sender)
					return;
			}

			throw EventTypeArgException();
		}

		//I would one take like to be rid of making the tuples like this, perhaps I can make a function I can use
		//Even if I can't get rid of the above, I would like to get rid of the boiler place code
		// surrounding how many finish calls there are.
		template <class This, class ParamTuple, class FirstArg, class... Args> requires(std::is_same_v<std::remove_reference_t<FirstArg>, EventArgument>)
			void FinishCall(This* a_this, void(This::* TestFunction)(FirstArg, Args...), ParamTuple& params)
		{
			static_assert(std::is_same<FirstArg, EventArgument&>::value, "The first optional value must be an EventArgument.");


			if (_locked) {
				using LockedTuple = std::tuple<This*, RefWrapper<EventArgument>, std::remove_reference_t<remove_ref_wrapper_t<Args>>...>;

				LockedTuple locked_params = std::tuple_cat(std::make_tuple(a_this, RefWrapper(*this)), params);
				std::apply(TestFunction, locked_params);
			}
			else {
				using RefTuple = std::tuple<This*, RefWrapper<EventArgument>, RefWrapper<std::remove_reference_t<remove_ref_wrapper_t<Args>>>...>;

				RefTuple ref_params = std::tuple_cat(std::make_tuple(a_this, RefWrapper(*this)), params);
				std::apply(TestFunction, ref_params);
			}
		}

		template <class This, class ParamTuple, class... Args>
		void FinishCall(This* a_this, void(This::* TestFunction)(Args...), ParamTuple& params)
		{
			//Non-Optional Version

			if (_locked) {
				using LockedTuple = std::tuple<This*, std::remove_reference_t<remove_ref_wrapper_t<Args>>...>;

				LockedTuple locked_params = std::tuple_cat(std::make_tuple(a_this), params);
				std::apply(TestFunction, locked_params);
			}
			else {
				using RefTuple = std::tuple<This*, RefWrapper<std::remove_reference_t<remove_ref_wrapper_t<Args>>>...>;

				RefTuple ref_params = std::tuple_cat(std::make_tuple(a_this), params);
				std::apply(TestFunction, ref_params);
			}
		}

		template <class ParamTuple, class FirstArg, class... Args> requires(std::is_same_v<std::remove_reference_t<FirstArg>, EventArgument>)
			void FinishCall(void(*TestFunction)(FirstArg, Args...), ParamTuple& params)
		{
			static_assert(std::is_same<FirstArg, EventArgument&>::value, "The first optional value must be an EventArgument.");


			if (_locked) {
				using LockedTuple = std::tuple<RefWrapper<EventArgument>, std::remove_reference_t<remove_ref_wrapper_t<Args>>...>;

				LockedTuple locked_params = std::tuple_cat(std::make_tuple(RefWrapper(*this)), params);
				std::apply(TestFunction, locked_params);
			}
			else {
				using RefTuple = std::tuple<RefWrapper<EventArgument>, RefWrapper<std::remove_reference_t<remove_ref_wrapper_t<Args>>>...>;

				RefTuple ref_params = std::tuple_cat(std::make_tuple(RefWrapper(*this)), params);
				std::apply(TestFunction, ref_params);
			}
		}


		template <class ParamTuple, class... Args>
		void FinishCall(void(*TestFunction)(Args...), ParamTuple& params)
		{
			//Non-Optional Version

			if (_locked) {
				using LockedTuple = std::tuple<std::remove_reference_t<remove_ref_wrapper_t<Args>>...>;

				LockedTuple locked_params = { params };
				std::apply(TestFunction, locked_params);
			}
			else {
				std::apply(TestFunction, params);
			}
		}





		std::string GetName() { return _name; }

		EventArgument(std::string name, EventHandler* sender) : _name(name), _sender(sender) {}
		EventArgument(std::string&& name, EventHandler* sender) : _name(name), _sender(sender) {}
		//{_name = name;}
		//EventArgument(std::string&& name){_name = name;}
	};



	class EventMember;

	using Function = void();

	struct CallPackage
	{
		virtual void Call(EventArgument& argument, EventParamBase& params) = 0;

		virtual bool IsFunction(void* function) = 0;
		virtual bool IsMember(EventMember* function) { return false; }

		virtual ~CallPackage()
		{
			logger::info("dead package\n");
		}
	};



	using CallPackPointer = std::unique_ptr<CallPackage>;


	//This is used for call packages that are completely static in nature, and as such have no need to know what it's going to be called on.
	template<class... Args>
	struct StaticCallPackage final : public CallPackage
	{
	private:
		using StaticDelegate = void(*)(Args...);


		StaticDelegate caller_function;



	public:

		StaticCallPackage(StaticDelegate func)
		{
			caller_function = func;
		}

		void Call(EventArgument& argument, EventParamBase& params) override
		{
			if constexpr (sizeof...(Args) == 0)
			{
				logger::info("static: zero arg call type");
				caller_function();
			}
			//Should this remove reference so I can send an error if it's not referenced?
			else if constexpr (sizeof...(Args) == 1 && std::is_same_v<std::tuple_element<0, std::tuple<Args...>>::type, EventArgument&>)
			{
				logger::info("static: event arg call type");
				std::invoke(caller_function, argument);
			}
			else
			{
				auto& tuple = params.GetParameters<Args...>();
				argument.FinishCall(caller_function, tuple);
			}
		}

		bool IsFunction(void* function) override { return static_cast<uint64_t>(function) == static_cast<uint64_t>(caller_function); }


		~StaticCallPackage() override
		{
			logger::info("dead static\n");
		}
	};





	//This is used for call packages that are completely static in nature, and as such have no need to know what it's going to be called on.
	template<class This, class... Args>
	struct MemberCallPackage : public CallPackage
	{
	private:
		using MemberDelegate = void(This::*)(Args...);


		This* this_object;

		MemberDelegate caller_function;

		static inline bool CompareMemberFunction(Function* a, Function* b)
		{
			return true;
		}
	public:

		MemberCallPackage(This* _this, MemberDelegate func)
		{
			this_object = _this;
			caller_function = func;
		}
		void Call(EventArgument& argument, EventParamBase& params) override
		{
			if constexpr (sizeof...(Args) == 0)
			{
				logger::info("membered: zero arg call type");
				std::invoke(caller_function, this_object);
			}
			//Should this remove reference so I can send an error if it's not referenced?
			else if constexpr (sizeof...(Args) == 1 && std::is_same_v<std::tuple_element<0, std::tuple<Args...>>::type, EventArgument&>)
			{
				logger::info("membered: event arg call type");
				std::invoke(caller_function, this_object, argument);
			}
			else
			{
				auto& tuple = params.GetParameters<Args...>();
				argument.FinishCall(this_object, caller_function, tuple);
			}
		}

		bool IsFunction(void* function) override 
		{
			//auto fptr = &f; 
			void* a = reinterpret_cast<void*&>(caller_function);
			return a == function;
			//return CompareMemberFunction(function, (Function*)caller_function); 
		}

		bool IsMember(EventMember* member) { This* dynamic_member = dynamic_cast<This*>(member); return this_object == dynamic_member; }


		~MemberCallPackage() override
		{
			logger::info("dead dynamic2\n");
		}
	};





	//To do
	// Change the parameters to be functional based on index of acceptance. With the condition of
	// static asserting that each




	//Primary control function to handle the dealing of events. Generally to be put on the 
	// combat actor (or combat data as it's so named), but also will be marked by a global object,
	// that recieves events about actors in combat, denoting the need for proper signage, notification, 
	// recording, etc.
	class EventNode
	{
	private:
		//If I would like to prevent these from being having multiple of the same function.
		// If I never need to remove stuff from them, I will make it so the functions will be associated via IDs.
		//std::vector<std::function<EventFunction>> _sinks;
		std::vector<CallPackPointer> _sinks;

		//Would like additional settings for this, like never send to global, this is important for stuff like personal update
		// functions that don't use these. Though for not, global is void so.
	public:
		void AddEvent(CallPackage* event_sink)
		{
			CallPackPointer sink_ptr = CallPackPointer(event_sink);

			_sinks.push_back(std::move(sink_ptr));
			//share function instead?
		}


		
		bool RemoveFunction(void* function)
		{
			for (std::vector<CallPackPointer>::iterator sink = _sinks.begin(); sink != _sinks.end(); ) {
				if ((*sink)->IsFunction(function) == true) {
					sink = _sinks.erase(sink);
					return true;
				}
				else {
					++sink;
				}
			}

			//for (auto& sink : _sinks) {if (sink->IsFunction(function) == true) {_sinks.erase(sink);	return true; } }
			return false;
		}


		
		bool RemoveMember(EventMember* member)
		{
			for (std::vector<CallPackPointer>::iterator sink = _sinks.begin(); sink != _sinks.end(); ) {
				if ((*sink)->IsMember(member) == true) {
					sink = _sinks.erase(sink);
					return true;
				}
				else {
					++sink;
				}
			}
			//for (auto& sink : _sinks) { if (sink->IsMember(member) == true) { _sinks.erase(sink); return true; } }

			return false;
		}



		//I'm thinking uses of this should have a static assert to their expected parameters so I don't mess
		// this up on accident.

		//THUNKOR
		//An idea for this is the following. Make this prepare to handle exceptions, but in particular to
		// handle my own. See, instead of doing the whole true false thing,
		// an event sumbitted will just throw an exception when the object doesn't have the right parameterrs,
		// an exception that will bring us back here, we intercept, and keep moving on.
		// prevents the function from running, and doesn't need if getValue == true over and over.
		void SendEvent(EventArgument& arg, EventParamBase& params, bool is_global = false) const
		{
			EventControl& control = arg.control;

			for (const auto& event_sink : _sinks)
			{
				switch (control)
				{

				case EventControl::kContinueGlobal:
					if (!is_global)
						goto exit;

				case EventControl::kContinueLocal:
					if (is_global)
						goto exit;

				case EventControl::kStop:
					exit:
					return;

				}

				//if (control == EventControl::kStop || (is_global && control == EventControl::kContinueLocal) == true) {return;}

				try {
					event_sink->Call(arg, params);
				}
				catch (EventTypeArgException exp) {
					logger::error("place holder args exception message, re make please.");
					//Report thing.
				}

			}
		}

	};

	class IEventHandler {};//this is an interface object, should do nothing. Merely exists.


	class EventHandler : public IEventHandler
	{
	private:
		template <class... Args>
		using StaticDelegate = void(*)(Args...);

		template <class This, class... Args>
		using MemberDelegate = void(This::*)(Args...);

		//Change to a vector if you can? Or unordered map.
		//Also, turn these back into string hashes. there's some overhead on strings I don't really need.
		std::map<std::string, EventNode> _eventMap;
		//std::map<StringHash, EventNode> _eventMap;

	private:

		//public until I can make it private without the use of a vtable for simple event handler.
		//Make this function a friend to the base of simplehandler, and make simple handler send this function off.
		EventControl Internal_SendEvent(std::string& event_name, EventInitFlags flags, EventArgument& argument, EventParamBase& event_params)
		{
			//This function will make no attempt to null check anything, that's to be handled by the stuff that actually made the event params
			// and argument.

			auto& source = _eventMap[event_name];

			//auto globalHandler = GetGlobalHandler();

			bool is_global = IsGlobalHandler();

			source.SendEvent(argument, event_params, is_global);

			//return EventControl(0);



			switch (argument.control)
			{
			default:
				return argument.control;

			case EventControl::kContinueGlobal:
			case EventControl::kContinue:
				if (is_global)
					return argument.control;
				else
					break;
			}

			EventHandler* global_handler = GlobalSingleton();

			if (!global_handler || global_handler->_eventMap.contains(event_name) == false)
				return argument.control;

			//You'll want to do this internally, so you don't have to resend the parameters and junk.
			global_handler->Internal_SendEvent(event_name, flags, argument, event_params);


			return argument.control;
		}

		friend class SimpleEventHandlerBase;

	public:

		static EventHandler* GlobalSingleton()
		{
			static EventHandler global_handler{};
			return &global_handler;
		}

		inline bool IsGlobalHandler() { return this == GlobalSingleton(); }

		inline bool ContainsEvent(std::string event_name) { return _eventMap.contains(event_name); }


		//the function for loading parameters is something that I think I want to handle in sequence instead
		// of handling them like a single object with a name. Both methods should basically be able to suffice.
		// though, I should decide on one.
		//Additionally, global will likely need different parameters than the other one.
		//EventHandler* GetGlobalHandler()
		//{
		//	static EventHandler singleton;
		//	return &singleton;
		//}


		//The add member event might not be needed, I could just move it to this. Would be less space.
		void AddEvent(std::string event_name, CallPackage* new_sink)
		{
			//auto _hash = hash(eventName.c_str(), eventName.size());
			//auto source = _eventMap[_hash];
			auto& source = _eventMap[event_name];
			source.AddEvent(new_sink);
		}

		template<class... Args>
		void AddEvent(std::string event_name, StaticDelegate<Args...> new_sink)
		{
			auto static_call_pack = new StaticCallPackage(new_sink);

			AddEvent(event_name, static_call_pack);

		}



		template<class This, class... Args>
		void RemoveEvent(std::string event_name, MemberDelegate<This, Args...> sink)
		{
			void* void_sink = reinterpret_cast<void*&>(sink);

			if (_eventMap.contains(event_name) == false)
				return;

			_eventMap[event_name].RemoveFunction(void_sink);
		}


		template<class This, class... Args>
		void RemoveEvent(MemberDelegate<This, Args...> sink)
		{
			void* void_sink = reinterpret_cast<void*&>(sink);

			for (auto& [key, node] : _eventMap) {
				node.RemoveFunction(void_sink);
			}
		}



		template<class... Args>
		void RemoveEvent(std::string event_name, StaticDelegate<Args...> sink)
		{
			if (_eventMap.contains(event_name) == false)
				return;

			_eventMap[event_name].RemoveFunction(sink);
		}


		template<class... Args>
		void RemoveEvent(StaticDelegate<Args...> sink)
		{
			for (auto& [key, node] : _eventMap) {
				node.RemoveFunction(sink);
			}
		}


		
		//The member types might accept a little more than just this. That way I can transfer it when I need to.
		// also, at some time, I may expand this to deal with non-EventMember event functions. But this works for now.
		void RemoveEvent(std::string event_name, EventMember* member)
		{
			if (_eventMap.contains(event_name) == false)
				return;

			_eventMap[event_name].RemoveMember(member);

		}

		void RemoveEvent(EventMember* member)
		{

			for (auto& [key, node] : _eventMap) {
				node.RemoveMember(member);
			}

		}




		//I some how can't seem to make this shit constant, so that might be a problem for things that demand the pointer remain so.
		template<EventInitFlags flags = EventInitFlags::None, class... Args>
		EventControl SendEvent(std::string eventName, Args&&... args)
		{
			//The const stuff doesn't work until this proves to be false.
			//static_assert(all_true<!std::is_const_v<Args>...>::value, "fgfnugbug");

			if (_eventMap.contains(eventName) == false)
				return EventControl::kNoEvent;

			EventControl result;

			if constexpr (sizeof...(Args) != 0) {
				EventArgument argument(eventName, this);
				//return EventControl::kContinue;
				auto event_params = CreateEventParameter(args...);
				
				result = Internal_SendEvent(eventName, flags, argument, event_params);
			}
			else {

				EventArgument argument(eventName, this);

				auto event_params = ZeroArgEventParameter();

				result = Internal_SendEvent(eventName, flags, argument, event_params);
			}

			return result;
		}


		//While this does lock, it won't trip the actual flag of locked. It will just make new arguments for the object to dump into.
		// A smart decision might be to create an internal SendEvent that has a first parameter that takes all sorts of unique traits
		// I can initialize the components with.
		template<EventInitFlags flags = EventInitFlags::None, class... Args>
		inline EventControl SendLockedEvent(std::string eventName, Args... args)
		{
			//We ain't ready for this one just yet.
			//return SendEvent<EventInitFlags::Locked>

			if constexpr (sizeof...(Args) != 0){
				return SendEvent(eventName, args...);
			}
			else {
				return SendEvent(eventName);
			}
		}
		
		//Function is not operational.
		template<EventInitFlags Flags = EventInitFlags::None, class... Args>
		inline EventControl SendNoRefEvent(std::string event_name, DisallowReference<Args>... args) { return SendEvent<Flags>(event_name, args...); }

#pragma region Grouped Events
		//Grouped event seemed cool at first, but considering that 1 type will only really have 1 function type, it's rough to qualify the existence of this.
		//Unless, I can think of a good method in which the names for which event changes.

		//Event Control predicate reads the string for which an event is happening, and can submit a different control. This is useful if you'd 
		// like a stop in one of them to mean stop in all of them, situationally
		//EventControl(*control_predicate)(std::string&, EventControl&) = nullptr
		//However, this idea is canned in leu of just using your fucking brain and having foresight equal to being blind in an eye
		//*
		using ControlPredicate = void(std::string&, EventControl&);

		void Internal_SendGroupedEvent(std::string* main_name, GroupEventControl& group_control, EventParamBase& event_params, EventInitFlags flags, ControlPredicate* Control_Function)
		{
			auto HandleSend = [this, flags, Control_Function](std::string& event_name, EventControl& last_control, EventParamBase& event_params) -> EventControl
			{
				if (_eventMap.contains(event_name) == false)
					return EventControl::kNoEvent;


				EventArgument argument(event_name, this);

				last_control = Internal_SendEvent(event_name, flags, argument, event_params);

				if (Control_Function)
					Control_Function(event_name, last_control);

				return last_control;
			};

			EventControl last_control = EventControl::kContinue;

			EventControl skip_main = EventControl::kContinue;

			int i = 1;

			for (auto& [event_name, event_control] : group_control.GetMap())
			{
				
				if ((last_control & EventControl::kGroupStopMain) != EventControl::kContinue) {
					event_control = last_control;
					continue;
				}
				else if (*main_name == event_name) {
					if ((skip_main & EventControl::kGroupStopGeneral) != EventControl::kContinue) {
						event_control = skip_main;
					}
					else {
						event_control = HandleSend(event_name, last_control, event_params);
					}					

					if (last_control != EventControl::kGroupStopAll) {
						//If last control was stop all we skip resetting.
						last_control = EventControl::kContinue;
						skip_main = EventControl::kContinue;
						main_name = &main_name[i];
						i++;
					}
					
					continue;
				}
				else if ((last_control & EventControl::kGroupStopSpecific) != EventControl::kContinue) {
					event_control = last_control;
					continue;
				}

				event_control = HandleSend(event_name, last_control, event_params);

				if ((last_control & EventControl::kGroupStopGeneral) != EventControl::kContinue) {
					skip_main = last_control;
				}
			}
		}


		//This may be the piviot function, so I can have single sub names too.
		template<EventInitFlags _Flags = EventInitFlags::None, ControlPredicate* Control_Function = nullptr, class... Args>
		inline GroupEventControl SendGroupedEvent(std::vector<std::string> event_names, std::vector<std::string> specific_names, Args&&... args)
		{
			//return GroupEventControl();

			GroupEventControl group_control(event_names, specific_names);

			if constexpr (sizeof...(Args) != 0)
			{
				auto event_params = CreateEventParameter(args...);
				Internal_SendGroupedEvent(event_names.data(), group_control, event_params, _Flags, Control_Function);
			}
			else
			{
				auto event_params = ZeroArgEventParameter();
				Internal_SendGroupedEvent(event_names.data(), group_control, event_params, _Flags, Control_Function);
			}


			return group_control;

		}

		template<EventInitFlags _Flags = EventInitFlags::None, ControlPredicate* Control_Function = nullptr, class... Args>
		inline GroupEventControl SendGroupedEvent(std::string event_name, std::vector<std::string> specific_names, Args&&... args)
		{
			GroupEventControl group_control(event_name, specific_names);
			
			if constexpr (sizeof...(Args) != 0)
			{
				auto event_params = CreateEventParameter(args...);
				Internal_SendGroupedEvent(&event_name, group_control, event_params, _Flags, Control_Function);
			}
			else
			{
				auto event_params = ZeroArgEventParameter();
				Internal_SendGroupedEvent(&event_name, group_control, event_params, _Flags, Control_Function);
			}
			

			return group_control;
		}

		


		

		/*
		template<ControlPredicate* Control_Function, class... Args>
		inline GroupEventControl SendGroupedEvent(std::string name, std::vector<std::string> specific_names, Args&&... args)
		{
			return SendGroupedEvent<EventInitFlags::None, Control_Function>(name, specific_names, args);
		}
		//*/
		template<ControlPredicate* Control_Function, class... Args>
		inline GroupEventControl SendGroupedEvent(std::vector<std::string> name, std::vector<std::string> specific_names, Args&&... args)
		{
			return SendGroupedEvent<EventInitFlags::None, Control_Function>(name, specific_names, args...);
		}
#pragma endregion
		
		
	};

	//The idea behind this is it's supposed to intercept events that are supposed to be fired and check for them here. One can make subscriptions to Local
	// events which prevent them from ever going global, or fire global events which don't do local at all (it's not a flag, more a function).
	//class GlobalEventHandler {};


	struct SimpleEventHandlerBase : public IEventHandler
	{
	protected:
		//Used as a dump object when some package isnt found. Doing so a new one isn't made for every simple
		static inline CallPackage* END = nullptr;

		static bool IsEnd(CallPackage*& test) { return &END == &test; }

		static EventControl SendGlobalEvent(std::string& event_name, EventInitFlags flags, EventArgument& argument, EventParamBase& event_params)
		{
			EventHandler* global_handler = EventHandler::GlobalSingleton();

			if (!global_handler || global_handler->ContainsEvent(event_name) == false)
				return argument.control;

			return global_handler->Internal_SendEvent(event_name, flags, argument, event_params);
		}
	};


	//A simple class that exists to accepted not only a fixed amount of events, but fixed names as well.
	//Because of this, it has no need for nodes, vectors, and only maintains call packages. It also functions as the object that sets up member events.
	template<StringLiteral... Event_Names> requires(sizeof...(Event_Names) > 0)
	class SimpleEventHandler : public SimpleEventHandlerBase
	{
	private:
		template <class... Args>
		using StaticDelegate = void(*)(Args...);

		template <class This, class... Args>
		using MemberDelegate = void(This::*)(Args...);


	public:
		static constexpr size_t eventSize = sizeof...(Event_Names);
		static constexpr auto events = std::tuple<decltype(Event_Names)...>{ Event_Names... };

		template <size_t I> static constexpr auto eventName = std::get<I>(events);
		
		static constexpr StringLiteral FirstName = eventName<0>;

		static inline const std::array<string_hash, eventSize> eventKey{ hash_string<Event_Names>... };//Could just hold the names. Nvm, can't be uniform.
		std::array<CallPackage*, eventSize> eventCallbacks{};//Doesn't need unique callback pointers for this.

		~SimpleEventHandler()
		{
			for (int i = 0; i < eventSize; i++)
			{
				auto& callback = eventCallbacks[i];

				if (callback)
					delete callback;
			}
		}

		CallPackage*& GetEvent(std::string& event_name)
		{
			auto name_hash = hash(event_name);

			for (int i = 0; i < eventSize; i++)
			{
				if (name_hash == eventKey[i])
					return eventCallbacks[i];
			}

			return SimpleEventHandlerBase::END;
		}


		//*
		EventControl Internal_SendEvent(std::string& event_name, CallPackage* node, EventInitFlags flags, EventArgument& argument, EventParamBase& event_params)
		{
			try {
				node->Call(argument, event_params);
			}
			catch (EventTypeArgException exp) {
				logger::error("place holder args exception message, re make please.");
				//Report thing.
			}
			
			switch (argument.control)
			{
			default:
				return argument.control;

			case EventControl::kContinueGlobal:
			case EventControl::kContinue:
				return SendGlobalEvent(event_name, flags, argument, event_params);
			}
		}

		//The add member event might not be needed, I could just move it to this. Would be less space.
		bool AddEvent(std::string event_name, CallPackage* new_sink, bool replace)
		{
			//auto _hash = hash(eventName.c_str(), eventName.size());
			//auto source = _eventMap[_hash];
			auto& event_slot = GetEvent(event_name);
			
			if (IsEnd(event_slot) == true)
				return false;

			if (event_slot)
				if (!replace) {
					logger::warn("event slot {} is filled.", event_name);
					return false;
				}
				else {
					delete event_slot;
					event_slot = nullptr;
				}


			event_slot = new_sink;

			return true;
		}


	public:

		//the function for loading parameters is something that I think I want to handle in sequence instead
		// of handling them like a single object with a name. Both methods should basically be able to suffice.
		// though, I should decide on one.
		//Additionally, global will likely need different parameters than the other one.
		//EventHandler* GetGlobalHandler()
		//{
		//	static EventHandler singleton;
		//	return &singleton;
		//}


	
		template<class... Args>
		bool AddEvent(std::string event_name, StaticDelegate<Args...> new_sink, bool replace)
		{
			auto static_call_pack = new StaticCallPackage(new_sink);

			if (AddEvent(event_name, static_call_pack, replace) == false) {
				delete static_call_pack;
				return false;
			}

			return true;

		}

		template<class This, class... Args>
		bool AddEvent(std::string event_name, This* a_this, MemberDelegate<This, Args...> new_sink, bool replace)
		{
			if (!a_this || !new_sink)
				return false;
			//I want this to fucking cast, but I think I don't really want to do this like dynamic cast.
			auto call_pack = new MemberCallPackage(a_this, new_sink);

			if (AddEvent(event_name, call_pack, replace) == false) {
				delete call_pack;
				return false;
			}

			return true;
		}

		//I some how can't seem to make this shit constant, so that might be a problem for things that demand the pointer remain so.
		template<EventInitFlags Flags = EventInitFlags::None, class... Args>
		EventControl SendEvent(std::string event_name, Args&&... args)
		{
			//The const stuff doesn't work until this proves to be false.
			//static_assert(all_true<!std::is_const_v<Args>...>::value, "fgfnugbug");
			auto& event_slot = GetEvent(event_name);

			if (!event_slot)
				return EventControl::kNoEvent;

			EventControl result;

			if constexpr (sizeof...(Args) != 0) {
				EventArgument argument(event_name, this);
				//return EventControl::kContinue;
				auto event_params = CreateEventParameter(args...);

				result = Internal_SendEvent(event_name, event_slot, Flags, argument, event_params);
			}
			else {

				EventArgument argument(event_name, this);

				auto event_params = ZeroArgEventParameter();

				result = Internal_SendEvent(event_name, event_slot, Flags, argument, event_params);
			}

			return result;
		}



		/*
		template<EventInitFlags Flags = EventInitFlags::None, class... Args>
		inline EventControl SendLockedEvent(std::string event_name, Args... args)
		{
			//We ain't ready for this one just yet.
			//return SendEvent<EventInitFlags::Locked>
			static constexpr auto new_flags = Flags | EventInitFlags::Locked;
			if constexpr (sizeof...(Args) != 0) {
				return SendEvent<new_flags>(event_name, args...);
			}
			else {
				return SendEvent<new_flags>(event_name);
			}
		}

		template<EventInitFlags Flags = EventInitFlags::None, class... Args>
		inline EventControl SendNoRefEvent(std::string event_name, DisallowReference<Args, true>... args) { return SendEvent<Flags>(event_name, args); }
		//*/
	};


	class EventMember
	{
	protected:
		virtual EventHandler* GetHandler() = 0;


	public:
		//#define CreateMemberEvent(a_event_name, a_new_sink) AddMemberEvent(a_event_name, this, a_new_sink)


		//At a later point make this a deconstructor that asks, should it remove everything from the event handler. Main reason, to save memory on mostly 
		// unloaded actors I'm thinking of making the distant behaviour system.


		//Create a macro for this so I don't have to do the whole this type dealy.
		template<class This, class... Args>
		bool CreateMemberEvent(std::string event_name, void(This::* new_sink)(Args...))
		{
			EventHandler* handler = GetHandler();

			if (!handler)
				return false;
			//I want this to fucking cast, but I think I don't really want to do this like dynamic cast.
			auto call_pack = new MemberCallPackage(static_cast<This*>(this), new_sink);

			handler->AddEvent(event_name, call_pack);

			return true;
		}
		
		void RemoveAllEvents()
		{
			EventHandler* handler = GetHandler();

			if (!handler)
				return;

			handler->RemoveEvent(this);
		}

		void RemoveEvent(std::string event_name)
		{
			EventHandler* handler = GetHandler();

			if (!handler)
				return;

			handler->RemoveEvent(event_name, this);
		}
		
		//I will not implement the deconstructor here for deleting events, it wont be needed here. It should specifically be in distant
	};

	
	//template<class... Args>
	//void SendEvent(CallPackPointer_OLD* call_package, unsigned int length, Args... args){ }

#pragma endregion






#ifdef enable_legacy_event_token183183
	namespace LegacyEventSystem
	{
		//Event control, as the name on the tin
		enum class EventControl
		{
			kContinue,			//Continue the loop. Simple.
			kContinueLocal,		//Disallow the event to reach global.
			kStop,				//Stop the event outright.
			kError				//An error was encountered, event didn't resolve properly
		};


		using EventFunction = void(EventObject&, EventControl&);


		//To do
		// Change the parameters to be functional based on index of acceptance. With the condition of
		// static asserting that each




		//Primary control function to handle the dealing of events. Generally to be put on the 
		// combat actor (or combat data as it's so named), but also will be marked by a global object,
		// that recieves events about actors in combat, denoting the need for proper signage, notification, 
		// recording, etc.
		class EventNode
		{
		private:
			//If I would like to prevent these from being having multiple of the same function.
			// If I never need to remove stuff from them, I will make it so the functions will be associated via IDs.
			std::vector<std::function<EventFunction>> _sinks;


			//Would like additional settings for this, like never send to global, this is important for stuff like personal update
			// functions that don't use these. Though for not, global is void so.
		public:
			void AddEvent(std::function<EventFunction> eventSink)
			{
				_sinks.push_back(eventSink);

				//Utility::DebugNotification(std::format("{}", _sinks.size()).c_str());

			}

			//I'm thinking uses of this should have a static assert to their expected parameters so I don't mess
			// this up on accident.

			//THUNKOR
			//An idea for this is the following. Make this prepare to handle exceptions, but in particular to
			// handle my own. See, instead of doing the whole true false thing,
			// an event sumbitted will just throw an exception when the object doesn't have the right parameterrs,
			// an exception that will bring us back here, we intercept, and keep moving on.
			// prevents the function from running, and doesn't need if getValue == true over and over.
			EventControl SendEvent(EventObject& arguments, EventControl& control, bool isGlobal = false) const
			{

				for (const auto& eventSink : _sinks)
				{
					if (control == EventControl::kStop || (isGlobal == true && control == EventControl::kContinueLocal) == true) {
						goto out_label;
					}

					eventSink(arguments, control);
				}

			out_label:
				return control;
			}

		};


		class EventHandler
		{
		private:
			//using StringHash = std::uint32_t;
			std::map<std::string, EventNode> _eventMap;
			//std::map<StringHash, EventNode> _eventMap;


		public:

			//the function for loading parameters is something that I think I want to handle in sequence instead
			// of handling them like a single object with a name. Both methods should basically be able to suffice.
			// though, I should decide on one.
			//Additionally, global will likely need different parameters than the other one.
			//EventHandler* GetGlobalHandler()
			//{
			//	static EventHandler singleton;
			//	return &singleton;
			//}



			void AddEvent(std::string eventName, std::function<EventFunction> eventSink)
			{
				//auto _hash = hash(eventName.c_str(), eventName.size());
				//auto source = _eventMap[_hash];
				auto& source = _eventMap[eventName];
				source.AddEvent(eventSink);
			}

			EventControl SendEvent(std::string eventName, EventObject& arguments)
			{
				//auto _hash = hash(eventName.c_str(), eventName.size());
				//auto& source = _eventMap[_hash];
				if (_eventMap.contains(eventName) == false)
					return EventControl::kError;

				arguments.SetEventName(eventName);

				auto& source = _eventMap[eventName];


				EventControl control = EventControl::kContinue;


				//auto globalHandler = GetGlobalHandler();

				//bool isGlobal = globalHandler == this;

				return source.SendEvent(arguments, control, false);


				//if (control != EventControl::kContinue || isGlobal)
				//	return;
				//Global needs to be handled by a different class, as to also include a caller source.
				// Actually, should likely be including that anyways.
				//globalHandler->SendEvent(eventName, arguments);
			}

			//Easy short hand for an empty one.
			EventControl SendEvent(std::string eventName)
			{
				EventObject empty;
				return SendEvent(eventName, empty);
			}



			//Make an inlined template function for send event that packs all the parameters itself.
		};


		//Event handler, handles the EventNodes.
		// EventNodes, associated with an event, calls functions in sequence.
		// event reciever

		//A convience class that combat behaviour will inherent from. it goes through the process of making lambdas
		// out of member functions to give to an event source, as well as encapsulating the instance for the function to call.
		class EventMember
		{
		protected:
			virtual EventHandler* GetHandler() = 0;


		public:
			//I would like an overload that does this via just the node


			template<class This>
			bool CreateMemberEvent(std::string eventName, EventHandler* handler, void(This::* function)(EventObject&, EventControl&))
			{
				if (!handler)
					return false;

				//using std::placeholders::_1;
				//source.AddEventSink(std::bind(function, this, _1));
				//The this could likely be a void pointer, but that would make it unsafer.

				auto evt = [this, function](EventObject& eventParams, EventControl& control)
				{
					//std::invoke(function, reinterpret_cast<This*>(this), eventParams, control);
					std::invoke(function, dynamic_cast<This*>(this), eventParams, control);
					//std::invoke(function, (This*)(this), eventParams, control);
					//function(a_this, eventParams); 
				};

				handler->AddEvent(eventName, evt);

				return true;
			}

			template<class This>
			bool CreateMemberEvent(std::string eventName, void(This::* function)(EventObject&, EventControl&)) { return CreateMemberEvent(eventName, GetHandler(), function); }

			//This just doesn't work on members. Rip.
			//template<class This>
			//bool CreateMemberEvent(std::string eventName, EventHandler* handler, void(&function)(This* a_this, EventObject&, EventControl&)){return false;}
		};
	}
#endif
}