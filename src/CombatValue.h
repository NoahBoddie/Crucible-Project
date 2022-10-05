#pragma once

#include "Utility.h"
#include "QueryContext.h"
#include "CombatEffect.h"

namespace Crucible
{
	class CombatData;
	class CacheCombatValue;
	
	enum class CombatValue : std::uint8_t
	{
		//Core
		kResist,
		kEvade,
		kStagger,
		kPrecision,
		//Resource
	};

	

	



	struct SimpleCombatValue
	{
		std::float_t totalMod{ 1.f };

		std::float_t positiveBase{ 0 };
		std::float_t negativeBase{ 0 };

		std::float_t generalMod{ 1.f };
		std::float_t positiveMod{ 1.f };
		std::float_t negativeMod{ 1.f };


		virtual std::string print()
		{ 
			return std::format("genMult: {}\n pos:{} \nposMult: {} \nneg: {} \nnegMult: {} \ntotalMult: {}",
			generalMod, positiveBase, positiveMod, negativeBase, negativeMod, totalMod).c_str();
		}

		virtual void prime()
		{
			//The mods I won't worry about, I cant be fucked that this point. But, they're supposed to have a minimum value
			// which gets breached if a number is high enough. I think I'll just do it simply like this, negative will not register 
			// as negative until a certain point. I want it to be something like 1, and that representing the minimum value
			// a mult can have in general. So a value of -1 mult would be for example -0.15. Any further reductions would then
			// be subject to reduction, as to not ruin the main value too harshly.
			//If a person wants to edit it raw, there can be a flag for translation or something. 
			//totalMod;

			positiveBase = fmax(0, positiveBase);
			negativeBase = fmax(0, negativeBase);
		}

		static SimpleCombatValue zero() 
		{ 
			SimpleCombatValue value;
			value.generalMod = 0;
			value.positiveMod = 0;
			value.negativeMod = 0;
			value.totalMod = 0;
			return value; 
		}

		virtual SimpleCombatValue& operator=(const std::float_t& a_rhs) {
			//sets the generalBase, but here does nothing
			return *this;
		}

		virtual SimpleCombatValue& operator&=(const std::float_t& a_rhs) {
			//Increments generalBase, but here does nothing
			return *this;
		}

		SimpleCombatValue& operator^=(const std::float_t& a_rhs) {
			//Increments the totalMod

			totalMod += a_rhs;
			return *this;
		}
		SimpleCombatValue& operator%=(const std::float_t& a_rhs) {
			//Increments the generalMod
			generalMod += a_rhs;
			return *this;
		}
		SimpleCombatValue& operator+=(const std::float_t& a_rhs) {
			//Increments the postiveBase
			positiveBase += a_rhs;
			return *this;
		}
		SimpleCombatValue& operator-=(const std::float_t& a_rhs) {
			//Increments the negativeBase
			negativeBase += a_rhs;
			return *this;
		}
		

		SimpleCombatValue& operator*=(const std::float_t& a_rhs) {
			//Increments the positiveMod
			positiveMod += a_rhs;
			return *this;
		}
		SimpleCombatValue& operator/=(const std::float_t& a_rhs) {
			//Increments the negativeMod
			negativeMod += a_rhs;
			return *this;
		}


		virtual SimpleCombatValue& operator+(const SimpleCombatValue& a_rhs);

		virtual SimpleCombatValue& operator-(const SimpleCombatValue& a_rhs);

		virtual CacheCombatValue& operator+(const CacheCombatValue& a_rhs);

		virtual CacheCombatValue& operator-(const CacheCombatValue& a_rhs);


		//Want an operator that makes this into a cached if operated on with a cache. But need a source file to do so.

	protected:
		virtual void Clear()
		{
			totalMod = 0;
			positiveBase = 0;
			negativeBase = 0;

			generalMod = 0;
			positiveMod = 0;
			negativeMod = 0;
		}
	};


	struct CacheCombatValue : public SimpleCombatValue
	{

