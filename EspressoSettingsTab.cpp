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
	static lv_coord_t parent_grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	lv_obj_set_grid_dsc_array(parent, parent_grid_col_dsc, parent_grid_row_dsc);

	lv_obj_t* cont = lv_obj_create(parent);
	lv_obj_set_size(cont, 760, 110);

	lv_obj_t* cont2 = lv_obj_create(parent);
	lv_obj_set_size(cont2, 760, 65);

	lv_obj_t* boilerPID_container = lv_obj_create(parent);
	lv_obj_set_size(boilerPID_container, 760, 130);

	lv_obj_set_grid_cell(cont,  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(cont2, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_cell(boilerPID_container, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);

	auto& settings = SettingsManager::get();

	// Temperature Settings Container
	auto [slider1, sliderlabel] = createSlider(cont, "BrewTemp", {85, 100}, "%d°c");
	auto [slider2, sliderlabel2] = createSlider(cont, "SteamTemp", {120, 150}, "%d°c");

	static lv_coord_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);

	lv_obj_set_grid_cell(createLabel(cont, "Brew Temp."),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(slider1, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(sliderlabel, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

	lv_obj_set_grid_cell(createLabel(cont, "Steam Temp."),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(slider2, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(sliderlabel2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);

	// Pressure Settings Container
	static lv_coord_t grid2_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t grid2_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	lv_obj_set_grid_dsc_array(cont2, grid2_col_dsc, grid2_row_dsc);

	auto [slider3, sliderlabel3] = createSlider(cont2, "BrewPressure", {6, 12}, "%d bar");
	lv_obj_set_grid_cell(createLabel(cont2, "Brew Pressure"),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(slider3, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(sliderlabel3, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

	// BoilerPID Container
	static lv_coord_t boilerPID_grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	static lv_coord_t boilerPID_grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

	lv_obj_set_grid_dsc_array(boilerPID_container, boilerPID_grid_col_dsc, boilerPID_grid_row_dsc);

	auto [slider4, sliderlabel4] = createSlider(boilerPID_container, "BoilerKp", {1, 500}, "%d");
	lv_obj_set_grid_cell(createLabel(boilerPID_container, "Kp Term"),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(slider4, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(sliderlabel4, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

	auto [slider5, sliderlabel5] = createSlider(boilerPID_container, "BoilerKi", {1, 500}, "%d");
	lv_obj_set_grid_cell(createLabel(boilerPID_container, "Ki Term"),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_cell(slider5, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_cell(sliderlabel5, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

	auto [slider6, sliderlabel6] = createSlider(boilerPID_container, "BoilerKd", {1, 500}, "%d");
	lv_obj_set_grid_cell(createLabel(boilerPID_container, "Kd Term"),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(slider6, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(sliderlabel6, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
}
