#pragma once
#include "Utility.h"
#include <xbyak/xbyak.h>



namespace Crucible
{
	//class bhkSimpleShapePhantom;//This is likely not the hit target, it's likely the arrow itself

	//Want a hook into this, to measure burst damage
	// virtual float CheckClampDamageModifier(ActorValue a_av, float a_delta);// 127
	// note, might be better to just check health before and after relevant factors
	// such as hits and projectile collisions

	/*
	template <std::size_t N>
	struct thunk_struct
	{
		static void thunk()
		{
			return func();
		}

		static inline REL::Relocation<decltype(thunk)> func[N];
	};
	//*/


	class CrucibleHook
	{
		//Use AttackData and AttackDataMap in order to get valuable information about how we should stagger and what move
		//Is being used at the moment
	public:

		using AnimEvent = RE::BSAnimationGraphEvent;
		using AnimSink = RE::BSTEventSink<AnimEvent>;
		using AnimSource = RE::BSTEventSource<AnimEvent>;

		/*
		static void TestPatch()
		{  // test
			auto hook_addr = REL::ID(37650).address() + 0x1AA;
			auto return_addr = hook_addr + 0x6;
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t func, uintptr_t ret_addr)
				{
					//sub(rsp, 0x20);
					mov(rax, func);
					call(rax);
					//add(rsp, 0x20);

					mov(rax, qword[rbx]);
					mov(rcx, ret_addr);
					jmp(rcx);
				}
			} code{ (uintptr_t)SetAV_PowerAttackHook, return_addr };
			auto size = code.getSize();
			auto& trampoline = SKSE::GetTrampoline();
			auto result = trampoline.allocate(size);
			std::memcpy(result, code.getCode(), size);
			_SetAV_PowerAttackHook = trampoline.write_branch<6>(hook_addr, (std::uintptr_t)result);
		}
		//*/
		/*
		static void TestPatch2()
		{
			
			
			//auto hook_addr = REL::ID(37938/*637a80/).address();
			auto return_addr = hook_addr + 0x9;
			/*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t _func, uintptr_t ret_addr)
				{
					mov(rax, _func);
					jmp(rax);

					/*
					mov(rax, _func);
					call(rax);

					test(rax, rax);

					jz("ret_loc");

					
					
					//jmp(_func);
					///sub(rsp, 0x20);
					//mov(rax, _func);
					//call(rax);
					
					///add(rsp, 0x20);

					//mov(rax, qword[rbx]);
				
					mov(rax, ret_addr);
					jmp(rax);
				ret_loc:
					ret();
					//*
				}
			} code{ (uintptr_t)thunk, return_addr };
			auto size = code.getSize();
			auto& trampoline = SKSE::GetTrampoline();
			auto result = trampoline.allocate(size);
			std::memcpy(result, code.getCode(), size);
			//trampoline.write_branch<5>(hook_addr, (std::uintptr_t)result);
			//func = return_addr;
			

			auto& trampoline = SKSE::GetTrampoline();
			trampoline.write_branch<5>(hook_addr, (uintptr_t)thunk);
			func = return_addr;
			
			auto place_query = trampoline.write_branch<5>(hook_addr, (uintptr_t)thunk2);

			if (place_query == hook_addr)
				func2 = return_addr;
			else
				func2 = place_query;
		}
		//*/

		static void TestPatch3()
		{
			//This hook doesn't really work, I'm gonna have to rely on get dead
			// THAT, or there's a function I can likely jump to here, I can fullfill the rdi test myself,
			// and then try to make a different measure of testing, one that I will jump over test rdi and go straight to the jump

			auto hook_addr = REL::ID(37633/*0x626400*/).address() + 0x886;//0x96F;
			auto return_addr = hook_addr + 0x5;
			/*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t _func, uintptr_t ret_addr)
				{


					//mov(rax, _func);
					//jmp(rax);

					//*
					//My only method to get these to work is xbyak, what I'll likely need to do is use my own function much in the way the patch does,
					// I'll write in a call to a function that never existed right before.
					// the thing written in willl be, the rearrangement of rcx and rdx to have both actors within, then it will use test 
					// on rax. The function I have will return if rdx is empty, as it originally intended.
					// BE VERY FUCKING SURE not to change the rdx register, as that is used later in a call, and it's assumed to be there.
					// Finally, right after we test the rax pointer, we jump to 884 as the return address, processing it's conditional jump,
					// and resuming functionality. I FUCKING HOPE.
					//To reiterate where we start, we hook into the test right before the condition jump that starts the recoil song and dance.	
					mov(rcx, rdi);
					push(rax);
					
					mov(rax, _func);
					call(rax);//This call, it could dement what is in the rdx address, for this purpose, I think I want to preserve it.
					mov(rdi, rcx);

					test(rax, rax);

					pop(rax);

					mov(rcx, ret_addr);
					jmp(rcx);
					
				}
			} code{ (uintptr_t)RecoilTest, return_addr };
			auto size = code.getSize();
			auto& trampoline = SKSE::GetTrampoline();

			auto result = trampoline.allocate(size);
			std::memcpy(result, code.getCode(), size);
			trampoline.write_branch<5>(hook_addr, (std::uintptr_t)result);
			//*/
		}
		