	public:
		//As confusing as I'm sure it will be, the general base represents many things now.
		// it represents the damage mod for resources, and a post mod for core values. Either way, when loaded
		// the original general base is thrown aside, and replaced with the calculated general base. Notable exceptions
		// are core values resist and evade which both need it.
		std::float_t generalBase{ 0 };

		std::string print() override
		{
		
			return std::format("gen: {} \n{}", generalBase, __super::print());
		}

		void prime() override
		{
			__super::prime();
			generalBase = fmax(0, generalBase);

		}

		static CacheCombatValue zero() { CacheCombatValue value; return value; }


		CacheCombatValue& operator=(const std::float_t& a_rhs) override {
			//sets the generalBase

			generalBase = a_rhs;
			return *this;
		}

		CacheCombatValue& operator&=(const std::float_t& a_rhs) override {
			//Increments generalBase

			generalBase += a_rhs;
			return *this;
		}

		CacheCombatValue& operator+=(const std::float_t& a_rhs) {
			//Increments the postiveBase, needs to be redone because of later operators? I guess????
			positiveBase += a_rhs;
			return *this;
		}

		CacheCombatValue& operator+(const SimpleCombatValue& a_rhs) override  {
			//makes a new value of the left hand values adds the values of the right hand
			CacheCombatValue a_lhs = *this;

			a_lhs.totalMod += a_rhs.totalMod;

			a_lhs.positiveBase += a_rhs.positiveBase;
			a_lhs.negativeBase += a_rhs.negativeBase;

			a_lhs.generalMod += a_rhs.generalMod;
			a_lhs.positiveMod += a_rhs.positiveMod;
			a_lhs.negativeMod += a_rhs.negativeMod;
			return a_lhs;
		}

		CacheCombatValue& operator-(const SimpleCombatValue& a_rhs) override {
			//makes a new value of the left hand values reduced by the right hand;
			CacheCombatValue a_lhs = *this;

			a_lhs.totalMod -= a_rhs.totalMod;

			a_lhs.positiveBase -= a_rhs.positiveBase;
			a_lhs.negativeBase -= a_rhs.negativeBase;

			a_lhs.generalMod -= a_rhs.generalMod;
			a_lhs.positiveMod -= a_rhs.positiveMod;
			a_lhs.negativeMod -= a_rhs.negativeMod;
			
			return a_lhs;
		}



		CacheCombatValue& operator+(const CacheCombatValue& a_rhs) override 
		{
			//makes a new value of the left hand values adds the values of the right hand
			CacheCombatValue a_lhs = *this;

			a_lhs.generalBase += a_rhs.generalBase;
			
			a_lhs.totalMod += a_rhs.totalMod;

			a_lhs.positiveBase += a_rhs.positiveBase;
			a_lhs.negativeBase += a_rhs.negativeBase;

			a_lhs.generalMod += a_rhs.generalMod;
			a_lhs.positiveMod += a_rhs.positiveMod;
			a_lhs.negativeMod += a_rhs.negativeMod;

			return a_lhs;
		}

		CacheCombatValue& operator-(const CacheCombatValue& a_rhs) override
		{
			//makes a new value of the left hand values reduced by the right hand;
			CacheCombatValue a_lhs = *this;

			a_lhs.generalBase -= a_rhs.generalBase;


			a_lhs.totalMod -= a_rhs.totalMod;

			a_lhs.positiveBase -= a_rhs.positiveBase;
			a_lhs.negativeBase -= a_rhs.negativeBase;

			a_lhs.generalMod -= a_rhs.generalMod;
			a_lhs.positiveMod -= a_rhs.positiveMod;
			a_lhs.negativeMod -= a_rhs.negativeMod;

			return a_lhs;

		}


		CacheCombatValue& operator=(const CacheCombatValue& a_rhs) {
			//makes a new value of the left hand values adds the values of the right hand
			generalBase = a_rhs.generalBase;
			totalMod = a_rhs.totalMod;

			positiveBase = a_rhs.positiveBase;
			negativeBase = a_rhs.negativeBase;

			generalMod = a_rhs.generalMod;
			positiveMod = a_rhs.positiveMod;
			negativeMod = a_rhs.negativeMod;

			return *this;
		}

