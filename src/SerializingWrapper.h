#pragma once

#include "ISerializer.h"
#include "SerialArgument.h"


namespace Crucible
{
	//Empty class used for the uniform detection of SerializingWrappers
	class ISerialWrapperBase : public ISerializer
	{
		//static void HandleSerialize(ISerialWrapperBase& base, SerialArgument& serializer);

	};


	//template<typename T, typename U>
	//concept WrappedFriend = requires (T x) {
	//	static_cast<U>(x);
	//}
	template <class WrapType>//This exists for name. it makes filling serial iterators easier, you just use derived_from this.
	class SerialWrapperBase : public ISerializer
	{

	};

	template< class Derived, class Base >
	concept DerivedSerialWrapper =
		std::is_base_of_v<SerialWrapperBase<Base>, Derived> &&
		std::is_convertible_v<const volatile Derived*, const volatile SerialWrapperBase<Base>*>;

	//Purpose is the simple supplanting of the original handle without making an extra class or anything like that.
	// Reverse of the serializable classes, which have the way they serialize built in, this is a type that seeks to have
	// the rules to how it serializes a case by case (see CombatDataMap,) while not taking up any extra space.
	//Additionally, it excels at handling objects who can't have this sort of functionality, as well as being able to double
	// as a version controlling object. Though, that would need some work to set up.
	template <class WrapType, void(*REPLACE_WRAP_HANDLE)(WrapType&, SerialArgument&, bool&) = nullptr>
	class SerializingWrapper final : public SerialWrapperBase<WrapType>
	{
	public:
		using Type = WrapType;
		using Wrapper = SerializingWrapper<WrapType, REPLACE_WRAP_HANDLE>;
		



	private:
		mutable WrapType _wrapObject;
		//static_assert(sizeof(SerialzingWrapper<WrapType, REPLACE_WRAP_HANDLE>) != 0x0)

	public:
		constexpr WrapType& GetWrapObject() { return _wrapObject; }

		//void HandleSerialize(SerialArgument& serializer, bool& success);
		constexpr void HandleSerialize(SerialArgument& serializer, bool& success) const
		{
			if constexpr (REPLACE_WRAP_HANDLE)
				return REPLACE_WRAP_HANDLE(_wrapObject, serializer, success);
			else
				success = serializer.Serialize(_wrapObject);
		}


		#pragma region Constructors_And_Assignment
		SerializingWrapper() = default;

		//Copy Constructor
		SerializingWrapper(WrapType& other) { _wrapObject = other; }

		//Copy Constructor
		SerializingWrapper(WrapType other) { _wrapObject = other; }


		//Copy Assignment
		SerializingWrapper& operator=(WrapType& a_rhs) { _wrapObject = a_rhs; }

		//Copy Assignment with loose
		//SerializingWrapper& operator=(WrapType a_rhs) { _wrapObject = a_rhs; }

		//Explicit move assignment
		//SerializingWrapper& operator=(WrapType&& a_rhs) { _wrapObject = a_rhs; }


		//Copy Assignment
		template<std::convertible_to<WrapType> TransType>
		SerializingWrapper& operator=(TransType& a_rhs) { _wrapObject = a_rhs;  return *this; }

		//Explicit move assignment
		template<std::convertible_to<WrapType> TransType>
		SerializingWrapper& operator=(TransType&& a_rhs) { _wrapObject = a_rhs; return *this; }



		// implicit conversion
		operator WrapType&() const { return _wrapObject; }

		template<std::convertible_to<WrapType> TransType>
		operator TransType () const { return _wrapObject; }
		//operator WrapType&() const { return _wrapObject; }


		//template <class TypeFriend> requires(std::is_convertible<WrapType, TypeFriend>::value)
		//operator TypeFriend() const { return _wrapObject; }

		//explicit conversion
		explicit operator WrapType* () const { return (&_wrapObject); }

		constexpr WrapType* operator->() { return &_wrapObject; }

		constexpr bool operator < (const Wrapper& _Right) const {
			return _wrapObject < _Right._wrapObject;
		}
		#pragma endregion
	};
}