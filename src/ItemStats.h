#pragma once

#include <cmath>

namespace Crucible
{

	//When I can't the difference between first blow and regular should be
	// first for first blow, follow for the follow up attacks.
	enum StatType//not class because I want to access it like normal.
	{
		//Minimum
		baseStagger,
		guardDamageMult,
		specialValue,		//No defined use, but is used for the special differences between specific item categories that it will not share with others.
		//Basic
		basePrecision,
		//Shield
		powWeightMult,
		counterBashStagger,
		counterBashDamageMult,
		counterBashRevengeDamageMult,
		counterBashStaminaBase,
		counterBashStaminaMult,
		counterBashHealthReq,
		counterBashCountReq,
		counterHitDecay,
		strongCounterHitDecay,
		strongGuardRecovery,
		//Bash
		bashStagger,
		bashPrecision,
		regBashWeightMult,
		powBashWeightMult,
		//Item
		baseStaminaCost,
		staminaCostClimb,
		staminaLossRateClimb,		//Controls the loss for each subsequent attack.
		//Shield & Item
		upkeepTime,
		raiseBaseCost,
		raiseWeightMult,
		regWeightMult,
		firstComboTime,				//The amount of time it takes for a combo to end after using this item
		penaltyComboTime,			//The amount of time for a combo to run out if you've used most of your stamina, magic should get probs
		weakGuardTime,
		//Melee
		firstBlowCost,
		staminaHitCostMult,
		followComboTime,			//The amount of time it takes for melee combos to end after following up with this item.
		firstBlowPowerRestore,
		swingPowerRestore,
		hitPowerRestore,
		powerAttackCostRate,		//Controls how much cheaper a power attack costs/requires to perform.
		firstBlowPowerCooldown,
		successPowerCooldown,
		failurePowerCooldown,
		//Shield & Melee
		blockResist,
		blockComboTime,
		weakBlockComboTime,
		dropGuardTime,
		exhaustGuardTime,
		minorUpkeepMult,
		majorUpkeepMult,
		unguardedPenalty,//Penalty for being not blocking a hit while guarding. May shift depending on item.
		//Race
		baseResist,
		baseEvade,
		basePoise,
		poiseHealthMult,
		poiseBonusHealthMult,
		//non-manual use
		concMagicPrecision,
		beamMagicPrecision,
		missileMagicPrecision,
		lobberMagicPrecision,
		coneMagicPrecision,
		arrowMagicPrecision
	};

	//Need an update to implement a few ideas.


	//need to update for: BashStagger, BashPrecision, BlockResist
	//Should I make stats for armors? So people can like, make their own special series of stats for different armor types
	// or make some armors better than others (if their like non-playable)



	//namespace Crucible
	//{

	class CombatData;
	class CombatDataBehaviour;
		
	template<class HeaderParent, class... SendArgs>
	class ITailNote;
		
	template<class HeaderParent, class... SendArgs>
	class IHeader;

	template<class HeaderParent, class... SendArgs>
	class INote;

	//This shit is going to need some sort of object to handle when it can load settings, there's a 
	// union for that. The ones created statically, load settings, and the ones that arent are just floats

	template<class HeaderParent, class... SendArgs>
	class IHeader
	{
	public:
		using HeaderType = IHeader<HeaderParent, SendArgs...>;
		using TailType = ITailNote<HeaderParent, SendArgs...>;
		using NoteType = INote<HeaderParent, SendArgs...>;


	private:
		//static inline std::unordered_map<HeaderType*, std::vector<TailType*>> pendingBehaviourList;
		static inline std::vector<HeaderType*> _headerList;
		static inline std::uint16_t _pendingCounter = 0;

		friend class TailType;
		friend class NoteType;

		inline void Register()
		{
			logger::info(">Header Detected:{}\n==========cccccccc====="sv, address());

			_pendingCounter++;
			//auto& just_initializing = pendingBehaviourList[this];
			_headerList.push_back(this);;

		}
	
	public:
		std::size_t _offsetDown;//I think this stores the offset of the very last object.
		

		

		void Unregister()
		{
			_pendingCounter--;
#define at_end
#ifdef at_end
			if (!_pendingCounter) {
				_headerList.clear();
			}

#else
			_headerList.erase(this);
#endif


			
		}


		IHeader()
		{
			Register();
		}
	

		std::uint64_t address() { return reinterpret_cast<std::uintptr_t>(this); }