		CacheCombatValue& operator=(const SimpleCombatValue& a_rhs) {
			//makes a new value of the left hand values adds the values of the right hand
			totalMod = a_rhs.totalMod;

			positiveBase = a_rhs.positiveBase;
			negativeBase = a_rhs.negativeBase;

			generalMod = a_rhs.generalMod;
			positiveMod = a_rhs.positiveMod;
			negativeMod = a_rhs.negativeMod;

			return *this;
		}


		CacheCombatValue& operator+=(const CacheCombatValue& a_rhs) {
			//makes a new value of the left hand values adds the values of the right hand
			generalBase += a_rhs.generalBase;
			totalMod += a_rhs.totalMod;

			positiveBase += a_rhs.positiveBase;
			negativeBase += a_rhs.negativeBase;

			generalMod += a_rhs.generalMod;
			positiveMod += a_rhs.positiveMod;
			negativeMod += a_rhs.negativeMod;

			return *this;
		}

		CacheCombatValue& operator+=(const SimpleCombatValue& a_rhs) {
			//makes a new value of the left hand values adds the values of the right hand
			totalMod += a_rhs.totalMod;

			positiveBase += a_rhs.positiveBase;
			negativeBase += a_rhs.negativeBase;

			generalMod += a_rhs.generalMod;
			positiveMod += a_rhs.positiveMod;
			negativeMod += a_rhs.negativeMod;

			return *this;
		}



		//CacheCombatValue(float init)
		//	: generalBase(init)
		//{ }

		float operator()() {
			float returnValue = totalMod * (generalBase * generalMod) + (positiveBase * positiveMod - negativeBase * negativeMod);
			return returnValue;
		}

	protected:
		void Clear() override
		{
			generalBase = 0;
			__super::Clear();
		}
	};



	enum ValueInputType : std::uint8_t
	{
		Set,							//Overwrites whatever is submitted if it finds one with a matching id.
		Modify,							//Modify will increase or decrease the value already presented
		Delete,							//Delete will ignore value, and delete any current records.

		ModifyNoCross = Modify | 0b100 	//Prevents modifiers from crossing from negative to positive or vice versa
	};


	enum struct ValueInputIdFlag : std::uint8_t
	{
		None,
		RefuseSave
	};


	struct ValueInputID
	{
	private:
		static constexpr std::uint8_t k_saveBit = 0b0001;
	public:
		union
		{
			uint32_t _id{};
			char _charID[4];
		};
		ValueModifier	 _modifier{};
		CombatValue		_valueType{};

		//In order to compare this, I could make this a union between

		constexpr bool IsCoreInput() { return _id & 0xFF; }

		constexpr bool IsSavable() { return IsCoreInput() || (_id >> 8 & k_saveBit); }

		constexpr ValueInputIdFlag GetInputFlags()
		{
			if (IsCoreInput() == true)
				return ValueInputIdFlag::None;

			ValueInputIdFlag flags{};

			flags |= IsSavable() ? ValueInputIdFlag::RefuseSave : ValueInputIdFlag::None;

			return flags;
		}


		constexpr ValueInputID() = default;
		constexpr ValueInputID(uint32_t id, ValueModifier mod) : _id(id), _modifier(mod) {}
		constexpr ValueInputID(uint32_t id, ValueModifier mod, CombatValue value) : _id(id), _modifier(mod), _valueType(value) {}

		operator std::uint32_t& () { return _id; }
		//operator std::uint32_t&() const { return _id; }

		ValueInputID& operator=(std::uint32_t new_id) { _id = new_id; return *this; }
		ValueInputID& operator=(ValueModifier new_mod) { _modifier = new_mod; return *this; }

		//100-4-0 110-3-0 

