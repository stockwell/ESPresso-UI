#pragma once

#include "lvgl.h"

#include "Settings/SettingsManager.hpp"

class EspressoSettingsTab
{
public:
	EspressoSettingsTab(lv_obj_t* parent);
	~EspressoSettingsTab() = default;

private:
	lv_obj_t*	m_parent;
};
