#include "SettingsManager.hpp"

void SettingsManager::loadDefaults()
{
	m_settings["BrewTemp"] = 93.0f;
	m_settings["SteamTemp"] = 145.0f;

	save();
}
