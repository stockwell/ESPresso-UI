#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <variant>

struct SettingDelegate
{
    virtual void onChanged(int) {};
    virtual void onChanged(float) {};
    virtual void onChanged(std::string) {};
};

class Setting
{
public:
    using SettingValue = std::variant<int, float, std::string>;

    template<typename T>
    void operator = (const T& val)
    {
       // if (std::get<T>(m_value) == val)
       //     return;

        m_value = val;

        for (auto delegate : m_delegates)
            delegate->onChanged(val);
    }

    template<typename T>
    T getAs() const { return std::get<T>(m_value); }

    SettingValue get() const { return m_value; }

    void registerDelegate(SettingDelegate* delegate)
    {
        m_delegates.emplace(delegate);
    }

    void deregisterDelegate(SettingDelegate* delegate)
    {
        m_delegates.erase(delegate);
    }

private:
    SettingValue                m_value;
    std::set<SettingDelegate*>  m_delegates;
};

class SettingsManager
{
public:
    SettingsManager(const SettingsManager &) = delete;
    SettingsManager& operator=(const SettingsManager &) = delete;

    static SettingsManager& get()
    {
        static SettingsManager manager;
        return manager;
    }

    Setting& operator[](const std::string& key)
    {
        if (auto it = m_settings.find(key); it != m_settings.end())
            return it->second;
        
        return m_settings[key];
    }

    void save();
    void load();

protected:
    SettingsManager() = default;

private:
    std::unordered_map<std::string, Setting> m_settings;
};
