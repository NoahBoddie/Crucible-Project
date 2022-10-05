#pragma once



#pragma region Property_Macro_Definition
//namespace detail{

//A defeating solution, but it works and keeps the look. Although, I'm wondering if it's even worth it at this point.
template<class T>
using pointer = T*;

	template<class ThisType, class CoreType>
	class alias_base;


	template<class ThisType, class CoreType>
	struct property_base
	{
		using This = std::remove_reference<ThisType>::type;

	protected:
		std::int16_t _offset;



		//Make this a ref type later please
		ThisType parent() const
		{
			auto prop_addr = reinterpret_cast<std::uintptr_t>(this);
			auto ptr = reinterpret_cast<This*>(prop_addr - _offset);

			return *ptr;
		}

		friend alias_base<ThisType, CoreType>;
	};


	template<class ThisType, class CoreType>
	struct alias_base
	{
		using Focus = property_base<ThisType, CoreType>;

	protected:

		ThisType parent() const
		{
			auto prop_addr = reinterpret_cast<std::uintptr_t>(this);
			auto ptr = reinterpret_cast<property_base<ThisType, CoreType>*>(sizeof(Focus) + prop_addr);

			if (!ptr)
				throw nullptr;//I'd like to throw a proper exception.

			return ptr->parent();
		}
	};




	template<class ThisType, class CoreType>
	using GetFuncPtr = CoreType(*)(ThisType);

	template<class ThisType, class CoreType>
	using SetFuncPtr = void(*)(ThisType, CoreType&);


	//Nothing special, just cv pastes all the other ones. To get a look at what this looks like, check out the public verison.
	#define CREATE_PROPERTY_TEMPLATE(mc_get_accessor, mc_set_accessor)																		\
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>	\
	struct mc_get_accessor##__##mc_set_accessor##__property_struct : public property_base<ThisType, CoreType>{								\
	private:																																\
		using Self = mc_get_accessor##__##mc_set_accessor##__property_struct<ThisType, CoreType, k_static_get, k_static_set>;				\
		friend std::remove_reference<ThisType>::type;																						\
		inline ThisType parent() const { return __super::parent(); }																		\
	public:																																	\
		mc_get_accessor##__##mc_set_accessor##__property_struct(ThisType _this, CoreType(*_get)(ThisType), void(*_set)(ThisType, CoreType&) = nullptr)\
		{																																	\
			this->_offset = reinterpret_cast<std::uintptr_t>(this) - reinterpret_cast<std::uintptr_t>(&_this);								\
			*k_static_get = _get;																											\
			*k_static_set = _set;																											\
		}																																	\
	mc_get_accessor:																														\
		operator CoreType() const { return (*k_static_get)(parent()); }																		\
		CoreType operator*() { return (*k_static_get)(parent()); }																			\
	mc_set_accessor:																														\
		Self& operator=(CoreType& a_rhs) { (*k_static_set)(parent(), a_rhs); return *this; }												\
		template<std::convertible_to<CoreType> TransType>																					\
		Self& operator=(TransType& a_rhs) { (*k_static_set)(parent(), a_rhs); return *this; }												\
		template<std::convertible_to<CoreType> TransType>																					\
		Self& operator=(TransType&& a_rhs) { (*k_static_set)(parent(), a_rhs); return *this; }												\
	}


	//CREATE_PROPERTY_TEMPLATE(public, public);//manually declaring this one, it's the base of all of these.


	//Last parameter of the above should be the original function.
	//Additionally, this whole tuple thing should be set up, and managed by a helper class
	// It will be hard to access, but since I know the direct args I should be able to compare...some how?
	// I just think this shit is a lot to do, so I want to opt to use the other instead.
	// I think if I store type codes like with the event parameters, maybe this works?
	//This will have to store, the first time its used, and then no more is done to it after.

	//Please make it so ThisType stops being a pointer, I need that a bit more versitile 
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>//should have a bool value to determine if the original get set can be used, and if not it forces get/set
	struct property_struct : public property_base<ThisType, CoreType>
	{
	private:
		using Self = property_struct<ThisType, CoreType, k_static_get, k_static_set>;
		//using This = std::remove_pointer<ThisType>::type;
		using This = std::remove_reference<ThisType>::type;

		//Please replace with friend functions, to prevent corruption of stuff like _offset (the only thing to edit here)
		//This will not permit something to be implicitly casting into something like format, if private.
		friend This;
		//friend ThisType;

		//Giving permission will be fucking hard though.


		std::uint64_t address() { return reinterpret_cast<std::uintptr_t>(this); }

		inline ThisType parent() const { return __super::parent(); }


		//Empty is used so there's always something to stick first, so both Get and Set can start their respectives
		// with ", ...." allowing either to be used in any order.
	public:
		//template<class This>
		//property_struct(This _this, GetFunc<This> added_get, SetFunc<This> added_set)
		//property_struct(This _this, decltype(k_static_get) _get, decltype(k_static_set) _set){	}


		property_struct(ThisType _this, CoreType(*_get)(ThisType), void(*_set)(ThisType, CoreType&) = nullptr)
		{
			auto par_addr = reinterpret_cast<std::uintptr_t>(&_this);
			auto prop_addr = reinterpret_cast<std::uintptr_t>(this);


			this->_offset = prop_addr - par_addr;

			*k_static_get = _get;
			*k_static_set = _set;
		}

	public:

		operator CoreType() const
		{
			return (*k_static_get)(parent());
		}

		CoreType operator*()
		{
			return (*k_static_get)(parent());
		}

		Self& operator=(CoreType& a_rhs)
		{
			(*k_static_set)(parent(), a_rhs);
			return *this;
		}

		//Copy Assignment
		template<std::convertible_to<CoreType> TransType>
		Self& operator=(TransType& a_rhs)
		{
			(*k_static_set)(parent(), a_rhs);
			return *this;
		}

		//Explicit move assignment
		template<std::convertible_to<CoreType> TransType>
		Self& operator=(TransType&& a_rhs)
		{
			(*k_static_set)(parent(), a_rhs);
			return *this;
		}
	};



	//The accessor class for protected properties, protected properties are always paired with a real property. So, instead of having any real significant data of it,
	// it shifts itself a byte, and calls get parent that way
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	struct alias_property_struct :public alias_base<ThisType, CoreType>
	{
		using Self = alias_property_struct<ThisType, CoreType, k_static_get, k_static_set>;
		//using This = std::remove_pointer<ThisType>::type;
		using This = std::remove_reference<ThisType>::type;

		friend This;


		std::uint64_t address() { return reinterpret_cast<std::uintptr_t>(this); }

		inline ThisType parent() const { return __super::parent(); }


	public:

		//While having all the nature of a property, this thing doesn't have a shred of use to it, it is an empty name.
		// And as such, there is nothing to construct.
		alias_property_struct() = default;

		//Attempts to prevent these from running if there's nothing at their locations is no good, they're always gonna be filled, 
		// further more, they get filled dynamically.

		operator CoreType() const
		{
			return (*k_static_get)(parent());
		}

		CoreType operator*()
		{
			return (*k_static_get)(parent());
		}

		Self& operator=(CoreType& a_rhs)
		{
			(*k_static_set)(parent(), a_rhs);
			return *this;
		}

		//Copy Assignment
		template<std::convertible_to<CoreType> TransType>
		Self& operator=(TransType& a_rhs)
		{
			auto value = static_cast<CoreType>(a_rhs);
			(*k_static_set)(parent(), value);
			return *this;
		}

		//Explicit move assignment
		template<std::convertible_to<CoreType> TransType>
		Self& operator=(TransType&& a_rhs)
		{
			auto value = static_cast<CoreType>(a_rhs);
			(*k_static_set)(parent(), value);
			return *this;
		}
	};

	//Make a macro version of EVERY SINGLE ALIAS. Stupidly insane, I'm aware, but hey, it should work.

	#pragma region property_aliases


	//Declare public public
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public__public__property_struct = property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using __property_struct = property_struct<ThisType, CoreType, k_static_get, k_static_set>;//should be set to whatevevr is the default.

	CREATE_PROPERTY_TEMPLATE(public, private);
	CREATE_PROPERTY_TEMPLATE(private, public);
	CREATE_PROPERTY_TEMPLATE(private, private);



	//Declare paired accessor
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public__property_struct = public__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private__property_struct = private__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	//Declare Specifier accessors
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private_get__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public_get__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private_set__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public_set__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	//Declare extra/out of order specifiers
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public_set__private_get__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private_get__public_set__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;


	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private_set__private_get__property_struct = private__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private_get__private_set__property_struct = private__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private_set__public_get__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public_get__private_set__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;


	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public_set__public_get__property_struct = public__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public_get__public_set__property_struct = public__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;


	//Declare Redundancy
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private_get__private_get__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public_get__public_get__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private_set__private_set__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public_set__public_set__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;



	//Declare protected automation
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected__property_struct = public__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;//determined by accessor

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected__protected__property_struct = public__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;//determined by accessor

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public__protected__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected__public__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private__protected__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected__private__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;


	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected_get__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected_set__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;



	//Declare extra/out of order specifiers
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public_set__protected_get__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected_get__public_set__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;


	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected_set__protected_get__property_struct = public__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected_get__protected_set__property_struct = public__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;


	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected_set__public_get__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using public_get__protected_set__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;


	//
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private_set__protected_get__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected_get__private_set__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected_set__private_get__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;
	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using private_get__protected_set__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;
	//


	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected_get__protected_get__property_struct = private__public__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	template<class ThisType, class CoreType, GetFuncPtr<ThisType, CoreType>* k_static_get, SetFuncPtr<ThisType, CoreType>* k_static_set>
	using protected_set__protected_set__property_struct = public__private__property_struct<ThisType, CoreType, k_static_get, k_static_set>;

	#pragma endregion


	#pragma region FOR_EACH_MACRO

	#ifdef original_for_each
	#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
	#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
	#define CONCATENATE2(arg1, arg2)  arg1##arg2

	#define FOR_EACH_1(what, x, ...) what(x)
	#define FOR_EACH_2(what, x, ...)\
		  what(x);\
		  FOR_EACH_1(what,  __VA_ARGS__);
	#define FOR_EACH_3(what, x, ...)\
		  what(x);\
		  FOR_EACH_2(what, __VA_ARGS__);
	#define FOR_EACH_4(what, x, ...)\
		  what(x);\
		  FOR_EACH_3(what,  __VA_ARGS__);
	#define FOR_EACH_5(what, x, ...)\
		  what(x);\
		 FOR_EACH_4(what,  __VA_ARGS__);
	#define FOR_EACH_6(what, x, ...)\
		  what(x);\
		  FOR_EACH_5(what,  __VA_ARGS__);
	#define FOR_EACH_7(what, x, ...)\
		  what(x);\
		  FOR_EACH_6(what,  __VA_ARGS__);
	#define FOR_EACH_8(what, x, ...)\
		  what(x);\
		  FOR_EACH_7(what,  __VA_ARGS__);

	#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
	#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__) 
	#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N 
	#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0

	#define FOR_EACH_(N, what, x, ...) CONCATENATE(FOR_EACH_, N)(what, x, __VA_ARGS__)
	#define FOR_EACH(what, x, ...) FOR_EACH_(FOR_EACH_NARG(x, __VA_ARGS__), what, x, __VA_ARGS__)

	#endif

	#ifdef use_old_revised_for_each

	#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
	#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
	#define CONCATENATE2(arg1, arg2)  arg1##arg2

	#define FOR_EACH_1(what, other, x, ...) what(x, other)
	#define FOR_EACH_2(what, other, x, ...)\
		  FOR_EACH_1(what, what(x, other),  __VA_ARGS__)

	#define FOR_EACH_3(what, other, x, ...)\
		  FOR_EACH_2(what, what(x, other),  __VA_ARGS__)

	#define FOR_EACH_4(what, other, x, ...)\
		  FOR_EACH_3(what, what(x, other),  __VA_ARGS__)

	#define FOR_EACH_5(what, other, x, ...)\
		  FOR_EACH_4(what, what(x, other),  __VA_ARGS__)

	#define FOR_EACH_6(what, other, x, ...)\
		  FOR_EACH_5(what, what(x, other),  __VA_ARGS__)

	#define FOR_EACH_7(what, other, x, ...)\
		  FOR_EACH_6(what, what(x, other),  __VA_ARGS__)

	#define FOR_EACH_8(what, other, x, ...)\
		  FOR_EACH_7(what, what(x, other),  __VA_ARGS__)


	#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
	#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__) 
	#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N 
	#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0, -1

	#define FOR_EACH_(N, what, x, ...) CONCATENATE(FOR_EACH_, N)(what,, x, __VA_ARGS__)
	#define FOR_EACH(what, x, ...) FOR_EACH_(FOR_EACH_NARG(x, __VA_ARGS__), what, x, __VA_ARGS__)

	#endif


	#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
	#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
	#define CONCATENATE2(arg1, arg2)  arg1##arg2


	// Make a FOREACH macro
	//#define FE_0(WHAT)
	//#define FE_1(WHAT, X) WHAT(X) 
	//#define FE_2(WHAT, X, ...) WHAT(X)FE_1(WHAT, __VA_ARGS__)
	//#define FE_3(WHAT, X, ...) WHAT(X)FE_2(WHAT, __VA_ARGS__)
	//#define FE_4(WHAT, X, ...) WHAT(X)FE_3(WHAT, __VA_ARGS__)
	//#define FE_5(WHAT, X, ...) WHAT(X)FE_4(WHAT, __VA_ARGS__)
	//... repeat as needed


		// Make a FOREACH macro
	#define FE_0(WHAT)
	#define FE_1(WHAT, X) WHAT(X) 
	#define FE_2(WHAT, X, ...) CONCATENATE(WHAT(X), FE_1(WHAT, __VA_ARGS__))
	#define FE_3(WHAT, X, ...) CONCATENATE(WHAT(X), FE_2(WHAT, __VA_ARGS__))
	#define FE_4(WHAT, X, ...) CONCATENATE(WHAT(X), FE_3(WHAT, __VA_ARGS__))
	#define FE_5(WHAT, X, ...) CONCATENATE(WHAT(X), FE_4(WHAT, __VA_ARGS__))
	//... repeat as needed

	#define GET_MACRO(_0,_1,_2,_3,_4,_5,NAME,...) NAME 
	#define FOR_EACH(action,...) \
	  GET_MACRO(_0,__VA_ARGS__,FE_5,FE_4,FE_3,FE_2,FE_1,FE_0)(action,__VA_ARGS__)

	#pragma endregion


	#define EMPLACE_ACCESSOR(mc_accessor) mc_accessor##__

	#define __detail_ACCESSOR(...) FOR_EACH(EMPLACE_ACCESSOR, __VA_ARGS__)

	#define DEFINE_CLASS_NAME_AS(mc_id, mc_value_type, mc_accessor)													\
	private:																										\
		auto static (*mc_id##class_type)() -> decltype(this);														\
	mc_accessor:																									\
		using mc_id##ClassTypeFunc =  std::remove_pointer<decltype(mc_id##class_type)>::type;						\
		using mc_id##ClassTypeRet =  std::invoke_result<mc_id##ClassTypeFunc>::type;								\
		using mc_id##ClassType =  std::remove_pointer<mc_id##ClassTypeRet>::type##mc_value_type


	#define DEFINE_CLASS_NAME() DEFINE_CLASS_NAME_AS(,, public)


	//This can be swapped out over time, for something like auto property that will swap out the first one.
	#define DEFINE_PROPERTY_TYPE(mc_prop_name)																								\
	static (*__macro__##mc_prop_name##_prop_type)();																						\
		private:																															\
		using __macro__##mc_prop_name##_PropTypeRaw = typename std::remove_pointer<decltype(__macro__##mc_prop_name##_prop_type)>::type;	\
		using __macro__##mc_prop_name##_PropType = std::invoke_result<__macro__##mc_prop_name##_PropTypeRaw>::type							\



		//The idea is, this would be used by the thing that actually sets up
	#define DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)	mc_accessor mc_struct_name mc_prop_name =	

	#define __macro__PROTECTED_NAME p_

	//This one is the chief example of how this works. The funniest part, is that protected is gonna need it's own special types.
	// In order to do private_protected, I'll need something like that. The rule will be, no private_private
	// aliased property can exist. I'll make it so there's a static assert in that one, and it should be the only one declared
	// by name.
	#define alias_property__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) 						\
		using __macro__##mc_prop_name##_alias_property_structure =											\
			alias_property_struct<																			\
			__macro__##mc_prop_name##_ClassType,															\
			__macro__##mc_prop_name##_PropType,																\
			&__macro__##mc_prop_name##_get,																	\
			&__macro__##mc_prop_name##_set>;																\
		friend class __macro__##mc_prop_name##_alias_property_structure;									\
	protected:																								\
	__macro__##mc_prop_name##_alias_property_structure CONCATENATE(__macro__PROTECTED_NAME, mc_prop_name);	\
	DEFINE_PROPERTY(mc_prop_name, __macro__##mc_prop_name##_property_structure, mc_accessor)




	#pragma region non_unique_define_macro

	//ALL of these, are the exact fucking same. Don't give a shit about any of them.
	//This should be set to whatever is considered the default.

	//These actually might get some level of uniqueness, each one needs to recieve something at the highest level they are.

	#define __DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define public__public__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define private__public__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define public__private__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define private__private__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define public__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define private__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define private_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define public_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define private_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define public_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define public_set__private_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define private_get__public_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)



	#define private_set__private_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define private_get__private_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define private_set__public_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define public_get__private_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)



	#define public_set__public_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define public_get__public_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define private_get__private_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define public_get__public_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define private_set__private_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define public_set__public_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define protected__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define protected__protected__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define private__protected__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define protected__private__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)



	#define protected_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define protected_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)





	#define protected_set__protected_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define protected_get__protected_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)



	#define private_set__protected_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define protected_get__private_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)


	#define protected_set__private_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define private_get__protected_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define protected_get__protected_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#define protected_set__protected_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor)

	#pragma endregion

	#pragma region protected_define_macro

	#define public_set__protected_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) \
		alias_property__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, public:)

	#define protected_get__public_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) \
		alias_property__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, public:)


	#define public__protected__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) \
		alias_property__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, public:)


	#define protected__public__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) \
		alias_property__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, public:)


	#define protected_set__public_get__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) \
		alias_property__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, public:)


	#define public_get__protected_set__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, mc_accessor) \
		alias_property__DEFINE_PROPERTY(mc_prop_name, mc_struct_name, public:)


	#pragma endregion




	//To this one, the value that should be accepted into the first part
	// should likely be what it should be setting and getting, and in this setting, it should be nothing.
	// In other settings, it should create a value, and point to that value.
	//TO this end, it should have a second variable here for getting the name of the variable, which is then used by a very 
	// similar object, however the heading part to get the prop name will have to be removed, and instead should be replaced with a version 
	// that gets it via type name.
	//Let me think. The macro can be split multiple times.
	// -Declaring PropType
	// -Declaring ClassName
	// -<!> From these, I think the declaration of the function types are constants, never changing, as well as the declaration of the 
	//  static getter and setter.
	// -Declaring property structure. Right now? Not much, later however, it will matter. Additionally, declaring protected should lead
	//  to a different function type entirely, but that might not matter much.
	// -The variable is the most important part, it is key, and not as simple as one would think
	#define __detail__PROPERTY_ACCESS(mc_prop_name, mc_accessor,...)																		\
		DEFINE_PROPERTY_TYPE(mc_prop_name);																									\
	/************************************************************************************************************************************/	\
		DEFINE_CLASS_NAME_AS(__macro__##mc_prop_name##_,&,private);																			\
	/************************************************************************************************************************************/	\
		using __macro__##mc_prop_name##_GetFuncType = __macro__##mc_prop_name##_PropType(*)(__macro__##mc_prop_name##_ClassType);			\
		using __macro__##mc_prop_name##_SetFuncType = void(*)(__macro__##mc_prop_name##_ClassType, __macro__##mc_prop_name##_PropType&);	\
	/************************************************************************************************************************************/	\
		static inline __macro__##mc_prop_name##_GetFuncType __macro__##mc_prop_name##_get;													\
		static inline __macro__##mc_prop_name##_SetFuncType __macro__##mc_prop_name##_set;													\
	/************************************************************************************************************************************/	\
		using __macro__##mc_prop_name##_property_structure =																				\
			CONCATENATE(__detail_ACCESSOR(__VA_ARGS__), property_struct)<																	\
			__macro__##mc_prop_name##_ClassType,																							\
			__macro__##mc_prop_name##_PropType,																								\
			&__macro__##mc_prop_name##_get,																									\
			&__macro__##mc_prop_name##_set>;																								\
		friend class __macro__##mc_prop_name##_property_structure;																			\
	/************************************************************************************************************************************/	\
		CONCATENATE(__detail_ACCESSOR(__VA_ARGS__), DEFINE_PROPERTY)(mc_prop_name, __macro__##mc_prop_name##_property_structure, mc_accessor)	

	//This last one does not need the second parameter in define_properterys

	//Old direct methodology
	//mc_accessor																															\
		__macro__##mc_prop_name##_property_structure mc_prop_name =																			

	//Friend class isn't really needed, I mean, sorta for the innereds, but this is fine as it is.

	//I don't think I need this.
	#define PROPERTY_ACCESS(mc_prop_name, mc_accessor, ...) __detail__PROPERTY_ACCESS(mc_prop_name, mc_accessor:, __VA_ARGS__)

	#define PROPERTY(mc_prop_name, ...) PROPERTY_ACCESS(mc_prop_name, public, __VA_ARGS__)

	//2 types, auto_property, and locked_property,the later of which will lock once set.

#define RESTRICTED_PROPERTY(mc_prop_name, mc_condition,...) \
	PROPERTY_ACCESS(mc_prop_name, public, __VA_ARGS__) {\
	GET(__macro__##mc_prop_name##_PropType) { return _this._##mc_prop_name; }\
	SET(__macro__##mc_prop_name##_PropType) { if (auto& _temp_ = _this._##mc_prop_name; (mc_condition) == true) _this._##mc_prop_name = value; }\
	}; protected: __macro__##mc_prop_name##_PropType _##mc_prop_name

#define PROPERTY_AUTO(mc_prop_name, ...) \
	PROPERTY_ACCESS(mc_prop_name, public, __VA_ARGS__) {\
	GET(__macro__##mc_prop_name##_PropType) { return _this._##mc_prop_name; }\
	SET(__macro__##mc_prop_name##_PropType) { _this._##mc_prop_name = value; }\
	}; protected: __macro__##mc_prop_name##_PropType _##mc_prop_name

#define PROPERTY_LOCK(mc_prop_name, ...) RESTRICTED_PROPERTY(mc_prop_name, !_temp_,__VA_ARGS__)
	
	//\
	PROPERTY_ACCESS(mc_prop_name, public, __VA_ARGS__) {\
	GET(__macro__##mc_prop_name##_PropType) { return _this._##mc_prop_name; }\
	SET(__macro__##mc_prop_name##_PropType) { if (!_this._##mc_prop_name) _this._##mc_prop_name = value; }\
	}; __macro__##mc_prop_name##_PropType _##mc_prop_name



//#define TEST_STUFF(mc_name, mc_conditions) if (auto& _temp_ = mc_name; (mc_conditions) == true) return
//#define TEST_IDEA(mc_stuff) TEST_STUFF(foo, mc_stuff)




	//Temp until I get this working better,
	//Turns out, I'm allowed to declare it like this, so I will so I can set the initial value of the property.
	// Make something called just AUTO_PROPERTY, when you don't want to create the varable, just encapsulate another one. This would take 
	// either 2 parameters, or would resolve to look like = _name; probably the former.fa;
	//#define MAKE_AUTO_PROPERTY(mc_prop_name) PROPERTY(mc_prop_name) {this, __macro__auto__##mc_prop_name }; __macro__auto__##mc_prop_name

	//so this is how setting the type works, the first value is get, the second value is set.
	// This looks like this {}_{}__property_detail.
	// All public has an alias that looks like __property_detail = public_public__property_detail. This way, having no args is equal to public.
	//setting a particular type looks like duing this "{}_set__property_detail" inputting "{}_set_{}_set__property_detail" will just result in the
	// previous. using get works the same way.
	//Overall, there's about 4 different combos. private_private, public_public, public_private, 

	//About the protected setting. this is best if you want things to inherit from it, then you'll make it public_public. While you can't have
	// also note, I want to make it so dereferencing gives you the get type or some type of operator like prop() or prop.get(). This way, when given
	// to function
	#define Private_Property(mc_prop_name) PROPERTY_ACCESS(mc_prop_name, private, __VA_ARGS__)


			//As much as you can, divorce these from the use of the macro types.
			//this, [](__macro__##mc_prop_name##_ClassType _this) ->__macro__##mc_prop_name##_PropType
			//this, [](__macro__##mc_prop_name##_ClassType _this, __macro__##mc_prop_name##_PropType& value) -> void

		//using GetFunc_This_name = PropType_name(*)(This);
		//using SetFunc_This_name = void(*)(This, PropType_name&);

	//ween get off of float, some how. Maybe auto is best? also, quit using THIS first, 
	#define GET(mc_type) *this, [](std::remove_pointer_t<decltype(this)>& _this) -> mc_type 
	//this will need a value before

	#define SET_AS(mc_type, mc_name) ,[](std::remove_pointer_t<decltype(this)>& _this, mc_type& mc_name) -> void
	#define SET(mc_type) SET_AS(mc_type, value)

	#define SET_ONLY_AS(mc_type, mc_name) *this, [](std::remove_pointer_t<decltype(this)>& _this, mc_type& mc_name) -> void
#define SET_ONLY (mc_type)
//}
#pragma endregion

namespace Test
{
#define INFO_PROPERTY(mc_prop_name,...) RESTRICTED_PROPERTY(mc_prop_name, !_temp_ && !_this._finished, __VA_ARGS__)
	//*
	
}
	enum ValueTYPERS {none, two};

	class example
	{
	public:
		bool temp = false;

		float PROPERTY(propertyName, public, protected)
		{
			GET(float)
			{
				logger::info("get fired. {}", _this.i);
				return _this.i;
			}
			SET(float)
			{
				int foo = 69;
				//if (auto _temp_ = foo; _temp_ == 69) return;
				logger::info("set fired, {}.", value);
				_this.i = value;
			}
		};
		ValueTYPERS RESTRICTED_PROPERTY(restrict_test, !_temp_ && !_this.temp, public);
		ValueTYPERS PROPERTY_LOCK(lockedProp, public, protected);

	private:
		float i = 69;
	public:

		void TestingFunction(bool test, bool& test2)
		{
			//using RefType = decltype(*this);
			//typedef std::remove_reference<RefType>::type Type;
			//auto b = &Type::bar;
			//auto b = &std::remove_reference<decltype(*this)>::type::bar;
			//foo_fighter.
			float test_float = propertyName;
			propertyName = 420.f;
			//logger::info("success, sent {}, {} : {} vs {}", test, test2, test_float, *propertyName);//this should be false if this works.

		}


		//*/
	};

