#include "EspressoBrewTab.hpp"
#include "Settings/SettingsManager.hpp"

namespace
{
	static lv_style_t style_title;
	static lv_style_t style_bullet;

	constexpr int kTimerPeriodMs = 100;
	constexpr int kShotTimeSec = 30;
	constexpr int kArcMax = (1000 / kTimerPeriodMs) * kShotTimeSec + 1;
	constexpr int kArcAngleIncrement = std::max(1, 360 / (kArcMax));

	struct TimerData
	{
		bool* timerRunning;

		float* pressure;
		float* temperature;

		uint64_t* time;

		lv_obj_t* resetSwitch;
		lv_obj_t* arc;
		lv_obj_t* chart;

		lv_chart_series_t* chartPressureSeries;
		lv_chart_series_t* chartTemperatureSeries;

		Logging* shotLog;
	};

	struct ResetSwitchData
	{
		lv_obj_t* switch1;
		lv_obj_t* arc;
		lv_timer_t* timer;
		uint64_t* time;
	};

	struct TimerSwitchData
	{
		bool* timerRunning;
		lv_obj_t* switch1;

		Logging* log;
	};

	static void timer_cb(lv_timer_t* t)
	{
		auto data = static_cast<TimerData*>(t->user_data);

		auto dataPoint = std::make_pair(*data->temperature, *data->pressure * 20);

		const auto& [temperature, pressure] = dataPoint;

		lv_chart_set_next_value(data->chart, data->chartTemperatureSeries, temperature);
		lv_chart_set_next_value(data->chart, data->chartPressureSeries, pressure);

		if (! *data->timerRunning)
			return;

		data->shotLog->AddData(dataPoint);

		(*data->time) += kTimerPeriodMs;

		auto* label = lv_obj_get_child(data->arc, 0);

		if (auto val = lv_arc_get_value(data->arc); val < kArcMax)
		{
			lv_arc_set_value(data->arc, val + 1);
		}
		else
		{
			if (auto angle = lv_arc_get_angle_start(data->arc) + kArcAngleIncrement; angle < 360)
			{
				lv_arc_set_start_angle(data->arc, angle);
			}
			else
			{
				lv_arc_set_value(data->arc, 0);
				lv_arc_set_start_angle(data->arc, 0);
			}
		}

		lv_label_set_text_fmt(label, "%u", static_cast<uint>(*data->time / 1000));
	}


	static void reset_switch_event_cb(lv_event_t* e)
	{
		if (e->code != LV_EVENT_RELEASED)
			return;

		auto data = *static_cast<ResetSwitchData*>(lv_event_get_user_data(e));

		// reset time
		*(data.time) = 0;

		// set arclabel to boiler state
		auto label = lv_obj_get_child(data.arc, 0);
		lv_label_set_text(label, "Ready");
		lv_arc_set_value(data.arc, 0);

		// set timer switch label to start and unchecked
		lv_obj_clear_state(data.switch1, LV_STATE_CHECKED);
		lv_label_set_text(lv_obj_get_child(data.switch1, 0), "Start");

		// disable self
		lv_obj_add_state(lv_event_get_target(e), LV_STATE_DISABLED);
	}

	static void timer_switch_event_cb(lv_event_t* e)
	{
		lv_obj_t* obj = lv_event_get_target(e);
		lv_obj_t* label = lv_obj_get_child(obj, 0);

		auto* data = static_cast<TimerSwitchData*>(lv_event_get_user_data(e));

		if (lv_obj_has_state(obj, LV_STATE_CHECKED))
		{
			lv_label_set_text(label, "Stop");

			if (lv_obj_get_state(data->switch1) == LV_STATE_DISABLED)
				lv_obj_clear_state(data->switch1, LV_STATE_DISABLED);

			*data->timerRunning = true;
		}
		else
		{
			if (*data->timerRunning)
			{
				lv_label_set_text(label, "Start");

				*data->timerRunning = false;

				data->log->FlushLog();
			}
		}
	}