		constexpr uint64_t compare_value() const
		{
			uint64_t value = _id;
		
			auto push = sizeof(_id) * 8;

			value |= (uint64_t)_modifier << push;

			push += sizeof(_modifier) * 8;
			
			value |= (uint64_t)_valueType << push;

			return value;
		}

		//Some how, it's the compare that's fucking up. It doesn't return true when the modifiers are less than each other.
		//1297045042/4 vs new: 1297045042/1   first: false/false && second: false/true
		constexpr bool operator < (const ValueInputID& a_rhs) const {
			//return (_id < a_rhs._id) || (_modifier < a_rhs._modifier) || (_valueType < a_rhs._valueType);
			
			//I'm allowed to do this, so I suggest that I make a union with the thing that's supposed to represent its bytes so I can handle over it's data easier.
			//std::byte bytes_1[8];
			//std::byte bytes_2[8];
			//return bytes_1 < bytes_2;

			return compare_value() < a_rhs.compare_value();
		}
	};

	//A major note that I'll try to keep to is keep a small section of scramble away from the ids that magic effects would use.
	//Which speaking of, this is how those would deal with that. I believe I would like some way to make a hash out of ids, and then transfer them back.
	class CombatValueInput
	{
	public:
		union
		{
			uint32_t _id{ 0 };
			char _charID[4];
		};
		bool _add{true};
		float _value{0.f};
		//stl::enumeration<ValueInputType, uint8_t> _inputType;
		ValueInputType _inputType{ ValueInputType::Set };

		ValueInputType GetInputType() const
		{
			if (_inputType == ValueInputType::ModifyNoCross && _add)
				return ValueInputType::Modify;//If it's not removing, no need to prevent it from crossing. From it's perspective it's the right direction.

			return _inputType;
		}

		float GetValue() const
		{
			switch (GetInputType())
			{
			case ValueInputType::Set:
				if (_add)
					return _value;
			case ValueInputType::Delete:
				return 0;
			default:
			case ValueInputType::Modify:
			case ValueInputType::ModifyNoCross:
				return _value * (_add ? 1 : -1);
			}
		}



		//Notice, instead of doing this dumb shit, why not have a tally score of orders for one way or another if there's a default or nullify command running?
		//Certainly more expensive, no doubt.

		//Each of these play by the rules that the float limit for inputs will be some where in the ball park of the 8 bit integer limit. Once it goes over that,
		// The value is reinterpreted. I'm considering messing with the numbers to make it doubtful one could even reach such numbers.

		//Nullifies main goal is to use set to subtract the value by a ton, putting it to 0 when interpreted. It should be noted that this overrides default.
		static constexpr CombatValueInput Nullify(uint32_t id, bool add) { return CombatValueInput(id, ValueInputType::Set, -0xFFFF, add); }
		

		//Defaults a value by setting it to 1000 of it's value, forcing it to be interpreted as it's default value. While it's techically possible to 
		// have this override Nullify, you'd need a lot of default orders.
		static constexpr CombatValueInput Default(uint32_t id, bool add) { return CombatValueInput(id, ValueInputType::Set, 0x0FFF, add); }



		constexpr CombatValueInput(uint32_t id, ValueInputType input) : _id(id), _inputType(input) {}
		constexpr CombatValueInput(uint32_t id, float value, bool add) : _id(id), _add(add), _value(value) {}
		constexpr CombatValueInput(uint32_t id, ValueInputType input, float value, bool add) : _id(id), _add(add), _value(value), _inputType(input) {}
		
		//template <unsigned short short int I>requires(I != 0 & I <= 4)
		//constexpr CombatValueInput(char[I] id, ValueInputType input, float value, bool add) : _id(id), _add(add), _value(value), _inputType(input) {}
		//1 for the string has to make sure it's a core input.
		constexpr CombatValueInput(std::string name, ValueInputType input, float value, bool add) : _id(hash(name) | 1), _add(add), _value(value), _inputType(input) {}
	};

	//Other notes on Inputs, I would like a macro that will automatically use a name of the object it's firing it, possibly using
	// decltype(this) in order to resolve it. Basically, if the type has something that dictates how it would like to handle it's add, it will use that
	// and if it