		bool IsRelated(TailType* behaviour)
		{
			auto alt_size = behaviour->GetAltSize();
			std::uintptr_t head_addr = address();// - sizeof(HeadBehaviour);//Don't really need this last bit.
			std::uintptr_t be_addr = reinterpret_cast<std::uintptr_t>(behaviour);
			//auto corrective_offset = offsetof(HeaderParent, HeaderParent::header);
			std::uintptr_t range = head_addr + (alt_size ? alt_size : sizeof HeaderParent);

			logger::info(" -head:{} < behaviour:{} < end:{}"sv, head_addr, be_addr, range);

			auto result = head_addr < be_addr && be_addr < range;

			if (result)
				PlaceTailNote(behaviour);

			return result;
		}

		void PlaceTailNote(TailType* behaviour)
		{
			auto self_addr = address();

			std::uintptr_t focus_addr = self_addr + _offsetDown;

			std::uintptr_t tail_addr = reinterpret_cast<std::uintptr_t>(behaviour);

			behaviour->_offsetUp = tail_addr - focus_addr;
			_offsetDown = tail_addr - self_addr;
			logger::info("TEST: {}/{}, tail {},  up:{} down:{}",self_addr, focus_addr, tail_addr, behaviour->_offsetUp, _offsetDown);


		}

		void SendOwner(HeaderParent* parent, SendArgs... args) 
		{
			if (!_offsetDown) {
				logger::info("No offsets for HeaderNote to resolve.");
				Unregister();
				return;
			}

			auto self_addr = address();

			std::uintptr_t tail_query = self_addr + _offsetDown;
			logger::info("RUN: {}+{} = , tail {}", self_addr, _offsetDown, tail_query);
			while (tail_query != self_addr)
			{
				auto old = tail_query;
				auto* tail_ptr = reinterpret_cast<TailType*>(tail_query);
				tail_query -= tail_ptr->_offsetUp;
				
				logger::info("RUNNING: {} - {} = {}", old, tail_ptr->_offsetUp, tail_query);


				tail_ptr->OnSendOwner(parent, args...);

				logger::info("NEXT========================================");
			}

			Unregister();

#ifdef using_vector_maps_which_I_wont_be

			{
				auto& behaviour_entries = pendingBehaviourList[this];

				for (auto& behaviour : behaviour_entries) {
					behaviour->OnSendOwner(parent, args...);
				}
			}

			_pendingCounter--;

			if (!_pendingCounter) {
				pendingBehaviourList.clear();
			}
#endif 
		}

		void SendOwner(SendArgs... args)
		{
			auto* parent = GetHeaderParent();
			SendOwner(parent, args...);
		}

		HeaderParent* GetHeaderParent()
		{
			auto offset = offsetof(HeaderParent, HeaderParent::header);
			auto addr = address();
			auto ptr = reinterpret_cast<HeaderParent*>(addr - offset);
			
			return ptr;
		}
	};
	



	template<class HeaderParent, class... SendArgs>
	struct ITailNote
	{
	public:
		using HeaderType = IHeader<HeaderParent, SendArgs...>;
		using TailType = ITailNote<HeaderParent, SendArgs...>;
		using NoteType = INote<HeaderParent, SendArgs...>;

		friend HeaderType;
		friend NoteType;

		#define intialized 0xFFFF
	public:
		
		std::size_t _altSize;
		std::size_t _offsetUp;//Considering using this instead of the affor mentioned system.
	protected:

		std::uint64_t address() { return reinterpret_cast<std::uintptr_t>(this); }

		bool Initialized() { return _altSize == 0xFFFF; }
	public:
		void Init(bool use_init = false)
		{
			if (Initialized() == true)
				return;

			if (use_init)
			{
				bool should_stop = false;

				auto* note = GetNoteParent();
				note->OnInit(should_stop);
				
				if (should_stop)
					return;
			}
			
			logger::info("*Note:{}\n"sv, address());

			for (auto& entry : HeaderType::_headerList) {
				if (entry->IsRelated(this) == true) {

					OnHeaderMatch(entry);
					_altSize == 0xFFFF;

					logger::info(">Finish \n==========================================="sv);
					return;
				}
			}

		}


		std::size_t GetAltSize() { return Initialized() ? 0 : _altSize; }
		void		SetAltSize(std::size_t value) { _altSize = Initialized() ? _altSize : value; }
	protected:
		//void OnInit(){auto* note = GetNoteParent();note->OnInit();}

		ITailNote()
		{
			Init(true);
		}

		void OnHeaderMatch(HeaderType* header)
		{
			auto* note = GetNoteParent();
			note->OnHeaderMatch(header);
		}


