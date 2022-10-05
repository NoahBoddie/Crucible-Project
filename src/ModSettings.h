#pragma once
namespace Crucible
{
	struct SettingBase
	{
	protected:
		virtual size_t GetEntryHash() = 0;
	
	public:
		template <class SettingKey>
		bool IsProperKey(SettingKey key)
		{
			return GetEntryHash() == typeid(key).hash_code();
		}

		//virtual float GetValue(std::string& name) { return 0.f; }
	};


	template<class KeyType>
	struct SettingTemplate : public SettingBase
	{
	protected:
		size_t GetEntryHash() override { return typeid(KeyType).hash_code(); }


	public:
		template <class SettingKey>
		float GetSettingValue(SettingKey key, std::string& valueName)
		{
			return 0.f;
		}


	};

}