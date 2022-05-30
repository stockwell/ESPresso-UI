#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <variant>

struct SettingDelegate
{
	virtual void onChanged(const std::string& key, const int val) { };
	virtual void onChanged(const std::string& key, const float val) { };
	virtual void onChanged(const std::string& key, const std::string& val) { };
};

class Setting
{
public:
	using SettingValue = std::variant<int, float, std::string>;

	template<typename T>
	Setting& operator=(const T& val)
	{
		try
		{
			if (std::get<T>(m_value) == val)
				return *this;
		}
		catch (std::bad_variant_access& e)
		{
			// This will always occur when setting the initial value on a non-int setting. I'm too
			// lazy to fix this.
		}

		m_value = val;

		for (auto delegate: m_delegates)
			delegate->onChanged(m_key, val);

		return *this;
	}

	template<typename T>
	T getAs() const
	{
		return std::get<T>(m_value);
	}

	SettingValue get() const
	{
		return m_value;
	}

	void registerDelegate(SettingDelegate* delegate)
	{
		m_delegates.emplace(delegate);
	}

	void deregisterDelegate(SettingDelegate* delegate)
	{
		m_delegates.erase(delegate);
	}

	void setKey(const std::string& key)
	{
		m_key = key;
	}

private:
	SettingValue m_value;
	std::string m_key;
	std::set<SettingDelegate*> m_delegates;
};

class SettingsManager
{
public:
	SettingsManager(const SettingsManager&) = delete;
	SettingsManager& operator=(const SettingsManager&) = delete;

	static SettingsManager& get()
	{
		static SettingsManager manager;
		return manager;
	}

	Setting& operator[](const std::string& key)
	{
		if (auto it = m_settings.find(key); it != m_settings.end())
			return it->second;

		m_settings[key].setKey(key);

		return m_settings[key];
	}

	// Must be implemented by SettingsManagerImpl
	void save();
	void load();

protected:
	SettingsManager() = default;

private:
	std::unordered_map<std::string, Setting> m_settings;
};