		/*
		template <class T>
		void write_jump(std::uintptr_t a_src)
		{
			AbsoluteJump patch;
			patch.absolute64 = std::addressof(T::thunk);
			REL::safe_write(a_src, &patch, sizeof(AbsoluteJump));
		}
		//*/
		//*
		template<class T>
		bool ArrayCheck(T& value)
		{
			return std::is_array<T>::value;
		}

		template <class T>
		void write_branch(std::uintptr_t a_src, std::uintptr_t ret_add = 0, int index = 0)
		{
			//should come with a size too. As well as a return address, which specifies what to set the func as.
			// this is optional for write branch only.
			auto& trampoline = SKSE::GetTrampoline();

			auto query = trampoline.write_branch<5>(a_src, (uintptr_t)T::thunk);

			auto& go_to = 0 <= index && ArrayCheck(T::func) ? T::func[index] : T::func;

			if (query == a_src)
				go_to = ret_add;
			else
				go_to = query;
		}

		template <class T>
		void write_vfunc(std::uintptr_t a_src, std::uintptr_t ret_add = 0, int index = 0)
		{
			//should come with a size too. As well as a return address, which specifies what to set the func as.
			// this is optional for write branch only.
			auto& trampoline = SKSE::GetTrampoline();

			auto query = trampoline.write_branch<5>(a_src, (uintptr_t)T::thunk);

			auto& go_to = 0 <= index && ArrayCheck(T::func) ? T::func[index] : T::func;

			if (query == a_src)
				go_to = ret_add;
			else
				go_to = query;
		}


		//stl::write_jump<WriteScreenshot>(WriteScreenshot_hook.address());
		/*
		template <std::size_t N, class... Args>
		struct thunk_struct
		{
			static void thunk()
			{
				return func();
			}

			static inline REL::Relocation<decltype(thunk)> func[N];
		};
		//int32 __fastcall Character::DoRecoil_1405FA080(Character* a3, bool isTimed)
		

		
		//*/
		
		struct BlockHitHook
		{
			static std::int32_t thunk(RE::Actor* actor);

			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct TestFistUpdateHook
		{
			//This hook is so ununique btw, that I think I can just write branch this shit. Straight up.
			static std::int64_t thunk(RE::ActorValueOwner* av_owner, RE::TESObjectWEAP* weap, std::int64_t unk3)
			{
				if (!weap)
					weap = RE::TESForm::LookupByID<RE::TESObjectWEAP>(0x1F4);

				return func(av_owner, weap, unk3);
			}

			static inline REL::Relocation<decltype(thunk)> func;
		};


		struct TestApplyCombatHitHook
		{
			//This hook is so ununique btw, that I think I can just write branch this shit. Straight up.
			static void thunk(RE::Actor* a_this, RE::InventoryEntryData* entry_data, bool is_left, RE::Actor* target)
			{
				if (!entry_data)
				{
					auto* weap = RE::TESForm::LookupByID<RE::TESBoundObject>(0x1F4);
					
					//RE::ApplyPerkEntries(uEntryPoint::kApplyCombatHitSpell, a_this, weap, target);
					auto data = new RE::InventoryEntryData(weap, 1);
					func(a_this, data, is_left, target);
					delete data;
					return;
				}
				
				func(a_this, entry_data, is_left, target);
			}

			static inline REL::Relocation<decltype(thunk)> func;
		};