	//To the above, it's mostly data, so I think what I'd like is, this type gets broken down and reduced to just an id and a float. No need to store the rest.
	// Also note, the first byte is reserved, and the randomly generated ids are handled via itself.
	// For something like that, I think I'll keep track of the smallest random id, and the largest and if it needs to wrap around. But since these are local, don't think I need.

	//Another important set of bits, I'm thinking the first 4 to the left of the first set. These bits can represent important flags.
	// First one? Do not save. This should be used for active magic effects as they'd apply their effects again on reload, but we'd want to make sure
	// they exist. These flags should only be relevant if it's not a core input (IE, what we do here).


	template<bool Is_Cache, typename Enable = void>
	struct ContainerData;

	// my favourite type :D
	template<bool Is_Cache>
	struct ContainerData<Is_Cache, std::enable_if_t<Is_Cache>> {
	public:
		CacheCombatValue value{};
		float mainInput{};//This is used for evade and resist to easily set and control it's main base type.
		
		constexpr CacheCombatValue* operator->() { return &value; }
		operator CacheCombatValue&() { return value; }
		operator float&() { return mainInput; }
	};

	// not my favourite type :(
	template<bool Is_Cache>
	struct ContainerData<Is_Cache, std::enable_if_t<!Is_Cache>> {
	public:
		using CombatValueType = std::conditional_t<Is_Cache, CacheCombatValue, SimpleCombatValue>;

		SimpleCombatValue value;

		constexpr SimpleCombatValue* operator->() { return &value; }
		operator SimpleCombatValue&() { return value; }
	};

	using CombatValueIDMap = std::map<ValueInputID, float>;//Make serializable when you can.

	template <bool Is_Cache>
	struct CombatValueContainer final
	{
		using CombatValueType = std::conditional_t<Is_Cache, CacheCombatValue, SimpleCombatValue>;
	private:
		//CombatValueType _coreValue{};
		ContainerData<Is_Cache> _coreValue{};
		
		const CombatValue _valueType;

		//std::list<CombatValueInput*> _inputList;
		//Ideally, I want a type with consistent delete times, but that doesn't necessarily need to be sorted.
		//Additional notes, all containers will share this. This will mean this object will need to be initialized, for this will be a reference.
		// The actorcombatValueOwner will be the thing that owns this, and IDs will get combat value types assigned to them as well.
		CombatValueIDMap& _inputMap;//Soon to be serializable.
		
	public:
		//The first thing we do is look for input. A didn,t, B we did

		//If A, we take the id, get the proper ValueModifier for the function used, and place it in the map, and stick the float in. Then we send that off to
		// the original.

		//Scratch this, what we first do is check the input type, if it's set we immediately use

		CombatValueType& core() { return _coreValue.value; }

		bool RecieveInput(ValueModifier modifier, float value, bool add)
		{
			if (value == 0)//Value does nothing, no need to waste a record for it.
			{
				//logger::info("Void");
				return false;

			}

			value *= add ? 1.f : -1.f;
			//logger::info("else {} {}/{}", value, add, (unsigned int)modifier);
			switch (modifier)
			{
			case ValueModifier::kGeneralMod:
				_coreValue->generalMod += value;
				break;
			case ValueModifier::kNegativeBase:
				_coreValue->negativeBase += value;
				break;
			case ValueModifier::kNegativeMod:
				_coreValue->negativeMod += value;
				break;
			case ValueModifier::kPositiveBase:
				//logger::info("occur {} {}", value, add);
				_coreValue->positiveBase += value;
				break;
			case ValueModifier::kPositiveMod:
				_coreValue->positiveMod += value;
				break;
			case ValueModifier::kTotalMod:
				_coreValue->totalMod += value;
				break;

			case ValueModifier::kGeneralBase:
				if constexpr (Is_Cache) {
					_coreValue->generalBase += value;
					break;
				}
				else {
					//logger::error("General base is not allowed for simple resource containers.");
					return false;
				}
			}

			return true;
		}