		NoteType* GetNoteParent()
		{
			auto offset = offsetof(NoteType, NoteType::header);// / 8;
			auto ptr = reinterpret_cast<NoteType*>(address() - offset);
			
			return ptr;
		}


		void OnSendOwner(HeaderParent* parent, SendArgs... args) 
		{
			auto* note = GetNoteParent();
			note->OnSendOwner(parent, args...);
		}
	};
	
	//requires(std::derived_from<NewTailNote, ITailNote<HeaderParent, SendArgs...>>)

	template<class HeaderParent, class... SendArgs>
	//From now on, this is the abstract class.
	class INote
	{
	public:
		using HeaderType = IHeader<HeaderParent, SendArgs...>;
		using TailType = ITailNote<HeaderParent, SendArgs...>;
		using NoteType = INote<HeaderParent, SendArgs...>;

	protected:
		//The behaviour header, who's constructor places the behaviour in registry without the need to implement a constructor each interation
		// of the class.
		TailType header;

	protected:
		virtual void OnInit(bool& wait) {}
		
		virtual void OnSendOwner(HeaderParent* parent, SendArgs... args) { }

		virtual void OnHeaderMatch(HeaderType* header_obj) { }

		friend class TailType;
	};
	//I have to take care of my little siblings, so I have to carry them within
	//Similar to tail note, it's imperitive that this exists



	//if the rest doesn't work we do this.
	// Inote, final
	// tail note, branched. Used primarily for the purposes of a accessible pointer.
	// second tail note class, should use a helper object to perform the functions it wants to perform.

	//Helper clapss is designated by the I note.



	struct StatBase;

	using StatHeader = IHeader<StatBase, const char*, RE::GameSettingCollection*>;

	struct StatBase
	{
		StatHeader header;

		void SetName(const char* stat_name)
		{
			using func_t = void(*)();
			REL::Relocation<func_t> func{ REL::ID(10962) };
			func();

			auto* setting_collection = RE::GameSettingCollection::GetSingleton();

			//RE::GameSettingCollection::InitCollection();
			if (!setting_collection)
				return;//Do some shit then return.

			header.SendOwner(this, stat_name, setting_collection);
		}

		virtual float GetValue(StatType getStat);
	};

	


	class StatValue : public INote<StatBase, const char*, RE::GameSettingCollection*>
	{

		constexpr const char* EmptyString() { return ""; }
		constexpr const char* HandleString() { return "_"; }

	public:
		//All of these must have an underscore, seperating the names. If it doesn't have one, we haven't handled it yet.
		
		//using HeaderType = IHeader<Type>;

		//std::string name;
		//const char* name = nullptr;
		RE::Setting _valueSetting{ EmptyString(), 0.f };

		

		virtual void OnInit(bool& wait) { wait = true; }


		void OnSendOwner(StatBase* parent, const char* prefix_name, RE::GameSettingCollection* collection) override
		{
	

			logger::info("{} and {}", _valueSetting.name, prefix_name);
			std::string prep_name = std::format("f{}_{}", prefix_name, _valueSetting.name);

			//const char* final_name = new const char[](*prep_name.c_str());
			//const char* final_name = _strdup(prep_name.c_str());

			std::size_t prep_size = prep_name.size() + 1;
			char* final_name = new char[prep_size]();
			std::memcpy(final_name, prep_name.c_str(), prep_size);

			//Looks dangerous, keep a look out.
			//auto& first_char = final_name[1];
			//first_char = toupper(first_char);

			_valueSetting.name = final_name;		

			logger::info("{} vs {} vs {}", _valueSetting.name, final_name, prep_name);
			collection->InsertSetting(&_valueSetting);
		}


		template <class MType>//requires(std::is_base_of< StatBase, MType>::value)
		StatValue(MType type, const char* a_name, float starting_value)
		{
			//Unsure how inforcable this is.
			//static_assert(a_name != nullptr && a_name[0] == NULL, "Accepted name for cannot be null.");
			header.SetAltSize(sizeof *type);
			_valueSetting.name = a_name;
			_valueSetting.data.f = starting_value;
			header.Init();
		}

		~StatValue()
		{
			//This is going to have to delete the string that in setting if it's going away, because that's
			// going to have to be created with a new string
			if (_valueSetting.name && strstr(_valueSetting.name, HandleString()) != nullptr){
				auto* ptr = _valueSetting.name;
				_valueSetting.name = nullptr;
				delete (ptr);
			}
		}

#pragma region Constructors_And_Assignment
		//SerializingWrapper() = default;

