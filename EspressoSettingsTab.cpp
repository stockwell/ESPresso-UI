#include "EspressoSettingsTab.hpp"

static lv_obj_t* createLabel(lv_obj_t* parent, const char* text)
{
	auto label = lv_label_create(parent);
	lv_label_set_text(label, text);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_PART_MAIN);

	return label;
}

static void sliderCb(lv_event_t* e)
{
	lv_obj_t* slider = lv_event_get_target(e);
	auto [label, key, fmt] = *static_cast<std::tuple<lv_obj_t*, std::string, std::string>*>(lv_event_get_user_data(e));
	auto val = lv_slider_get_value(slider);

	lv_label_set_text_fmt(label, fmt.c_str(), val);

	auto& settings = SettingsManager::get();
	settings[key] = static_cast<float>(val);
	settings.save();
}

static std::pair<lv_obj_t*, lv_obj_t*> createSlider(lv_obj_t* parent, const std::string& key, const std::pair<int, int>& range, const std::string& fmt)
{
	auto slider = lv_slider_create(parent);
	auto initial = SettingsManager::get()[key].getAs<float>();

	lv_slider_set_range(slider, range.first, range.second);
	lv_slider_set_value(slider, static_cast<int>(initial), LV_ANIM_OFF);
	lv_obj_set_size(slider, 580, 20);

	auto label = lv_label_create(parent);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_label_set_text_fmt(label, fmt.c_str(), static_cast<int>(initial));

	lv_obj_add_event_cb(slider, sliderCb, LV_EVENT_VALUE_CHANGED, new std::tuple<lv_obj_t*, std::string, std::string>(label, key, fmt));

	return { slider, label };
}

EspressoSettingsTab::EspressoSettingsTab(lv_obj_t* parent)
: m_parent(parent)
{
	lv_obj_set_flex_flow(m_parent, LV_FLEX_FLOW_ROW);

	static lv_coord_t parent_grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t parent_grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	lv_obj_set_grid_dsc_array(parent, parent_grid_col_dsc, parent_grid_row_dsc);

	lv_obj_t* boilerSettingsContainer = lv_obj_create(parent);
	lv_obj_set_size(boilerSettingsContainer, 760, 110);

	lv_obj_t* boilerPIDContainer = lv_obj_create(parent);
	lv_obj_set_size(boilerPIDContainer, 760, 130);

	lv_obj_t* pumpSettingsContainer = lv_obj_create(parent);
	lv_obj_set_size(pumpSettingsContainer, 760, 65);

	lv_obj_t* pumpPIDContainer = lv_obj_create(parent);
	lv_obj_set_size(pumpPIDContainer, 760, 130);

	lv_obj_set_grid_cell(boilerSettingsContainer,  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(boilerPIDContainer, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_cell(pumpSettingsContainer, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(pumpPIDContainer, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 3, 1);

	auto& settings = SettingsManager::get();

	// Temperature Settings Container
	auto [slider1, sliderlabel] = createSlider(boilerSettingsContainer, "BrewTemp", {85, 100}, "%d°c");
	auto [slider2, sliderlabel2] = createSlider(boilerSettingsContainer, "SteamTemp", {120, 150}, "%d°c");

	static lv_coord_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	lv_obj_set_grid_dsc_array(boilerSettingsContainer, grid_col_dsc, grid_row_dsc);

	lv_obj_set_grid_cell(createLabel(boilerSettingsContainer, "Brew Temp."),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(slider1, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(sliderlabel, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

	lv_obj_set_grid_cell(createLabel(boilerSettingsContainer, "Steam Temp."),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(slider2, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(sliderlabel2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);


	// BoilerPID Container
	static lv_coord_t boilerPID_grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t boilerPID_grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

	lv_obj_set_grid_dsc_array(boilerPIDContainer, boilerPID_grid_col_dsc, boilerPID_grid_row_dsc);

	auto [slider4, sliderlabel4] = createSlider(boilerPIDContainer, "BoilerKp", {1, 500}, "%d");
	lv_obj_set_grid_cell(createLabel(boilerPIDContainer, "Kp Term"),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(slider4, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(sliderlabel4, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

	auto [slider5, sliderlabel5] = createSlider(boilerPIDContainer, "BoilerKi", {1, 500}, "%d");
	lv_obj_set_grid_cell(createLabel(boilerPIDContainer, "Ki Term"),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_cell(slider5, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_cell(sliderlabel5, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

	auto [slider6, sliderlabel6] = createSlider(boilerPIDContainer, "BoilerKd", {1, 500}, "%d");
	lv_obj_set_grid_cell(createLabel(boilerPIDContainer, "Kd Term"),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(slider6, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(sliderlabel6, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);

	// Pressure Settings Container
	static lv_coord_t grid2_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t grid2_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	lv_obj_set_grid_dsc_array(pumpSettingsContainer, grid2_col_dsc, grid2_row_dsc);

	auto [slider3, sliderlabel3] = createSlider(pumpSettingsContainer, "BrewPressure", {6, 12}, "%d bar");
	lv_obj_set_grid_cell(createLabel(pumpSettingsContainer, "Brew Pressure"),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(slider3, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(sliderlabel3, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

	// PumpPID Container
	static lv_coord_t pumpPID_grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t pumpPID_grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

	lv_obj_set_grid_dsc_array(pumpPIDContainer, pumpPID_grid_col_dsc, pumpPID_grid_row_dsc);

	auto [pumpKpSlider, pumpKpLabel] = createSlider(pumpPIDContainer, "PumpKp", {1, 500}, "%d");
	lv_obj_set_grid_cell(createLabel(pumpPIDContainer, "Kp Term"),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(pumpKpSlider, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(pumpKpLabel, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

	auto [pumpKiSlider, pumpKiLabel] = createSlider(pumpPIDContainer, "PumpKi", {1, 500}, "%d");
	lv_obj_set_grid_cell(createLabel(pumpPIDContainer, "Ki Term"),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_cell(pumpKiSlider, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_cell(pumpKiLabel, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

	auto [pumpKdSlider, pumpKdLabel] = createSlider(pumpPIDContainer, "PumpKd", {1, 500}, "%d");
	lv_obj_set_grid_cell(createLabel(pumpPIDContainer, "Kd Term"),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(pumpKdSlider, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(pumpKdLabel, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
}
