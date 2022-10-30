#pragma once

#include <memory>

#include "lvgl.h"

#include "BoilerController.hpp"
#include "ScalesController.hpp"
#include "EspressoBrewTab.hpp"
#include "EspressoSettingsTab.hpp"

class EspressoUI
{
public:
	EspressoUI() = default;
	~EspressoUI() = default;

	void init(BoilerController* boiler, ScalesController* scales);

private:
	enum class DisplaySize
	{
		Small,
		Medium,
		Large,
	};

	std::unique_ptr<EspressoBrewTab>		m_brewTab;
	std::unique_ptr<EspressoSettingsTab>	m_settingsTab;
};
