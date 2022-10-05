#pragma once

#include "SerialConstructor.h"
#include "SerialArgument.h"


namespace Crucible
{

	//Would be helpful having this derive from something like iterator.
	template<class TypeA, class TypeB>
	class SerializablePair : public SerializationHandler
	{
	private: 
		using Pair = std::pair<TypeA, TypeB>;
	protected:
		Pair _pair;
	public:

		void HandleSerialize(SerialArgument& serializer, bool& success) override 
		{ 
			serializer.Serialize(_pair.first);
			serializer.Serialize(_pair.second);
		}

		TypeA& first = _pair.first;
		TypeB& second = _pair.second;

		constexpr Pair& get() noexcept { return _pair; }

		constexpr Pair* operator->() noexcept { return &_pair; }



	};


}