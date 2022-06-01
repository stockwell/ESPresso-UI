#include "SettingsManager.hpp"

void SettingsManager::save()
{
	printf("%s - Saving %zu keys..\n", __PRETTY_FUNCTION__, m_settings.size());

	for (const auto& [key, value]: m_settings)
	{
		switch (value.get().index())
		{
		case 0:
			printf("--> %s: %d\n", key.c_str(), value.getAs<bool>());
			break;

		case 1:
			printf("--> %s: %lld\n", key.c_str(), value.getAs<int64_t>());
			break;

		case 2:
			printf("--> %s: %f\n", key.c_str(), value.getAs<float>());
			break;

		case 3:
			printf("--> %s: %s\n", key.c_str(), value.getAs<std::string>().c_str());
			break;
		}
	}
}

void SettingsManager::load()
{
	printf("%s - Loading Defaults\n", __PRETTY_FUNCTION__);
	loadDefaults();
}