		struct AllowIdleHook
		{
			//This hook is so ununique btw, that I think I can just write branch this shit. Straight up.
			static RE::TESIdleForm* thunk(RE::TESIdleForm* a_this, RE::ConditionCheckParams* params, void* unk3);

			static inline REL::Relocation<decltype(thunk)> func;
		};


		struct TestNoMarkerHook
		{
			using uMapMarkerData = RE::MapMarkerData;

			static uMapMarkerData* thunk(RE::TESObjectREFR* refr)
			{
				return nullptr;
			}

			static inline REL::Relocation<decltype(thunk)> func;
		};
		
		/*
		struct RegularRecoilHook
		{
			static std::int32_t thunk(RE::Actor* actor, bool isTimed)
			{
				Utility::DebugMessageBox("Regular Recoil")
				return func();
			}

			static inline REL::Relocation<decltype(thunk)> func;
		};
		//*/

		struct TimedRecoilHook
		{
			static std::int32_t thunk(RE::Actor* actor, bool is_timed)
			{
				if (!is_timed)
				return func(actor, is_timed);
			}

			static inline REL::Relocation<decltype(thunk)> func;
		};




		static void Install()
		{
			SKSE::AllocTrampoline(256);

			REL::Relocation<std::uintptr_t> Player_Main_Vtbl{ REL::ID(261916) };
			REL::Relocation<std::uintptr_t> Character_Main_Vtbl{ REL::ID(261397) };

			REL::Relocation<std::uintptr_t> Actor_Main_Vtbl{ REL::ID(260538) };


			REL::Relocation<std::uintptr_t> Player_AnimationGraphManagerHolder_Vtbl{ REL::ID(261919) };
			REL::Relocation<std::uintptr_t> Character_AnimationGraphManagerHolder_Vtbl{ REL::ID(261400) };


			REL::Relocation<std::uintptr_t> Player_GraphEvent_Vtbl{ REL::ID(261918) };
			REL::Relocation<std::uintptr_t> Character_GraphEvent_Vtbl{ REL::ID(261399) };

			REL::Relocation<std::uintptr_t> ActorMagicCaster_MagicCaster_Vtbl{ REL::ID(257613) };


			//*These function writes weren't bad, but I was likely getting into the wrong place. Try this again at some point.
			//REL::Relocation<std::uintptr_t> Player_ActorState_Vtbl{ REL::ID(261922) };
			//REL::Relocation<std::uintptr_t> Character_ActorState_Vtbl{ REL::ID(261403) };

			REL::Relocation<std::uintptr_t> HitCollisionHook{ REL::ID(37650/*627930*/) };
			

			REL::Relocation<std::uintptr_t> hitEventHook{ REL::ID(37673/*628c20*/) };//courtesy of stagger on hit from loki
			//REL::Relocation<std::uintptr_t> hitProjEventHook{ REL::ID(43013)/*7521F0*/ };
			REL::Relocation<std::uintptr_t> hitProjEventHook{ REL::ID(43015)/*7528E0*/ };
			


			REL::Relocation<std::uintptr_t> missileVtbl{ REL::ID(263942) };
			REL::Relocation<std::uintptr_t> arrowVtbl{ REL::ID(263776) };
			REL::Relocation<std::uintptr_t> coneVtbl{ REL::ID(263822) };
			//REL::Relocation<std::uintptr_t> flameVtbl{ REL::ID(263884) };
			//REL::Relocation<std::uintptr_t> beamVtbl{ REL::ID(263808) };


			REL::Relocation<uintptr_t> mainHook{ REL::ID(35551) }; // 5AF3D0, courtesy of TDM from Ersh

			



			//May not need this anymore.
			_ProjectileCollision_Missile = missileVtbl.write_vfunc(0xBE, ProjectileCollision);
			_ProjectileCollision_Arrow = arrowVtbl.write_vfunc(0xBE, ProjectileCollision);;
			//_ProjectileCollision_Beam = beamVtbl.write_vfunc(0xBE, ProjectileCollision);;
			//_ProjectileCollision_Flame = flameVtbl.write_vfunc(0xBE, ProjectileCollision);;
			_ProjectileCollision_Cone = coneVtbl.write_vfunc(0xBE, ProjectileCollision);;
			//beamVtbl.write_vfunc(0xB7, Func183);

			_ProcessPlayerAnim = Player_GraphEvent_Vtbl.write_vfunc(0x1, ProcessPlayerAnim);
			_ProcessCharacterAnim = Character_GraphEvent_Vtbl.write_vfunc(0x1, ProcessCharacterAnim);
			//_UpdateAnimation = Player_Main_Vtbl.write_vfunc(0x7D, UpdateAnimation);

			//RAISHA
			//_Func29 = ActorMagicCaster_MagicCaster_Vtbl.write_vfunc(0x1D, Func29);

			_NotifyPlayerAnimationGraph = Player_AnimationGraphManagerHolder_Vtbl.write_vfunc(0x1, NotifyAnimationGraph);
			_NotifyCharacterAnimationGraph = Character_AnimationGraphManagerHolder_Vtbl.write_vfunc(0x1, NotifyAnimationGraph);

			auto& trampoline = SKSE::GetTrampoline();
			
			//This is our bread and butter right here, override this in the right places, and we have our ticket for power attack stamina draining.
			//void __fastcall ActorValueOwner::sub_1403BEC90(ActorValueOwner * a1, BGSAttackData * a2)
			// If we use this, we can actually override and apply this to when the stamina drain would
			// normally occur.

			//would like to make these vtable calls.
			_ProcessHitEvent = trampoline.write_call<5>(hitEventHook.address() + 0x3C0, ProcessHitEvent);
			o__TestA = trampoline.write_call<5>(hitProjEventHook.address() + 0x216, TestA);

			//Testing sweep
			REL::Relocation<std::uintptr_t> HitCollisionHook2{ REL::ID(37689/*62B870*/) };


			_OnMeleeHit = trampoline.write_call<5>(HitCollisionHook.address() + 0x38B, OnMeleeHit);
			o__OnSweepHit = trampoline.write_call<5>(HitCollisionHook2.address() + 0xDD, OnSweepHit);

			_MainUpdate = trampoline.write_call<5>(mainHook.address() + 0x11F, MainUpdate);


			//Actor hook, I would like to not write the main hook, SO instead I think
			// I'm gonna write the actor hook, with the player being the main hook. for timers (given they're always around).
			//REL::Relocation<std::uintptr_t> ActUpAddr{ REL::ID(39375) };  // +69e580//In player update, insufficient
			REL::Relocation<std::uintptr_t> ActUpAddr{ REL::ID(39378) };  // +69F520
			//REL::Relocation<std::uintptr_t> ActUpAddr{ REL::ID(39657) };  // +636EB0

			//_ActorUpdate = trampoline.write_call<5>(ActUpAddr.address() + 0x8AC, ActorUpdate);
			//_ActorUpdate = trampoline.write_call<5>(ActUpAddr.address() + 0x141, ActorUpdate);//did not work at all
			
			//_CharacterUpdate = trampoline.write_call<5>(ActUpAddr.address() + 0x63C, ActorUpdate);
			
			_CharacterUpdate = Character_Main_Vtbl.write_vfunc(0xAD, CharacterUpdate);
			_PlayerUpdate = Player_Main_Vtbl.write_vfunc(0xAD, PlayerUpdate);

			REL::Relocation<std::uintptr_t> PreTestProj{ REL::ID(42982/*74ed10*/) };
			REL::Relocation<std::uintptr_t> PreTestFlm{ REL::ID(42728/*73dd40*/) };
			REL::Relocation<std::uintptr_t> GetRefrFromNodeTest{ REL::ID(42983/*74F650*/) };


			//_PreTest = trampoline.write_branch<5>(GetRefrFromNodeTest.address(), PreTest);
			_PreTestRegular = trampoline.write_call<5>(PreTestProj.address() + 0x839, PreTest);
			_PreTestFlame = trampoline.write_call<5>(PreTestFlm.address() + 0x44A, PreTest);
			_NiNodeTest = trampoline.write_call<5>(GetRefrFromNodeTest.address() + 0x1C7, NiNodeTest);
			
			//REL::Relocation<std::uintptr_t> powStaminaCost{ REL::ID(25863/*3BEC90*/) };
			//REL::Relocation<std::uintptr_t> powStaminaCost{ REL::ID(38047/*63CFB0*/) };
			
			
			REL::Relocation<std::uintptr_t> HitDataHandle{ REL::ID(37633/*626400*/) };

			//140626400 + 8B0 =140626CB0//This is the function for do recoil, I would like to only do this part,
			// the other part is for timed bashes.
			//140626400 + 84E =140626C4E// Block hit, nothing too much to say here. Is as it sounds. A suggestion
			// is using the above "IsDead" call above it so I can run functions on the blocking actor, returning
			// it pure if the person is indeed dead. But if they aren't, I can return true and have the shit go out.
			// note, it is a virtual function call, so it will not play nice unless I can figure out how to.

			//_PerkEntryHook_PowerAttackStamina = trampoline.write_call<5>(powStaminaCost.address() + 0xC8, PerkEntryHook_PowerAttackStamina);
			//_SetAV_PowerAttackHook = trampoline.write_call<5>(powStaminaCost.address() + 0xA2, SetAV_PowerAttackHook);
			//_SetAV_PowerAttackHook = trampoline.write_call<6>(HitCollisionHook.address() + 0x1AA, SetAV_PowerAttackHook);
			
			o__GetAttackStaminaCost = trampoline.write_call<5>(HitCollisionHook.address() + 0x16E, GetAttackStaminaCost);

			o__GetBlockStaminaCost = trampoline.write_call<5>(HitDataHandle.address() + 0x8D4, GetBlockStaminaCost);
			//TestPatch();


			//TestPatch2();
			

			


			//_CollisionNullifier = trampoline.write_call<5>(GetRefrFromNodeTest.address() + 0x1FC, CollisionNullifier);

			//CalcCost_Concentrate = trampoline.write_call<5>(Func29Addr.address() + 0x8AC, ActorUpdate);
			//CalcCost_Charging = trampoline.write_call<5>(Func29Addr.address() + 0x8AC, ActorUpdate);
			//_Func29_ModActorValue = trampoline.write_call<5>(Func29Addr.address() + 0x2AD, Func29_ModActorValue);


			//TestPatch3();

			//REL::Relocation<std::uintptr_t> RecoilCall{ REL::ID(37633) };//0x626400
			//REL::Relocation<std::uintptr_t> RecoilDirectCall{ REL::ID(36699) };//0x5FA080
			
			REL::Relocation<std::uintptr_t> MeleeHit{ REL::ID(37633) };//0x626400


			BlockHitHook::func = trampoline.write_call<5>(MeleeHit.address() + 0x84E, BlockHitHook::thunk);
			
			REL::Relocation<std::uintptr_t> MarkerHook{ REL::ID(50870) };//0x886C60

			//TestNoMarkerHook::func = trampoline.write_call<5>(MarkerHook.address() + 0x3FC, TestNoMarkerHook::thunk);
			
			REL::Relocation<std::uintptr_t> WeaponSpeedHook{ REL::ID(41694) };//0x71B670


			//Keep an eye out on this.
			TimedRecoilHook::func = trampoline.write_call<5>(HitDataHandle.address() + 0x773, TimedRecoilHook::thunk);
			

			TestFistUpdateHook::func = trampoline.write_call<5>(WeaponSpeedHook.address() + 0x29, TestFistUpdateHook::thunk);


			TestApplyCombatHitHook::func = trampoline.write_call<5>(hitEventHook.address() + 0x185, TestApplyCombatHitHook::thunk);

			REL::Relocation<std::uintptr_t> IdleConditionHook{ REL::ID(24068) };//0x358250+0x5E=0x3582AE
			AllowIdleHook::func = trampoline.write_call<5>(IdleConditionHook.address() + 0x5E, AllowIdleHook::thunk);

			logger::info("Hooks finished.");

		}


