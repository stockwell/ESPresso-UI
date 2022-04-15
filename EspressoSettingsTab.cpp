#include "EspressoSettingsTab.hpp"

static lv_obj_t* createLabel(lv_obj_t* parent, const char* text)
{
    auto label = lv_label_create(parent);
    lv_label_set_text(label, text);

    return label;
}

static void temperatureSliderCb(lv_event_t* e)
{
    lv_obj_t* slider = lv_event_get_target(e);
    auto [label, key] = *static_cast<std::pair<lv_obj_t*, std::string>*>(lv_event_get_user_data(e));
    auto val = lv_slider_get_value(slider);

    lv_label_set_text_fmt(label, "%d°c", val);

    SettingsManager::get()[key] = val;
}

static std::pair<lv_obj_t*, lv_obj_t*> createTemperatureSlider(lv_obj_t* parent, const std::string& key)
{
    auto slider = lv_slider_create(parent);
    auto initial = SettingsManager::get()[key].getAs<int>();

    lv_slider_set_range(slider, 50, 150);
    lv_slider_set_value(slider, initial, LV_ANIM_OFF);

    auto label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text_fmt(label, "%d°c", initial);

    lv_obj_add_event_cb(slider, temperatureSliderCb, LV_EVENT_VALUE_CHANGED, new std::pair<lv_obj_t*, std::string>(label, key));

    return { slider, label };
}

EspressoSettingsTab::EspressoSettingsTab(lv_obj_t* parent)
: m_parent(parent)
{
    lv_obj_set_flex_flow(m_parent, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 445, 240);

    auto& settings = SettingsManager::get();

    auto [slider1, sliderlabel] = createTemperatureSlider(cont, "BrewTemp");
    auto [slider2, sliderlabel2] = createTemperatureSlider(cont, "SteamTemp");

    static lv_coord_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);

    lv_obj_set_grid_cell(createLabel(cont, "Brew Temp."),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_grid_cell(slider1, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_grid_cell(sliderlabel, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_set_grid_cell(createLabel(cont, "Steam Temp."),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(slider2, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(sliderlabel2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
}