		//Copy Constructor
		//SerializingWrapper(WrapType & other) { _wrapObject = other; }

		//Copy Constructor
		//SerializingWrapper(WrapType other) { _wrapObject = other; }

		//Copy Assignment
		template<std::convertible_to<float> ValueType>
		StatValue& operator=(ValueType& a_rhs) { _valueSetting.data.f = a_rhs;  return *this; }

		//Copy Assignment with loose
		//SerializingWrapper& operator=(WrapType a_rhs) { _wrapObject = a_rhs; }

		//Explicit move assignment
		template<std::convertible_to<float> ValueType>
		StatValue& operator=(ValueType&& a_rhs) { _valueSetting.data.f = a_rhs; return *this; }

		// implicit conversion
		operator float() const { return _valueSetting.data.f; }
		//operator WrapType&() const { return _wrapObject; }

		//explicit conversion
		explicit operator float* const () { return (&_valueSetting.data.f); }

		constexpr RE::Setting* operator->() { return &_valueSetting; }

		constexpr RE::Setting* operator*() { return &_valueSetting; }
#pragma endregion
	};
	
//Binds a stat value in a manner that will automatically update the name.
#define BIND_VALUE(a_variable, a_value) a_variable = { this, #a_variable##, a_value }

	struct MinimumStat : public StatBase
	{
		StatValue BIND_VALUE(baseStagger, 1);//float baseStagger = 1;
		StatValue BIND_VALUE(guardDamageMult, 1);//float guardDamageMult = 1;
		
		StatValue BIND_VALUE(specialValue, 1);//float guardDamageMult = 1;
		

		float GetValue(StatType getStat) override;
	};

	struct BasicStat : public MinimumStat
	{
	public:
		StatValue BIND_VALUE(basePrecision, 1);//float basePrecision = 0;

		//
		

		float GetValue(StatType getStat) override;
	};
	
	//^At a later point there's likely going to be a branch class on this for shields for their specific stats.

	//I need this shit to have much of the functionality that weapons do, as a matter of fact.
	// Nothing like a climb factor though, it doesn't need it.
	// A notice for the cooldown type of thing, this things power attack cooldown should be low.
	//It should get 1 power attack cooldown value, perhaps if it gets the hit it should be really
	// low or none existent. Make sure the cooldown doesn't bleed into regular power attacks.
	struct ShieldStat : public BasicStat
	{
		StatValue BIND_VALUE(blockResist, 3);//float blockResist = 3;
		//float GetValue(StatType getStat) override;//For now this has nothing, so it won't be using this.

		StatValue BIND_VALUE(regWeightMult, 0.5f);//float regWeightMult = 0.5f;
		StatValue BIND_VALUE(powWeightMult, 1);//float powWeightMult = 1;

		StatValue BIND_VALUE(baseStagger, 2);//float blockUpMult = 2;


		StatValue BIND_VALUE(upkeepTime, 4);//float upkeepTime = 0.25f;

		StatValue BIND_VALUE(minorUpkeepMult, 0.5f);//float blockMinorUpkeepMult = 0.25f;
		StatValue BIND_VALUE(majorUpkeepMult, 1.f);//float blockMajorUpkeepMult = 0.25f;

		StatValue BIND_VALUE(unguardedPenalty, 2);



		StatValue BIND_VALUE(blockComboTime, 1.5f);
		StatValue BIND_VALUE(weakBlockComboTime, 2.5f);
		StatValue BIND_VALUE(weakGuardTime, 4.f);
		StatValue BIND_VALUE(dropGuardTime, 1.f);
		StatValue BIND_VALUE(exhaustGuardTime, 2.5f);
		StatValue BIND_VALUE(raiseBaseCost, 15);
		StatValue BIND_VALUE(raiseWeightMult, 0.5f);



		StatValue BIND_VALUE(counterBashStagger, 2);
		StatValue BIND_VALUE(counterBashDamageMult, 15.f);//0.25, 2 for testing.
		StatValue BIND_VALUE(counterBashRevengeDamageMult, 25.f);//0.75
		StatValue BIND_VALUE(counterBashStaminaBase, 10);
		StatValue BIND_VALUE(counterBashStaminaMult, 0.5f);
		StatValue BIND_VALUE(counterBashHealthReq, 0.15f);
		StatValue BIND_VALUE(counterBashCountReq, 3);
		StatValue BIND_VALUE(counterHitDecay, 5);
		StatValue BIND_VALUE(strongCounterHitDecay, 10);
		StatValue BIND_VALUE(strongGuardRecovery, 5);



		float GetValue(StatType getStat) override;
	};


