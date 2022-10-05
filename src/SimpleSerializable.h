#pragma once

#include "SerialArgument.h"


namespace Crucible
{
	//I think the way things are going, there's enough to suggest that all serializables and the serialize class
	// of something like timers share a core functionality.

	//Serializes simple data like bools, integers, floats simple and in packs.
	template<class SerializableData>
	class SimpleSerializable//This might basically be serializable data
	{
	protected:
		SerializableData _minorData;

	public:
		//Used to determine if data is relvent for serialization, if primary, it's version is max value, if data its value is zero
		// unless explicitly marked otherwise.

		constexpr SerializableData& GetSimpleData() { return _minorData; }
		//Need to see if this can resolve it's abiguity via access
		constexpr SerializableData& operator->() noexcept { return GetSimpleData(); }

		virtual bool	IsRelevant() { return true; }
		void			SerializeSimpleData(SerialArgument& serializer) { serializer.Serialize(_minorData); }

	};


}