	private:
		static void thunk(RE::ActorEquipManager* a_this, RE::Actor* a_actor, RE::TESBoundObject* a_object, RE::ExtraDataList* a_extraData, std::uint32_t a_count, const RE::BGSEquipSlot* a_slot, bool a_queueEquip, bool a_forceEquip, bool a_playSounds, bool a_applyNow)
		{
			Utility::DebugMessageBox("No equippy mmm mmm no siree");
			//return true;
			if (a_actor)//This is done to preserve the functionality to get to that point.
				return func(a_this, a_actor, a_object, a_extraData, a_count, a_slot, a_queueEquip, a_forceEquip, a_playSounds, a_applyNow);
		}
		static inline REL::Relocation<decltype(thunk)> func;
		

		static void thunk2(RE::ActorEquipManager* a_this, RE::Actor* a_actor, RE::TESBoundObject* a_object, RE::ExtraDataList* a_extraData, std::uint32_t a_count, const RE::BGSEquipSlot* a_slot, bool a_queueEquip, bool a_forceEquip, bool a_playSounds, bool a_applyNow)
		{
			Utility::DebugMessageBox("double no equippes");
			//return true;
			if (a_actor)//This is done to preserve the functionality to get to that point.
				return func2(a_this, a_actor, a_object, a_extraData, a_count, a_slot, a_queueEquip, a_forceEquip, a_playSounds, a_applyNow);
		}
		static inline REL::Relocation<decltype(thunk2)> func2;
		