	struct BashStat : public BasicStat
	{
		StatValue BIND_VALUE(bashStagger, 1);//float bashStagger = 1;
		StatValue BIND_VALUE(bashPrecision, 3);//float bashPrecision = 4;//For now, this will just beat out everyone if not dodging. Busted number was 8.

		StatValue BIND_VALUE(regBashWeightMult, 0.5f);
		StatValue BIND_VALUE(powBashWeightMult, 0.75f);


		float GetValue(StatType getStat) override;
	};

	struct ItemStat : public BashStat
	{
	public:
		StatValue BIND_VALUE(baseStaminaCost, 5);//float baseStaminaCost = 5;
		StatValue BIND_VALUE(staminaCostClimb, 1);//float staminaCostClimb = 1;

		StatValue BIND_VALUE(staminaLossRateClimb, 15);//float staminaLossRateClimb = 15;
		StatValue BIND_VALUE(firstComboTime, 0.3f);//float firstComboTime = 0.3f;
		StatValue BIND_VALUE(penaltyComboTime, 3);//float penaltyComboTime = 3.f;//Should you expend too much stamina, this is how long you must wait for the combo to end.
		StatValue BIND_VALUE(regWeightMult, 1);//float regWeightMult = 1;

		float GetValue(StatType getStat) override;
	};

	struct MeleeStat : public ItemStat
	{
	public:
		StatValue BIND_VALUE(firstBlowCost, 2.5f);//float firstBlowCost = 2.5f;
		StatValue BIND_VALUE(staminaHitCostMult, 0.5f);//float staminaHitCostMult = 0.5f;

		StatValue BIND_VALUE(powerAttackCostRate, 10);//float powerAttackCostRate = 10;//For now, unused.

		//power attack restore on attack
		StatValue BIND_VALUE(firstBlowPowerRestore, 2);//float firstBlowPowerRestore = 2;
		StatValue BIND_VALUE(swingPowerRestore, 5);//float swingPowerRestore = 5;
		StatValue BIND_VALUE(hitPowerRestore, 8);//float hitPowerRestore = 8;

		
		StatValue BIND_VALUE(followComboTime, 1.55f);//float followComboTime = 1.55f;



		//Cooldown on power attack
		StatValue BIND_VALUE(firstBlowPowerCooldown, 15);//float firstBlowPowerCooldown = 15;
		StatValue BIND_VALUE(successPowerCooldown, 10);//float successPowerCooldown = 10;
		StatValue BIND_VALUE(failurePowerCooldown, 20);//float failurePowerCooldown = 20;
		
		StatValue BIND_VALUE(blockResist, 1);//float blockResist = 1;

		StatValue BIND_VALUE(minorUpkeepMult, 0.1f);
		StatValue BIND_VALUE(majorUpkeepMult, 0.05f);

		StatValue BIND_VALUE(unguardedPenalty, 1);


		StatValue BIND_VALUE(upkeepTime, 1);//float upkeepTime = 0.25f;


		StatValue BIND_VALUE(weakGuardTime, 5);
		StatValue BIND_VALUE(blockComboTime, 2);
		StatValue BIND_VALUE(weakBlockComboTime, 2.5f);
		StatValue BIND_VALUE(dropGuardTime, 2.f);
		StatValue BIND_VALUE(exhaustGuardTime, 1.5f);
		StatValue BIND_VALUE(raiseBaseCost, 0);
		StatValue BIND_VALUE(raiseWeightMult, 0.0f);




		//For the items associated with these, these will also have to have verious safety settings for things like length

		float GetValue(StatType getStat) override;
	};

	struct RaceStat : public MeleeStat
	{
		//Experimental idea for non-character actors to determine not only their innate states for stagger and precision
		// NPCs have no innate bonuses to anything, while anything that's deemed a creature race will have strong innate
		// settings.

	public:
		StatValue BIND_VALUE(baseResist, 0);//float baseResist = 0;
		StatValue BIND_VALUE(baseEvade, 3);//float baseEvade = 3;//

		StatValue BIND_VALUE(basePoise, 100);//float basePoise = 100;//This doesn't grow mind you, this is only handled for regular, non-effective attacks.
		StatValue BIND_VALUE(poiseHealthMult, 0.1f);//float poiseHealthMult = 0.1f;//This one grows with the more max health you've got.
		StatValue BIND_VALUE(poiseBonusHealthMult, 0.25f);//float poiseBonusHealthMult = 0.25f;
		//Other stats that should be included here would be stuff like power attack cooldown mults IF I do them
		// and penalty cooldown mults and such
		//Addition values because creatures like giants and falmer use weapons, is mults that control how some of the below things are effected in the case 
		// they aren't using their fists, and instead are using weapon or spell. 
		float GetValue(StatType getStat) override;
	};

