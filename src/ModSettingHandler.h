#pragma once

#include "ModSettings.h"
#include "ModSettingContainer.h"


namespace Crucible
{
	class ModSettingHandler
	{
	private:
		//All enums will be turned into their underlying types.
		static inline std::map<size_t, std::map<std::int32_t, SettingBase*>> settingHolder;

		GeneralSettings _generalSetting;

		//Unused function for not, something that will throw an exception if enabled to do so.
		bool ThrowOrFalse(bool value) { if (!value) return false; throw nullptr; }
	public:
		//This is not to be used, the crux of the idea was suppose to be that you use a form similarly immediately
		// Get the value you're looking for. But instead, it will have to be a setting key. There's no
		// piecing method unfortunately.
		template <class SettingKey>
		static float GetSettingValue(SettingKey key, std::string& valueName)
		{
			//Basically does nothing for now.
			//const std::type_info& info = typeid(key);
			//auto& specMap = settingHolder[info.hash_code()];
			//auto underlying = static_cast<std::int32_t>(key);
			//specMap[underlying] = setting;
			return 0.f;
		}

		template <class SettingKey>
		static SettingBase* GetSetting(SettingKey key)
		{
			const std::type_info& info = typeid(key);

			auto& specMap = settingHolder[info.hash_code()];

			auto underlying = static_cast<std::int32_t>(key);

			return specMap[underlying];
		}
		
		template <class AsType, class SettingKey>
		static AsType* GetSetting(SettingKey key)
		{
			return dynamic_cast<AsType*>(GetSetting(key));
		}

		static GeneralSettings& GetGeneralSettings()
		{
			return General::settings;
		}


		//This takes 2 versions. Either it will take a reference, or a pointer.
		// The pointer indicates prior creation. While using a reference will have the thing turned
		// into a pointer and then stored.

		//The question is how would I like to initialize all these values?
		// I would not like to use new, because I'd like to think of the point they are owned at.

		template<class SettingKey>
		static bool AddSetting(SettingBase* setting, SettingKey key)
		{
			if (!setting || setting->IsProperKey(key) == false)
				return false;

			const std::type_info& info = typeid(key);
			//*1
			auto& specMap = settingHolder[info.hash_code()];

			auto underlying = static_cast<std::int32_t>(key);
			//*1
			if (specMap[underlying] == setting)
				return false;

			specMap[underlying] = setting;

			//For both of *1, you'll want to check if they actually exist before searching for them.

			return true;
		}

		template<class SettingKey>
		static bool AddSetting(SettingBase& setting, SettingKey key)
		{
			return AddSetting(&setting, key);
		}


	private:
		//static inline RecoverySetting magicRecoverSetting;
		//static inline RecoverySetting staminaRecoverSetting;
		//static inline RecoverySetting speedRecoverSetting;


		static void LoadActionSettings()
		{
			ActionSettings::Initialize();

			AddSetting(ActionSettings::autoDodge, ActionPoint::autoDodge);
			AddSetting(ActionSettings::genericDodge, ActionPoint::genericDodge);
			AddSetting(ActionSettings::genericGuard, ActionPoint::genericGuard);

			AddSetting(ActionSettings::guardDropDodge, ActionPoint::guardDropDodge);
			AddSetting(ActionSettings::lightShieldGuard, ActionPoint::lightStrongGuard);
			AddSetting(ActionSettings::heavyShieldGuard, ActionPoint::heavyStrongGuard);
		}


		static void LoadStrikeSettings()
		{
			StrikeSettings::Initialize();

			AddSetting(StrikeSettings::offenseHitSetting, StrikeThreshold::offenseHit);
			AddSetting(StrikeSettings::defenseHitSetting, StrikeThreshold::defenseHit);
			AddSetting(StrikeSettings::offenseEvadeSetting, StrikeThreshold::offenseEvade);
			AddSetting(StrikeSettings::defenseEvadeSetting, StrikeThreshold::defenseEvade);
		}

		static void LoadRecoverySettings()
		{
			RecoverySettings::Initialize();

			AddSetting(RecoverySettings::poiseSetting, RecoverValue::kPoise);
			AddSetting(RecoverySettings::staminaSetting, RecoverValue::kStamina);
			AddSetting(RecoverySettings::magickaSetting, RecoverValue::kMagicka);
			AddSetting(RecoverySettings::speedSetting, RecoverValue::kSpeed);
		}

	public:
		static void Initialize()
		{
			//The relevant objects will be keep in their respective files, I'll just load them from here.
			LoadActionSettings();
			LoadStrikeSettings();
			LoadRecoverySettings();
		}
	};

	




	

}