		static void TestA(//magic projectile hit
			RE::ActorMagicCaster* a1,
			RE::NiPoint3* a2,
			RE::Projectile* a3,
			RE::TESObjectREFR* a4,
			float a5,
			float a6,
			char a7,
			char a8)
		{
			if (a4)
				Utility::DebugMessageBox(std::format("Hit: {}", a4->GetName()));

			o__TestA(a1, a2, a3, a4, a5, a6, a7, a8);
		}
		static inline REL::Relocation<decltype(TestA)> o__TestA;

		

		static inline RE::Projectile* projectileStore;
		static RE::BSFadeNode* PreTest(void* a1, RE::Projectile* a2, RE::BGSProjectile* a3, std::int64_t a4, float a5)
		{
			//function for getting collidable: void boy, number, collidable reference?

			/*/
			using func_t = decltype(empty);//&ActorValueOwner::GetClampedActorValue);
			REL::Relocation<func_t> func{ 75992 };
			
			RE::hkpCollidable* collidable = nullptr;

			for (int i = 0; func(a1, i, collidable); i++)
			{
				if (collidable)
					Utility::DebugMessageBox(std::format("hit on num {}", i));
			}


			return nullptr;
			//*/

			projectileStore = a2;
		
			RE::BSFadeNode* result = nullptr;
			
			if (a2 && a2->GetFormType() == RE::FormType::ProjectileFlame)
			{
				result = _PreTestFlame(a1, a2, a3, a4, a5);
			}
			else
			{
				result = _PreTestRegular(a1, a2, a3, a4, a5);
			}

			projectileStore = nullptr;

			return result;
		}
		static inline REL::Relocation<decltype(PreTest)> _PreTestRegular;
		static inline REL::Relocation<decltype(PreTest)> _PreTestFlame;