		inline bool EnableInput(ValueModifier modifier, float value) { return RecieveInput(modifier, value, true); }

		inline bool DisableInput(ValueModifier modifier, float value) { return RecieveInput(modifier, value, false); }

		bool CanBeNewInput(ValueInputType input)
		{
			switch (input)
			{
			case ValueInputType::Delete:		//Delete does nothing on it's own
			case ValueInputType::ModifyNoCross:	//Modify no cross is intended to be a decrement to zero. If it's at zero, it's reached it's goal.
				return false;
			default:
				return true;
			}
		}
		//
		void HandleInput(ValueInputID id, ValueInputType input, float value)
		{
			//situations to look out for, trying to set zero. Set can end up here, but consider, it will do nothing 
			// and muck up a slot.

			//Also, mod twists the value already, no need to do it again.

			//I believe in part modification is an issue. If I'm looking to modify, but negative, it gets flipped. Like 3 times I believe?
			// It might not be totally but examine it.

			//Set is the problem. Even though its supposed to set, it's not handling properly.

			auto input_set = _inputMap.find(id);

			if (_inputMap.end() == input_set) {
				if (CanBeNewInput(input) && EnableInput(id._modifier, value)) {
					//If you use an invalid type like base this wont save the object.
					_inputMap[id] = value;
					//logger::info("apply- {} {}", value, _inputMap.size());

				}
				else
				{
					//logger::info("NO ADD OR EXIST: {:X}/{}/{}", id._id, (unsigned int)id._modifier, (unsigned int)id._valueType);
				}

				return;
			}
			ValueInputID store_id = input_set->first;
			float& store_value = input_set->second;

			float combine_value = store_value + value;

			if (store_id != id)
				RE::DebugMessageBox(std::format("stored: {:X}/{}/{} vs new: {:X}/{}/{}", store_id._id, (unsigned int)store_id._modifier, (unsigned int)store_id._valueType,
					id._id, (unsigned int)id._modifier, (unsigned int)id._valueType));

			//logger::info("stored: {:X}/{}/{} vs new: {:X}/{}/{}, cmp({}) && cmp({}", store_id._id, (unsigned int)store_id._modifier, (unsigned int)store_id._valueType,
			//	id._id, (unsigned int)id._modifier, (unsigned int)id._valueType, store_id < id, id < store_id);

			switch (input)
			{
			case ValueInputType::ModifyNoCross:
				//If this will cross into zero, delete it
				if ((store_value < 0 && combine_value > 0) || (store_value > 0 && combine_value < 0)) {
					//logger::info("test_mod_nc- cv:{} cmp_v:{}", value, combine_value);
					goto do_delete;//I can encap everything upto delete.
				}
			case ValueInputType::Modify:
				//logger::info("test_mod- cv:{} cmp_v:{}", value, combine_value);

				value = combine_value;

			case ValueInputType::Set:
				if (value != 0)//if zero, it's the same as delete.
				{
					DisableInput(id._modifier, store_value);
					EnableInput(id._modifier, value);
					store_value = value;
					//logger::info("test- sv:{} cv:{} rv:{}", input_set->second, value, store_value);

					break;
				}
			case ValueInputType::Delete:
				//If delete, erase location, and return false so no further action comes about. Nullify the value
			do_delete:
				//logger::info("literally anything should show up between this.");
				DisableInput(id._modifier, store_value);
				//logger::info("test({})({})[{}] cv:{} rv:{}", (unsigned int)input, (unsigned int)id._modifier, (unsigned int)id._valueType, value, store_value);
				_inputMap.erase(input_set);

				break;
			}
		}

		//This would be the core function used by none core types.
		void RemoveInputAt(ValueInputID id)
		{
			auto input_set = _inputMap.find(id);

			if (_inputMap.end() == input_set) {
				return;
			}
			else
			{
				ValueInputID& id = input_set->first;
				float& store_value = input_set->second;
				DisableInput(id._modifier, store_value);
			}

			_inputMap.erase(input_set);
		}

