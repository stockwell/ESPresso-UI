#include "EspressoSettingsTab.hpp"

static lv_obj_t* createSlider(lv_obj_t* parent)
{
    auto slider = lv_slider_create(parent);

    return slider;
}

static lv_obj_t* createLabel(lv_obj_t* parent, const char* text)
{
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);

    return label;
}

EspressoSettingsTab::EspressoSettingsTab(lv_obj_t* parent)
: m_parent(parent)
{
    lv_obj_set_flex_flow(m_parent, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 445, 240);

    auto slider1 = createSlider(cont);
    auto sliderLabel = createLabel(cont, "0°c");

    auto slider2 = createSlider(cont);

    static lv_coord_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);

    lv_obj_set_grid_cell(createLabel(cont, "Brew Temp."),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_grid_cell(slider1, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_grid_cell(sliderLabel, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 0, 1);

    lv_obj_set_grid_cell(createLabel(cont, "Steam Temp."),  LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(slider2, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);
}