	static lv_obj_t* create_meter_box(lv_obj_t* parent)
	{
		lv_obj_t* meter = lv_meter_create(parent);
		lv_obj_remove_style(meter, nullptr, LV_PART_MAIN);

		lv_obj_t* label1 = lv_label_create(meter);
		lv_obj_set_style_text_font(label1, &lv_font_montserrat_16, 0);
		lv_obj_set_pos(label1, 45, 115);

		return meter;
	}

	static void sliderCb(lv_event_t* e)
	{
		lv_obj_t* slider = lv_event_get_target(e);
		auto [key, fmt] = *static_cast<std::pair<std::string, std::string>*>(lv_event_get_user_data(e));
		auto val = lv_slider_get_value(slider);

		auto& settings = SettingsManager::get();
		settings[key] = static_cast<float>(val*6);
		settings.save();
	}

	static lv_obj_t* createSlider(lv_obj_t* parent, const std::string& key, const std::pair<int, int>& range, const std::string& fmt)
	{
		auto slider = lv_slider_create(parent);
		auto initial = SettingsManager::get()[key].getAs<float>();

		lv_slider_set_range(slider, range.first, range.second/6);
		lv_slider_set_value(slider, static_cast<int>(initial), LV_ANIM_OFF);
		lv_obj_set_size(slider, 330, 15);
		lv_obj_center(slider);

		lv_obj_add_event_cb(slider, sliderCb, LV_EVENT_VALUE_CHANGED, new std::pair<std::string, std::string>(key, fmt));

		return slider;
	}

	lv_group_t* g;

	void lv_group_init()
	{
		g = lv_group_create();

		lv_indev_t* cur_drv = nullptr;
		for (;;)
		{
			cur_drv = lv_indev_get_next(cur_drv);
			if (! cur_drv)
				break;

			if (cur_drv->driver->type == LV_INDEV_TYPE_KEYPAD)
				lv_indev_set_group(cur_drv, g);
		}
	}
}