	struct RangedStat : public ItemStat
	{
		float GetValue(StatType getStat) override;
	};

	struct MagicSpellStat : public MinimumStat
	{
		//These hold the stats for a magic spell, primarily associated with the school it belongs to.

		//For now, you're gonna see nothing. But at a later point it will hold things such as dual casting information.

		float GetValue(StatType getStat) override;
	};


	struct MagicProjStat : public StatBase
	{
		//This singleton object stores the precision and other (if any others that is) stats
		// associated with each type of magical projectile.

		StatValue BIND_VALUE(concMagicPrecision, 0);//float concMagicPrecision;
		StatValue BIND_VALUE(beamMagicPrecision, 0);//float beamMagicPrecision;
		StatValue BIND_VALUE(missileMagicPrecision, 0);//float missileMagicPrecision;
		StatValue BIND_VALUE(coneMagicPrecision, 0);//float coneMagicPrecision;
		StatValue BIND_VALUE(arrowMagicPrecision, 0);//float arrowMagicPrecision;
		StatValue BIND_VALUE(lobberMagicPrecision, 0);//float lobberMagicPrecision;

		//May include other types,
		// explosion
		// hazard
		float GetValue(StatType getStat) override;
	};

	//using StatObject = std::derived_from<ItemStat>;




	//We need the container, generic. 
	// This is comprised of 2 types, one that owns its dictionary, and one that shares it.

	//Next, we need an object that gets put in a vector


	class StatCondition
	{
		//Interface class, this thing is slated to have 
		// a few different versions of this function.
		//It should have:
		// HasKeyword, holding a keyword
		// IsName, holding a string and comparing the name
		// IsEditor, comparing editor names (would need po3 Tweakths)
		// And finally IsID which compares form ids, should come with esp specification.
	public:
		virtual bool IsMatchingForm(RE::TESForm* form) = 0;
	};

	template<class StatObject>
	class ConditionalStat
	{
	protected:
		std::uint8_t _priority;
		StatObject _statObject;
		StatCondition* _primaryCondition;
	public:

		constexpr auto GetPriority() noexcept { return _priority; }
		constexpr StatBase* GetItemStat() noexcept { return &_statObject; }
		bool IsMatchingForm(RE::TESForm* form) { return _primaryCondition ? _primaryCondition->IsMatchingForm(form) : false; }
		void SetPrimaryCondition(StatCondition* condition) { _primaryCondition = condition; }


	};


	//Restructure this to use a list, vectors are better types for appending stuff, but not insertions.`
	template<class StatObject>
	class StatContainerBase
	{
		using ExtraStat = ConditionalStat<StatObject>;
		using ExtraStatContainer = std::vector<ExtraStat*>;

	public:
		StatObject defaultStat;


	protected:

	public:

		//constexpr StatObject* operator->() noexcept { return &defaultStat; }
		//constexpr StatObject* operator*() noexcept { return &defaultStat; }

		//Don't know why this doesn't work, probably do not fucking need.
		//constexpr StatObject* operator->() noexcept { return &defaultStat; }//Stat base was stat object. Doesn't need to be.
		//constexpr StatContainerBase* GetPtr() noexcept { return this; }
		//*
		void AddExtraStat(ExtraStat* stat)
		{
			//handle, do binary search on where to place this shit, just in case.
			//For now, just put it in back.

			//cpp this please thx
			auto container = GetContainer();

			if (!container)
				return;

			auto insert_at = GetLatestPriority(stat->GetPriority());

			if (insert_at == -1 || insert_at >= container->size())
				container->push_back(stat);
			else
			{
				auto it = container->begin();
				container->insert(std::next(it, insert_at), stat);//it = 
			}
		}


		//Set this to StatObject once the constraint actually starts working.
		StatBase* GetStatBase(RE::TESForm* form)
		{
			auto container = GetContainer();

			if (container && container->size() != 0){
				for (auto& extra_stat : *container) {
					if (extra_stat->IsMatchingForm(form) == true)
						return extra_stat->GetItemStat();
				}
			}
			
			return &defaultStat;
		}

		float GetStatValue(RE::TESForm* form, StatType type)
		{
			auto stat_base = GetStatBase(form);

			if (!stat_base)
				return 0.f;//or was it negative 1?

			return stat_base->GetValue(type);
		}

