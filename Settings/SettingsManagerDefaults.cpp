#include "SettingsManager.hpp"

void SettingsManager::loadDefaults(bool doSave)
{
	m_settings["BrewTemp"] = 93.0f;
	m_settings["SteamTemp"] = 145.0f;
	m_settings["BrewPressure"] = 9.0f;

	m_settings["BoilerKp"] = 100.0f;
	m_settings["BoilerKi"] = 10.0f;
	m_settings["BoilerKd"] = 300.0f;

	m_settings["PumpKp"] = 1.0f;
	m_settings["PumpKi"] = 1.0f;
	m_settings["PumpKd"] = 1.0f;

	m_settings["ManualPumpControl"] = 0.0f;
	m_settings["ManualPumpControlEnabled"] = false;

	if (doSave)
		save();
}