		static RE::TESObjectREFR* NiNodeTest(RE::NiNode* a1, std::int64_t a2)/**/;/*/
		{
			//shit but it works. Needs further testing.

			if (!projectileStore || projectileStore->GetFormType() == RE::FormType::ProjectileGrenade){
				return _NiNodeTest(a1, a2);
			}

			auto refr = _NiNodeTest(a1, a2);
			auto attacker = projectileStore ? projectileStore->shooter.get() : nullptr;

			if (attacker && refr && refr->GetFormType() == RE::FormType::ActorCharacter)
			{
				Utility::DebugMessageBox(std::format("{} vs attacker ({})",
					refr->GetName(), attacker->GetName()));

				return nullptr;
			}

			return refr;
		}
		//*/
		static inline REL::Relocation<decltype(NiNodeTest)> _NiNodeTest;



		static void ProjectileCollision(RE::Projectile* a_this, RE::hkpAllCdPointCollector* cdPoints);
		
		static void PerkEntryHook_PowerAttackStamina(RE::BGSEntryPoint::ENTRY_POINT ep, RE::Actor* perkOwner, RE::TESForm* item, float* value);
		static inline REL::Relocation<decltype(PerkEntryHook_PowerAttackStamina)> _PerkEntryHook_PowerAttackStamina;

		static void  SetAV_PowerAttackHook(RE::ActorValueOwner* avOwner, AVModifier modifier, RE::ActorValue actorValue, float value);
		static inline REL::Relocation<decltype(SetAV_PowerAttackHook)> _SetAV_PowerAttackHook;

		static float GetAttackStaminaCost(RE::ActorValueOwner* a_this, RE::BGSAttackData* attack_data);
		static inline REL::Relocation<decltype(GetAttackStaminaCost)> o__GetAttackStaminaCost;