		inline std::string print() { return _coreValue->print(); }

		//Enable this when were done. I want to draw out  =float
		//*
		CombatValueContainer& operator=(const float& a_rhs) requires(Is_Cache) {
			//Increments generalBase, but as a one off. This is used for default cache values, like the resist or evade an animal has.
			DisableInput(ValueModifier::kGeneralBase, _coreValue.mainInput);
			EnableInput(ValueModifier::kGeneralBase, a_rhs);
			_coreValue.mainInput = a_rhs;

			return *this;
		}
		//*/

		operator CombatValueType&() { return _coreValue.value; }

		CombatValueContainer& operator&=(const CombatValueInput& a_rhs) requires(Is_Cache) {
			//Increments generalBase
			ValueInputID id = { a_rhs._id, ValueModifier::kGeneralBase, _valueType };
			HandleInput(id, a_rhs.GetInputType(), a_rhs.GetValue());

			return *this;
		}

		CombatValueContainer& operator^=(const CombatValueInput& a_rhs) {
			//Increments the totalMod
			ValueInputID id = { a_rhs._id, ValueModifier::kTotalMod, _valueType };
			HandleInput(id, a_rhs.GetInputType(), a_rhs.GetValue());

			return *this;
		}
		CombatValueContainer& operator%=(const CombatValueInput& a_rhs) {
			//Increments the generalMod
			ValueInputID id = { a_rhs._id, ValueModifier::kGeneralMod, _valueType };
			HandleInput(id, a_rhs.GetInputType(), a_rhs.GetValue());
			//This one is having major issues. I'm not sure why, but it won't get deleted. Or maybe, something else is meeting the 
			// constraints? Like the other 2 meet it, and the third category doesnt? And it's getting dumped somewhere i cant see like negative mult?
			return *this;
		}
		CombatValueContainer& operator+=(const CombatValueInput& a_rhs) {
			//Increments the postiveBase
			ValueInputID id = { a_rhs._id, ValueModifier::kPositiveBase, _valueType };
			HandleInput(id, a_rhs.GetInputType(), a_rhs.GetValue());

			return *this;
		}
		CombatValueContainer& operator-=(const CombatValueInput& a_rhs) {
			//Increments the negativeBase
			ValueInputID id = { a_rhs._id, ValueModifier::kNegativeBase, _valueType };
			HandleInput(id, a_rhs.GetInputType(), a_rhs.GetValue());

			return *this;
		}


		CombatValueContainer& operator*=(const CombatValueInput& a_rhs) {
			//Increments the positiveMod
			ValueInputID id = { a_rhs._id, ValueModifier::kPositiveMod, _valueType };
			HandleInput(id, a_rhs.GetInputType(), a_rhs.GetValue());

			return *this;
		}
		CombatValueContainer& operator/=(const CombatValueInput& a_rhs) {
			//Increments the negativeMod
			ValueInputID id = { a_rhs._id, ValueModifier::kNegativeMod, _valueType };
			HandleInput(id, a_rhs.GetInputType(), a_rhs.GetValue());

			return *this;
		}


		constexpr CombatValueContainer(CombatValue value, CombatValueIDMap& map) : _valueType(value), _inputMap(map) {}

	};

	using CachedCombatValueContainer = CombatValueContainer<true>;
	using SimpleCombatValueContainer = CombatValueContainer<false>;


	//I want an interface between this and ConditionalCombatValue

	//What I want is combat effects, They work similar to the lambda that are kept. They have their effect's special id
	// kept close to them. They have apply, dispel, and change functions. CombatValueEffects Don't use any of these,
	// as they evaluate in the time, checking for it's own conditions, and if it has none sending it's value.


	struct CombatValueEffect : public CombatEffect
	{
		//Needs a centalized active effect, this should only be one branch of an effect.
		//Not quite finished yet, don't use.
	protected:
		CombatValue		valueType;
		ValueModifier	modifier;


		bool conditional;//I attached to an effect that has counter effects it's marked as conditional.


	public:

