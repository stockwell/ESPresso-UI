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

	lv_chart_series_t* series1 = nullptr;
	lv_chart_series_t* series2 = nullptr;
	lv_obj_t* chart1 = nullptr;
	float* pressure = nullptr;
	float* temperature = nullptr;
	bool timerRunning = false;

	static void timer_cb(lv_timer_t* t)
	{
		if (series1)
			lv_chart_set_next_value(chart1, series1, *temperature);

		if (series2)
			lv_chart_set_next_value(chart1, series2, *pressure);

		if (! timerRunning)
			return;

		auto [resetSwitch, arc, time] =
			*static_cast<std::tuple<lv_obj_t*, lv_obj_t*, uint64_t*>*>(t->user_data);

		(*time) += kTimerPeriodMs;

		auto* label = lv_obj_get_child(arc, 0);

		if (auto val = lv_arc_get_value(arc); val < kArcMax)
		{
			lv_arc_set_value(arc, val + 1);
		}
		else
		{
			if (auto angle = lv_arc_get_angle_start(arc) + kArcAngleIncrement; angle < 360)
			{
				lv_arc_set_start_angle(arc, angle);
			}
			else
			{
				lv_arc_set_value(arc, 0);
				lv_arc_set_start_angle(arc, 0);
			}
		}

		lv_label_set_text_fmt(label, "%u", *time / 1000);



	}

	struct ResetSwitchData
	{
		lv_obj_t* switch1;
		lv_obj_t* arc;
		lv_timer_t* timer;
		uint64_t* time;
	};

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

		auto [switch1, timer] =
			*static_cast<std::pair<lv_obj_t*, lv_timer_t*>*>(lv_event_get_user_data(e));

		if (lv_obj_has_state(obj, LV_STATE_CHECKED))
		{
			lv_label_set_text(label, "Stop");

			if (lv_obj_get_state(switch1) == LV_STATE_DISABLED)
				lv_obj_clear_state(switch1, LV_STATE_DISABLED);

			timerRunning = true;
		}
		else
		{
			lv_label_set_text(label, "Start");
			timerRunning = false;
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
}

EspressoBrewTab::EspressoBrewTab(lv_obj_t* parent, BoilerController* boiler)
{
	m_boilerController = boiler;
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
		lv_meter_add_needle_line(m_meter2, scale3, 2, lv_palette_main(LV_PALETTE_RED), -10);

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
	lv_chart_set_range(m_chart, LV_CHART_AXIS_SECONDARY_Y, 0, 16);
	lv_chart_set_point_count(m_chart, 300);


	lv_chart_set_axis_tick(m_chart, LV_CHART_AXIS_PRIMARY_X, 4, 2, 12, 3, true, 40);
	lv_chart_set_axis_tick(m_chart, LV_CHART_AXIS_PRIMARY_Y, 4, 2, 6, 2, true, 50);
	lv_chart_set_axis_tick(m_chart, LV_CHART_AXIS_SECONDARY_Y, 4, 2, 3, 4, true, 50);

	lv_obj_set_style_text_font(m_chart, &lv_font_montserrat_10, 0);

	m_series1 = lv_chart_add_series(m_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
	m_series2 = lv_chart_add_series(m_chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);

	// Panel 2 - Timer and brew/steam setting
	lv_obj_t* panel2 = lv_obj_create(parent);
	lv_obj_set_size(panel2, 370, 360);
	lv_obj_set_style_pad_row(panel2, 13, 0);

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
	lv_obj_remove_style(arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
	lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
	lv_obj_center(arc);
	lv_obj_set_size(arc, 250, 250);

	m_arcLabel = lv_label_create(arc);
	lv_label_set_text(m_arcLabel, "Heating");
	lv_obj_center(m_arcLabel);
	lv_obj_set_style_text_font(m_arcLabel, &lv_font_montserrat_28, 0);

	static lv_coord_t grid_col_dsc[] = {40, 120, 120, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	lv_obj_set_grid_dsc_array(panel2, grid_col_dsc, grid_row_dsc);
	lv_obj_set_grid_cell(arc, LV_GRID_ALIGN_END, 0, 3, LV_GRID_ALIGN_CENTER, 0, 1);
	lv_obj_set_grid_cell(m_switch2, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 1, 1);
	lv_obj_set_grid_cell(m_switch3, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);

	auto* timerData = new std::tuple<lv_obj_t*, lv_obj_t*, uint64_t*>(m_switch3, arc, &m_stopwatchTime);
	m_timer = lv_timer_create(timer_cb, kTimerPeriodMs, timerData);

	auto* resetSwitchData = new ResetSwitchData
	{
		m_switch2,
		arc,
		m_timer,
		&m_stopwatchTime,
	};

	lv_obj_add_event_cb(m_switch3, reset_switch_event_cb, LV_EVENT_ALL, resetSwitchData);

	auto* timerSwitchData = new std::pair<lv_obj_t*, lv_timer_t*>(m_switch3, m_timer);
	lv_obj_add_event_cb(m_switch2, timer_switch_event_cb, LV_EVENT_ALL, timerSwitchData);

	static lv_coord_t cont_grid_col_dsc[] =
		{LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t cont_grid_row_dsc[] =
		{LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

	lv_obj_set_grid_dsc_array(parent, cont_grid_col_dsc, cont_grid_row_dsc);
	lv_obj_set_grid_cell(cont, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(m_chart, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_cell(panel2, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 2);

	m_boilerController->registerBoilerTemperatureDelegate(this);

	pressure = &m_currentPressure;
	temperature = &m_currentTemp;
	series1 = m_series1;
	series2 = m_series2;
	chart1 = m_chart;
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
