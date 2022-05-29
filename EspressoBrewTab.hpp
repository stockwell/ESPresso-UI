#pragma once

#include "lvgl.h"

#include "BoilerController.hpp"

class EspressoBrewTab
	: public BoilerTemperatureDelegate
{
public:
	EspressoBrewTab(lv_obj_t* parent, BoilerController* boiler);
	virtual ~EspressoBrewTab() = default;

	// BoilerTemperatureDelegate i/f
	void onBoilerCurrentTempChanged(float temp) override;
	void onBoilerTargetTempChanged(float temp) override;

private:
	enum
	{
		indic_temp,
		indic_arc,
	};

	uint64_t m_stopwatchTime = 0;
	float m_targetTemp = 0.0f;

	lv_obj_t* m_meter1;
	lv_obj_t* m_switch2;
	lv_obj_t* m_arcLabel;

	lv_meter_indicator_t* m_indic[2];

	lv_timer_t* m_timer;

	BoilerController* m_boilerController;
};