		bool IsConditional() { return conditional; }


		SimpleCombatValue temp_AsCombatValue()
		{
			SimpleCombatValue tempValue;

			float value = GetMagnitude();

			switch (modifier)
			{
			case ValueModifier::kGeneralMod:
				tempValue.generalMod = value;
				break;

			case ValueModifier::kNegativeBase:
				tempValue.negativeBase = value;
				break;

			case ValueModifier::kNegativeMod:
				tempValue.negativeMod = value;
				break;

			case ValueModifier::kPositiveBase:
				tempValue.positiveBase = value;
				break;

			case ValueModifier::kPositiveMod:
				tempValue.positiveMod = value;
				break;

			case ValueModifier::kTotalMod:
				tempValue.totalMod = value;
				break;
			}

			return tempValue;
		}


		void TryApplyValue(SimpleCombatValue& value, QueryContext& context)
		{
			if (true)//If the conditions set upon ye ring true,
			{
				value = temp_AsCombatValue();
			}
		}

		Archetype GetArchetype() override { return Archetype::kValueModifier; }
	};

	
	
	/*
	struct CombatValue
	{
	public:
		std::int8_t generalBase{ 0 };
		std::int8_t positiveBase{ 0 };
		std::int8_t negativeBase{ 0 };

		std::float_t generalMod{ 1.f };
		std::float_t positiveMod{ 1.f };
		std::float_t negativeMod{ 1.f };

		std::float_t totalMod{ 1.f };

		std::int8_t IncValue(std::int8_t inc)
		{
			generalBase += inc;
			return generalBase;
		}

		//The short hand, I would like this to be implicit later.
		float DeleteLater()
		{
			float returnValue = totalMod *(generalBase * generalMod) + (positiveBase * positiveMod - negativeBase * negativeMod);

			return returnValue / 2;
		}


		CombatValue& operator=(const std::int8_t& a_rhs) {
			//Unlike the others, sets the general base. It's annoying to have to set it like this but 
			// for the convience of the others.

			generalBase = a_rhs;
			return *this;
		}
		CombatValue& operator&=(const std::int8_t& a_rhs) {
			//Unlike the others, sets the general base. It's annoying to have to set it like this but 
			// for the convience of the others.

			generalBase += a_rhs;
			return *this;
		}
		CombatValue& operator^=(const std::float_t& a_rhs) {
			//Unlike the others, sets the general base. It's annoying to have to set it like this but 
			// for the convience of the others.

			totalMod = a_rhs;
			return *this;
		}
		CombatValue& operator+=(const std::int8_t& a_rhs) {
			//Increments the postiveBase
			positiveBase += a_rhs;
			return *this;
		}
		CombatValue& operator-=(const std::int8_t& a_rhs) {
			//Increments the negativeBase
			negativeBase += a_rhs;
			return *this;
		}
		CombatValue& operator%=(const std::float_t& a_rhs) {
			//Increments the generalMod
			generalMod += a_rhs;
			return *this;
		}
		CombatValue& operator*=(const std::float_t& a_rhs) {
			//Increments the positiveMod
			positiveMod += a_rhs;
			return *this;
		}
		CombatValue& operator/=(const std::float_t& a_rhs) {
			//Increments the negativeMod
			negativeMod += a_rhs;
			return *this;
		}
		//implicitly casts to a float, and when it does it is the result of everything together.
		// however, it's important to note that a function should be the thing that does this, and that function
		// has to accept conditional values, just the operator does it with none.
		
		//At a later point I want to override the operators for this. 
		// = sets generalBase, += sets positiveBase -= sets negative base.
		//And never mind, do it now.
	};


	struct DualCombatValue : public CombatValue
	{
		//This is the only thing that makes an impact on left hands.
		std::int8_t leftGeneralBase{ 0 };
	};
	//*/

	//Kill
	using ValueEffectContainer = std::map<const CombatValue, std::list<CombatValueEffect*>>;
	//using CombatValueContainer = std::map<const CombatValue, CacheCombatValue>;
}