EspressoBrewTab::EspressoBrewTab(lv_obj_t* parent, BoilerController* boiler, ScalesController* scales)
	: m_shotLogger(false, "shot")
	, m_boilerController(boiler)
	, m_scalesController(scales)
{
	lv_group_init();

	lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);

	lv_style_init(&style_title);
	lv_style_set_text_font(&style_title, LV_FONT_DEFAULT);

	lv_style_init(&style_bullet);
	lv_style_set_border_width(&style_bullet, 0);
	lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);
	lv_obj_set_style_pad_row(parent, 10, 0);

	// Panel 1 -- Temperature Gauge
	lv_obj_t* cont = lv_obj_create(parent);
	lv_obj_set_size(cont, 370, 180);
	lv_obj_set_style_pad_row(cont, 0, 0);
	lv_obj_set_style_pad_column(cont, 20, 0);

	m_meter1 = create_meter_box(cont);

	/*Add a special circle to the needle's pivot*/
	lv_obj_set_style_size(m_meter1, 4, LV_PART_INDICATOR);
	lv_obj_set_style_radius(m_meter1, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
	lv_obj_set_style_bg_opa(m_meter1, LV_OPA_COVER, LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(m_meter1, lv_palette_darken(LV_PALETTE_GREY, 4), LV_PART_INDICATOR);
	lv_obj_set_style_text_font(m_meter1, &lv_font_montserrat_12, 0);

	lv_meter_scale_t* scale = lv_meter_add_scale(m_meter1);
	lv_meter_set_scale_ticks(m_meter1, scale, 41, 1, 8, lv_palette_main(LV_PALETTE_GREY));
	lv_meter_set_scale_major_ticks(m_meter1, scale, 8, 2, 8, lv_color_black(), 10);

	lv_meter_set_scale_range(m_meter1, scale, 0, 200, 270, 135);

	m_indic[indic_temp] =
		lv_meter_add_needle_line(m_meter1, scale, 2, lv_palette_main(LV_PALETTE_RED), -10);
	m_indic[indic_arc] =
		lv_meter_add_arc(m_meter1, scale, 3, lv_palette_main(LV_PALETTE_GREEN), 0);

	lv_meter_indicator_t* indic = lv_meter_add_arc(m_meter1, scale, 2, lv_palette_main(LV_PALETTE_BLUE), 0);
	lv_meter_set_indicator_start_value(m_meter1, indic, 0);
	lv_meter_set_indicator_end_value(m_meter1, indic, 40);

	indic = lv_meter_add_scale_lines(m_meter1,
		scale,
		lv_palette_main(LV_PALETTE_BLUE),
		lv_palette_main(LV_PALETTE_BLUE),
		false,
		0);
	lv_meter_set_indicator_start_value(m_meter1, indic, 0);
	lv_meter_set_indicator_end_value(m_meter1, indic, 40);

	indic = lv_meter_add_arc(m_meter1, scale, 2, lv_palette_main(LV_PALETTE_RED), 0);
	lv_meter_set_indicator_start_value(m_meter1, indic, 160);
	lv_meter_set_indicator_end_value(m_meter1, indic, 200);

	indic = lv_meter_add_scale_lines(m_meter1,
		scale,
		lv_palette_main(LV_PALETTE_RED),
		lv_palette_main(LV_PALETTE_RED),
		false,
		0);
	lv_meter_set_indicator_start_value(m_meter1, indic, 160);
	lv_meter_set_indicator_end_value(m_meter1, indic, 200);

	indic = lv_meter_add_scale_lines(m_meter1,
		scale,
		lv_palette_main(LV_PALETTE_RED),
		lv_palette_main(LV_PALETTE_RED),
		false,
		0);
	lv_meter_set_indicator_start_value(m_meter1, indic, 160);
	lv_meter_set_indicator_end_value(m_meter1, indic, 200);

	m_meter2 = create_meter_box(cont);

	/*Add a special circle to the needle's pivot*/
	lv_obj_set_style_size(m_meter2, 4, LV_PART_INDICATOR);
	lv_obj_set_style_radius(m_meter2, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
	lv_obj_set_style_bg_opa(m_meter2, LV_OPA_COVER, LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(m_meter2, lv_palette_darken(LV_PALETTE_GREY, 4), LV_PART_INDICATOR);
	lv_obj_set_style_text_font(m_meter2, &lv_font_montserrat_12, 0);

	lv_meter_scale_t* scale2 = lv_meter_add_scale(m_meter2);
	lv_meter_set_scale_ticks(m_meter2, scale2, 41, 1, 8, lv_palette_main(LV_PALETTE_GREY));
	lv_meter_set_scale_major_ticks(m_meter2, scale2, 8, 2, 8, lv_color_black(), 10);
	lv_meter_set_scale_range(m_meter2, scale2, 0, 20, 270, 135);

	lv_meter_scale_t* scale3 = lv_meter_add_scale(m_meter2);
	lv_meter_set_scale_ticks(m_meter2, scale3, 41, 1, 8, lv_palette_main(LV_PALETTE_GREY));
	lv_meter_set_scale_range(m_meter2, scale3, 0, 400, 270, 135);

	m_indic[indic_pressure] =
		lv_meter_add_needle_line(m_meter2, scale3, 2, lv_palette_main(LV_PALETTE_BLUE), -10);

	static lv_coord_t outer_grid_col_dsc[] =
		{LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t outer_grid_row_dsc[] =
		{LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	lv_obj_set_grid_dsc_array(cont, outer_grid_col_dsc, outer_grid_row_dsc);
	lv_obj_set_grid_cell(m_meter1, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(m_meter2, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_START, 0, 1);

	// Chart
	m_chart = lv_chart_create(parent);
	lv_obj_set_size(m_chart, 370, 170);
	lv_obj_align(m_chart, LV_ALIGN_CENTER, 0, 0);
	lv_chart_set_range(m_chart, LV_CHART_AXIS_PRIMARY_Y, 50, 150);
	lv_chart_set_range(m_chart, LV_CHART_AXIS_SECONDARY_Y, 0, 14*20);
	lv_chart_set_point_count(m_chart, 325);

	lv_chart_set_axis_tick(m_chart, LV_CHART_AXIS_PRIMARY_X, 3, 2, 12, 3, true, 40);
	lv_chart_set_axis_tick(m_chart, LV_CHART_AXIS_PRIMARY_Y, 3, 2, 6, 2, true, 50);

	lv_obj_set_style_text_font(m_chart, &lv_font_montserrat_8, 0);

	m_series1 = lv_chart_add_series(m_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
	m_series2 = lv_chart_add_series(m_chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);

	// Panel 2 - Timer and brew/steam setting
	lv_obj_t* panel2 = lv_obj_create(parent);
	lv_obj_set_size(panel2, 370, 180);
	lv_obj_set_style_pad_all(panel2, 0, LV_PART_MAIN);

	m_switch2 = lv_btn_create(panel2);
	lv_obj_align(m_switch2, LV_ALIGN_CENTER, 0, 0);
	lv_obj_add_state(m_switch2, LV_STATE_DISABLED);
	lv_obj_add_flag(m_switch2, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_set_height(m_switch2, LV_SIZE_CONTENT);

	auto swLabel2 = lv_label_create(m_switch2);
	lv_label_set_text(swLabel2, "Start");
	lv_obj_set_style_text_font(swLabel2, &lv_font_montserrat_20, 0);
	lv_obj_center(swLabel2);

	m_switch3 = lv_btn_create(panel2);
	lv_obj_align(m_switch3, LV_ALIGN_CENTER, 0, 0);
	lv_obj_add_state(m_switch3, LV_STATE_DISABLED);
	lv_obj_set_height(m_switch3, LV_SIZE_CONTENT);

	auto swLabel3 = lv_label_create(m_switch3);
	lv_label_set_text(swLabel3, "Reset");
	lv_obj_set_style_text_font(swLabel3, &lv_font_montserrat_20, 0);
	lv_obj_center(swLabel3);

	lv_obj_t* arc = lv_arc_create(panel2);

	lv_arc_set_rotation(arc, 270);
	lv_arc_set_bg_angles(arc, 0, 360);
	lv_arc_set_angles(arc, 0, 0);
	lv_arc_set_range(arc, 0, kArcMax);
	lv_obj_remove_style(arc, nullptr, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
	lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
	lv_obj_center(arc);
	lv_obj_set_size(arc, 160, 160);

	m_arcLabel = lv_label_create(arc);
	lv_label_set_text(m_arcLabel, "Heating");
	lv_obj_center(m_arcLabel);
	lv_obj_set_style_text_font(m_arcLabel, &lv_font_montserrat_28, 0);

	lv_obj_align(arc, LV_ALIGN_TOP_LEFT, 30, 10);

	auto timerData = new TimerData
	{
		.timerRunning = &m_timerRunning,
		.pressure = &m_currentPressure,
		.temperature = &m_currentTemp,
		.time = &m_stopwatchTime,
		.resetSwitch = m_switch3,
		.arc = arc,
		.chart = m_chart,
		.chartPressureSeries = m_series2,
		.chartTemperatureSeries = m_series1,
		.shotLog = &m_shotLogger,
	};

	m_timer = lv_timer_create(timer_cb, kTimerPeriodMs, timerData);

	auto* resetSwitchData = new ResetSwitchData
	{
		m_switch2,
		arc,
		m_timer,
		&m_stopwatchTime,
	};

	lv_obj_add_event_cb(m_switch3, reset_switch_event_cb, LV_EVENT_ALL, resetSwitchData);

	auto* timerSwitchData = new TimerSwitchData
	{
		&m_timerRunning,
		m_switch3,
		&m_shotLogger
	};

	lv_obj_add_event_cb(m_switch2, timer_switch_event_cb, LV_EVENT_ALL, timerSwitchData);

	lv_obj_align(m_switch2, LV_ALIGN_TOP_MID, 100, 35);
	lv_obj_align_to(m_switch3, m_switch2, LV_ALIGN_CENTER, 0, 70);

	lv_obj_t* panel3 = lv_obj_create(parent);
	lv_obj_set_size(panel3, 370, 100);
	lv_obj_set_style_pad_all(panel3, 0, LV_PART_MAIN);

	m_weightLabel = lv_label_create(panel3);
	lv_label_set_text(m_weightLabel, "0.00g");
	lv_obj_align(m_weightLabel, LV_ALIGN_CENTER, -80, 0);
	lv_obj_set_style_text_font(m_weightLabel, &lv_font_montserrat_30, 0);

	m_pressureLabel = lv_label_create(panel3);
	lv_label_set_text(m_pressureLabel, "0.0 Bar");
	lv_obj_align(m_pressureLabel, LV_ALIGN_CENTER, 80, 0);
	lv_obj_set_style_text_font(m_pressureLabel, &lv_font_montserrat_30, 0);

	lv_obj_t* panel4 = lv_obj_create(parent);
	lv_obj_set_size(panel4, 370, 60);
	lv_obj_set_style_pad_all(panel4, 0, LV_PART_MAIN);

	auto slider = createSlider(panel4, "ManualPumpControl", {1, 100}, "%d%%");

	lv_group_add_obj(g, slider);
	lv_obj_add_flag(slider, LV_OBJ_FLAG_CHECKABLE);

	static lv_coord_t cont_grid_col_dsc[] =
		{LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t cont_grid_row_dsc[] =
		{LV_GRID_CONTENT, 180, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

	lv_obj_set_grid_dsc_array(parent, cont_grid_col_dsc, cont_grid_row_dsc);
	lv_obj_set_grid_cell(cont, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 2);
	lv_obj_set_grid_cell(m_chart, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 2);
	lv_obj_set_grid_cell(panel2, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 2);
	lv_obj_set_grid_cell(panel3, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(panel4, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 3, 1);

	m_boilerController->registerBoilerTemperatureDelegate(this);
}

void EspressoBrewTab::onBoilerTargetTempChanged(float temp)
{
	m_targetTemp = temp;

	auto round = [](int val) {
		val = val + 5 / 2;
		val -= val % 5;
		return val;
	};

	auto start = round(temp - 5);
	auto end = round(temp + 5);

	lv_meter_set_indicator_start_value(m_meter1, m_indic[indic_arc], start);
	lv_meter_set_indicator_end_value(m_meter1, m_indic[indic_arc], end);
}

void EspressoBrewTab::onBoilerCurrentTempChanged(float temp)
{
	lv_meter_set_indicator_end_value(m_meter1, m_indic[indic_temp], static_cast<int>(temp));

	lv_obj_t* label = lv_obj_get_child(m_meter1, -1);
	lv_label_set_text_fmt(label, "%.01f°c", temp);

	m_currentTemp = temp;
}

void EspressoBrewTab::onBoilerPressureChanged(float pressure)
{
	lv_meter_set_indicator_end_value(m_meter2, m_indic[indic_pressure], static_cast<int>(pressure*20));

	lv_obj_t* label = lv_obj_get_child(m_meter2, -1);
	lv_label_set_text_fmt(label, "%.01f bar", pressure);

	m_currentPressure = pressure;
}

void EspressoBrewTab::onBoilerStateChanged(BoilerState state)
{
	switch (state)
	{
	case BoilerState::Heating:
		lv_label_set_text(m_arcLabel, "Heating");
		lv_obj_add_state(m_switch2, LV_STATE_DISABLED);
		break;

	case BoilerState::Inhibited:
		lv_label_set_text(m_arcLabel, "Inhibited");
		lv_obj_add_state(m_switch2, LV_STATE_DISABLED);
		break;

	case BoilerState::Idle:
		lv_label_set_text(m_arcLabel, "Idle");
		lv_obj_add_state(m_switch2, LV_STATE_DISABLED);
		break;

	case BoilerState::Ready:

		if (m_lastState != BoilerState::Brewing)
		{
			lv_label_set_text(m_arcLabel, "Ready");
			lv_obj_clear_state(m_switch2, LV_STATE_DISABLED);
		}
		else
		{
			lv_obj_clear_state(m_switch2, LV_STATE_CHECKED);
		}
		break;

	case BoilerState::Brewing:
		lv_obj_add_state(m_switch2, LV_STATE_CHECKED);
		lv_obj_clear_state(m_switch3, LV_STATE_DISABLED);
		break;
	}

	m_lastState = state;
}
