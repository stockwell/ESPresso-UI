#pragma once

#include "lvgl.h"

#include "BoilerController.hpp"
#include "ScalesController.hpp"

#include "Logging.hpp"

class EspressoBrewTab
	: public BoilerTemperatureDelegate
	, public ScalesWeightDelegate
{
public:
	EspressoBrewTab(lv_obj_t* parent, BoilerController* boiler, ScalesController* scales);
	virtual ~EspressoBrewTab() = default;

	// BoilerTemperatureDelegate i/f
	void onBoilerCurrentTempChanged(float temp) override;
	void onBoilerTargetTempChanged(float temp) override;
	void onBoilerStateChanged(BoilerState state) override;
	void onBoilerPressureChanged(float pressure) override;

	// ScalesWeightDelegate i/f
	void onScalesWeightChanged(float weight) override;

	void lvglEventAdapter(lv_event_t* e);

private:
	void manualControlBtnEvent(lv_event_t* e);

private:
	enum
	{
		indic_temp,
		indic_arc,
		indic_pressure
	};

	bool m_timerRunning = false;

	uint64_t m_stopwatchTime = 0;
	float m_targetTemp = 0.0f;

	lv_obj_t* m_meter1;
	lv_obj_t* m_meter2;
	lv_obj_t* m_switch2;
	lv_obj_t* m_switch3;
	lv_obj_t* m_arcLabel;
	lv_obj_t* m_weightLabel;
	lv_obj_t* m_pressureLabel;
	lv_obj_t* m_manualControlBtn;

	lv_obj_t* m_chart;
	lv_chart_series_t* m_series1;
	lv_chart_series_t* m_series2;

	lv_meter_indicator_t* m_indic[3];

	lv_timer_t* m_timer;

	BoilerController* m_boilerController;
	BoilerState m_lastState = BoilerState::Heating;

	float m_currentTemp = 0.0f;
	float m_currentPressure = 0.0f;

	ScalesController* m_scalesController;
	float m_weight = 0.0f;

	Logging m_shotLogger;
};