		//Will not work if you have zero stamina. MIIIIIGHT need patching in order to do this. Something with
		// xybak might be in order, a simple call if there's no stamina.
		static float GetBlockStaminaCost(RE::HitData& a_hitData);
		static inline REL::Relocation<decltype(GetBlockStaminaCost)> o__GetBlockStaminaCost;



		//There is a way to have these be the same, just haven't really done it yet.
		static void OnMeleeHit(RE::Actor* hit_causer, RE::Actor* hit_target, std::int64_t a_int1, bool a_bool, void* a_unkptr);
		static void OnSweepHit(RE::Actor* hit_causer, RE::Actor* hit_target, std::int64_t a_int1, bool a_bool, void* a_unkptr) {
			if (!hit_target || hit_target->IsInCombat() == true)
			{
				return o__OnSweepHit(hit_causer, hit_target, a_int1, a_bool, a_unkptr);
			}

			Utility::DebugNotification("Extra targets not in combat");
		}
		static inline REL::Relocation<decltype(OnSweepHit)> o__OnSweepHit;

		static void MainUpdate(RE::Main* a_this, float a2);


		static uMagicCastState Func29(RE::ActorMagicCaster* a_this, void* a2, float delta);

		static inline REL::Relocation<decltype(Func29)> _Func29;


		static void UpdateAnimation(RE::TESObjectREFR* a_this, float delta)
		{
			//Use this for hit frames

			static bool once = false;
			if (!once)
			{
				bool result = a_this->SetGraphVariableBool("bAnimationDriven", true);

				if (result)
					Utility::DebugMessageBox("D");

				once = true;
			}

			return _UpdateAnimation(a_this, delta * 0.5f);
		}
		static inline REL::Relocation<decltype(UpdateAnimation)> _UpdateAnimation;

		//static inline void ActorUpdate(RE::Actor* a_this, float delta, void* a3);
		static std::int64_t CharacterUpdate(RE::Actor* a_this, float a2, void* a3);
		static std::int64_t PlayerUpdate(RE::Actor* a_this, float a2, void* a3);
		
		
		static void ProcessHitEvent(RE::Actor* a_actor, RE::HitData& a_hitData);

		static void ProcessAnimEvent(AnimSink* a_this, const AnimEvent* a_event, AnimSource* a_dispatcher);
		static EventResult ProcessPlayerAnim(AnimSink* a_this, const AnimEvent* a_event, AnimSource* a_dispatcher);
		static EventResult ProcessCharacterAnim(AnimSink* a_this, const AnimEvent* a_event, AnimSource* a_dispatcher);

		//This will effectively be the thing that's used to prevent 
		static bool NotifyAnimationGraph(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName);

		static inline REL::Relocation<decltype(MainUpdate)> _MainUpdate;
		static inline REL::Relocation<decltype(CharacterUpdate)> _CharacterUpdate;
		static inline REL::Relocation<decltype(PlayerUpdate)> _PlayerUpdate;

		static inline REL::Relocation<decltype(OnMeleeHit)> _OnMeleeHit;
		static inline REL::Relocation<decltype(ProcessHitEvent)> _ProcessHitEvent;

		static inline REL::Relocation<decltype(ProjectileCollision)> _ProjectileCollision_Missile;
		static inline REL::Relocation<decltype(ProjectileCollision)> _ProjectileCollision_Arrow;
		static inline REL::Relocation<decltype(ProjectileCollision)> _ProjectileCollision_Beam;
		static inline REL::Relocation<decltype(ProjectileCollision)> _ProjectileCollision_Flame;
		static inline REL::Relocation<decltype(ProjectileCollision)> _ProjectileCollision_Cone;
		//static inline REL::Relocation<decltype(ProjectileCollision)> _ProjectileCollision_Lobber;//Dunno if I want


		static inline REL::Relocation<decltype(ProcessPlayerAnim)> _ProcessPlayerAnim;
		static inline REL::Relocation<decltype(ProcessCharacterAnim)> _ProcessCharacterAnim;

		static inline REL::Relocation<decltype(NotifyAnimationGraph)> _NotifyPlayerAnimationGraph;
		static inline REL::Relocation<decltype(NotifyAnimationGraph)> _NotifyCharacterAnimationGraph;

	

	};

}

