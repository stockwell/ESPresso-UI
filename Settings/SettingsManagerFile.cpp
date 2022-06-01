#include "SettingsManager.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>

#include "nlohmann/json.hpp"

namespace
{
	std::string_view kSettingsPath = "Settings.json";
}

void SettingsManager::save()
{
	std::ofstream settingsFile(kSettingsPath);

	nlohmann::json settingsJSON;

	//TODO: Less fragile system for encoding Type.. but meh this works.
	for (const auto& [key, value]: m_settings)
	{
		switch (value.get().index())
		{
		case 0:
			settingsJSON[key] = {
				{"Type", value.get().index()},
				{"Value", value.getAs<bool>()},
			};

		case 1:
			settingsJSON[key] = {
				{"Type", value.get().index()},
				{"Value", value.getAs<int64_t>()},
			};
			break;

		case 2:
			settingsJSON[key] = {
				{"Type", value.get().index()},
				{"Value", value.getAs<float>()},
			};
			break;

		case 3:
			settingsJSON[key] = {
				{"Type", value.get().index()},
				{"Value", value.getAs<std::string>()},
			};
			break;
		}
	}

	settingsFile << std::setw(4) << settingsJSON << std::endl;
}

void SettingsManager::load()
{
	std::ifstream settingsFile(kSettingsPath);
	nlohmann::json settingsJSON;

	try
	{
		settingsFile >> settingsJSON;

		for (auto& [key, value]: settingsJSON.items())
		{
			switch (value["Type"].get<int>())
			{
			case 0:
				m_settings[key] = value["Value"].get<bool>();
				break;

			case 1:
				m_settings[key] = value["Value"].get<int64_t>();
				break;

			case 2:
				m_settings[key] = value["Value"].get<float>();
				break;

			case 3:
				m_settings[key] = value["Value"].get<std::string>();
				break;
			}
		}
	}
	catch (std::exception& e)
	{
		printf("Error loading Settings.json, restoring defaults..\n");
		printf("\t%s\n", e.what());
		loadDefaults();
		save();
	}
}