	protected:
		virtual ExtraStatContainer* GetContainer() = 0;
		constexpr ExtraStatContainer& GetContRef() { return *GetContainer(); }

	private:

		int HalfSize(int size)
		{
			auto base_index = size * 0.5f;

			//if (ceil)
			//	base_index = ceilf(base_index);
			//else
			//	base_index = floorf(base_index);

			base_index = ceilf(base_index);


			return static_cast<int>(base_index);
		}




		int GetPriorityInRange(int priority, int begin, int range, int size, int dead_zone)
		{
			int last_index = size - 1;

			//might redesign this recursion, i doubt it will but it might cause stack overflow

			//it would seem this returns begin + 1, but what if it wants to insert at 0?


			//I think I will test the largest and the smallest so I don't have to go through the entire thing

			//0 - 10
			//looking for zero
			//0, r5 = 5
			// within, 0, r3 = 3
			// within 0, r2 = 2
			// within 0, r1 = 1
			// within 0, r0 defaults to range.
			// exits out on 0 + 1, see issue?
			// 
			// 
			//0 - 10
			//looking for 10
			//0, r5 = 5
			// not within, goto 5, r3 = 8
			// not within, goto 8, r2 = 10
			// is, go 8+2, 10 - 10 (0), 10, 10
			//range is 0 places as 10.

			//0 - 10
			//looking for 10
			//0, r5 = 5
			// not within, goto 6, r2 = 8
			// not within, goto 9, r1 = 10
			// is, go 8+2, 10 - 10 (0), 10, 10
			//range is 0 places as 10.


			//0 - 14
			//looking for 14
			//0, r8 = 8
			// not within, goto 9, r4 = 13
			// not within, goto 14, r(2)1 = 15
			// is go 14
			// is, go 8+2, 10 - 10 (0), 10, 10
			//range is 0 places as 10.


			//0 - 10
			//looking for 8
			//0, r6 = 6
			// without, 7, r3 = 10
			//within


			//0 - 10
			//looking for 2
			//0, r6 = 6
			// within, 0, r3 = 3
			//within, 0, r2 = 2
			//is with, 2, 0 = 2

			auto stat_priority = GetContRef()[begin + range].GetPriority();


			if (!range) {
				if (stat_priority >= priority)
					return begin + 1;

				return begin;//fmin(begin + 1, size);//begin + 1;//

			}


			//Raw using begin has issues putting something in dead last. EXACTLY as I thought.


			if (stat_priority > priority)//Explore end side
			{
				//crashed inconsistently when this was max, ie always the last index, should that have even happened? Current crash may not be
				// begin based anymore.
				auto new_begin = __min(begin + range, last_index);
				auto new_range = range == 1 ? 0 : HalfSize(range);

				//Somehow, this causes major issues. No idea how.
				//auto new_begin = __max(begin + range + 1, last_index);//begin + range;//
				//auto new_range = (range == 1 ? 0 : HalfSize(range, false));//HalfSize(range, false);//range;//

				// 15, 0
				new_range = new_range + new_begin >= size ? (last_index)-new_begin : new_range;
				//new_range = new_range + new_begin > size ? size - new_begin : new_range;

				return GetPriorityInRange(priority, new_begin, new_range, size, dead_zone);
			}
			if (stat_priority < priority)//Explore beginning side
			{
				auto new_dead_zone = (begin + range) - 1;
				auto new_range = range == 1 ? 0 : HalfSize(range);// 0;//

				//new_range = new_range + new_begin >= size ? (size - 1) - new_begin : new_range;

				return GetPriorityInRange(priority, begin, new_range, size, new_dead_zone);
			}
			else//explore down, for an equal priority, if priority is diffent than stat, return the previous one.
			{
				//possible stack overflow here.

				//must be 60

				//15
				//20

				//range 5
				//return begin + range;//Testing to see if this fixes all crashing, then we proceed from there to fix the below.
				auto new_begin = (begin + range);//maybe this was the issue?
				auto new_range = __max(dead_zone - new_begin, 0);

				return GetPriorityInRange(priority, new_begin, new_range, size, dead_zone);
			}

		}

		int GetLatestPriority(int priority)
		{
			int size = GetContRef().size();

			if (!size)
				return -1;//When it returns -1, it will just push back.

			int range = __max(HalfSize(size), size - 1);
			//Need to make sure this actually works.

			return GetPriorityInRange(priority, 0, range, size, size - 1);

		}

		//*/
	};

	//These have no v func, it mighty be a good idea to have a shared stat container central class. edit: nvm

