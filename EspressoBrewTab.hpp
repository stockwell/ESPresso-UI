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
	void onBoilerStateChanged(BoilerState state) override;

	void onBoilerPressureChanged(float pressure) override;

private:
	enum
	{
		indic_temp,
		indic_arc,
		indic_pressure
	};

	uint64_t m_stopwatchTime = 0;
	float m_targetTemp = 0.0f;

	lv_obj_t* m_meter1;
	lv_obj_t* m_meter2;
	lv_obj_t* m_switch2;
	lv_obj_t* m_switch3;
	lv_obj_t* m_arcLabel;

	lv_obj_t* m_chart;
	lv_chart_series_t* m_series1;
	lv_chart_series_t* m_series2;

	lv_meter_indicator_t* m_indic[3];

	lv_timer_t* m_timer;

	BoilerController* m_boilerController;
	BoilerState m_lastState = BoilerState::Heating;

	float m_currentTemp = 0.0f;
	float m_currentPressure = 0.0f;
};