	template<class StatObject>
	class StatContainer : public StatContainerBase<StatObject>
	{
		using ExtraStat = ConditionalStat<StatObject>;
		using ExtraStatContainer = std::vector<ExtraStat*>;

		ExtraStatContainer _container;

	public:
		
		StatContainer(const char* stat_name) { this->defaultStat.SetName(stat_name); }
	private:
		ExtraStatContainer* GetContainer() override { return &_container; }

	};
	template<class StatObject>
	class SharedStatContainer;

	template<typename Type, typename TemplateType>
	concept SharableContainer = true;//requires(Type<TemplateType> a)
	//{
	//	true;//std::derived_from<Type, SharedStatContainer<TemplateType>>::value;
	//};

	template<class StatObject>
	class SharedStatContainer : public StatContainerBase<StatObject>
	{
		using ExtraStat = ConditionalStat<StatObject>;
		using ExtraStatContainer = std::vector<ExtraStat*>;


		//This is used for objects like magic or race, who's conditionals they will share instead of different branches for each.

		ExtraStatContainer* _sharedContainer;

		//This static method should the primary way this is used from now on, it makes it automatic via constructor how this is handled.
		static ExtraStatContainer* _staticSharedContainer;

		static constexpr uintptr_t noQueueValue =		0x0000000000000000;
		static constexpr uintptr_t singleQueueValue =	0xFFFFFFFFFFFFFFFF;

		ExtraStatContainer* CreateStaticContainer() { return new ExtraStatContainer(); }


		void QueueSharedContainer()
		{
			uintptr_t shared_value = static_cast<uintptr_t>(_staticSharedContainer);

			switch (shared_value)
			{
			case noQueueValue:
				_staticSharedContainer = static_cast<ExtraStatContainer*>(singleQueueValue);
			case singleQueueValue:
				_staticSharedContainer = CreateStaticContainer();
			}
		}

		ExtraStatContainer* GetSharedContainer()
		{
			uintptr_t shared_value = static_cast<uintptr_t>(_staticSharedContainer);

			switch (shared_value)
			{
			case noQueueValue:
			case singleQueueValue:
				return nullptr;

			default:
				return _staticSharedContainer;
			}
		}


	private:
		ExtraStatContainer* GetContainer() override { return _sharedContainer; }
		
	//public:
		void SetContainer(ExtraStatContainer* new_container) { _sharedContainer = new_container; }
		ExtraStatContainer* CreateContainer() { return new ExtraStatContainer(); }

	public:
		//Turn this over to item stat handler.
		template <class... SharableContainer>
		static void ShareContainer(SharableContainer&... containers)
		{
			std::vector sharing_containers{ &containers... };

			if (sharing_containers.size() == 0)
				return;

			ExtraStatContainer* new_shared_cont = new ExtraStatContainer();

			for (auto& container : sharing_containers)
			{
				if (container->_sharedContainer)
					delete container->_sharedContainer;//Or skip?

				container->_sharedContainer = new_shared_cont;
			}
		}

		SharedStatContainer(const char* stat_name) { this->defaultStat.SetName(stat_name); }
	};



	struct SharedContainer//A dummy class that is used as a centralized place to hold share container functions.
	{
		//using ExtraStat = ConditionalStat<StatObject>;
		//using ExtraStatContainer = std::vector<ExtraStat*>;


		//template<class T, class U>
		//concept Derived = std::is_base_of<U, T>::value;

		//template<Derived<Base> T>
		//void f(T); // T is constrained by Derived<T, Base>

		//Turn this over to item stat handler.
		
		/*
		template <SharableContainer<Base>... T>
		static void ShareContainer(T&... containers)
		{
			std::vector sharing_containers{ &containers... };

			if (sharing_containers.size() == 0)
				return;

			//ExtraStatContainer* new_shared_cont = new ExtraStatContainer();
			auto* new_shared_cont = new std::vector<ConditionalStat<StatObject>*>();

			for (auto& container : sharing_containers)
			{
				if (container->_sharedContainer)
					delete container->_sharedContainer;//Or skip?

				container->_sharedContainer = new_shared_cont;
			}
		}


		static void test()

		{
			ShareContainer(3, 3, 3, 3, 3);
		}
		//*/
	};



	//inline SharedStatContainer<MeleeStat> meleeStateth;
	//inline SharedStatContainer<MeleeStat> otherMeleeStateth;
	//inline SharedStatContainer<MeleeStat> thirdMeleeStateth;
	//inline SharedStatContainer<MeleeStat> finalMeleeStateth;


}