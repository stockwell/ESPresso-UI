#include "EspressoBrewTab.hpp"

static lv_style_t style_title;
static lv_style_t style_bullet;
static const lv_font_t* font_large;
static lv_obj_t * chart1;
static lv_chart_series_t * ser1;

static void switch_event_cb(lv_event_t* e)
{
    BoilerController* boiler = (BoilerController*)e->user_data;
    lv_obj_t * obj = lv_event_get_target(e);

    bool steam = lv_obj_has_state(obj, LV_STATE_CHECKED);
    boiler->setBoilerTargetTemp(steam ? 130 : 93);

}

static lv_obj_t * create_meter_box(lv_obj_t * parent, const char * title, const char * text1, const char * text2, const char * text3)
{
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 210, 240);
    //lv_obj_set_flex_grow(cont, 1);
#if 0
    lv_obj_t * title_label = lv_label_create(cont);
    lv_label_set_text(title_label, title);
    lv_obj_add_style(title_label, &style_title, 0);
    #endif

    lv_obj_t * meter = lv_meter_create(cont);
    lv_obj_remove_style(meter, NULL, LV_PART_MAIN);

    lv_obj_t * bullet1 = lv_obj_create(cont);
    lv_obj_set_size(bullet1, 13, 13);
    lv_obj_remove_style(bullet1, NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet1, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet1, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_t * label1 = lv_label_create(cont);
    lv_label_set_text(label1, text1);

    lv_obj_t * bullet2 = lv_obj_create(cont);
    lv_obj_set_size(bullet2, 13, 13);
    lv_obj_remove_style(bullet2, NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet2, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet2, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_t * label2 = lv_label_create(cont);
    lv_label_set_text(label2, text2);

    static lv_coord_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
    lv_obj_set_grid_cell(meter, LV_GRID_ALIGN_START, 0, 4, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_grid_cell(bullet1, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
    lv_obj_set_grid_cell(bullet2, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_START, 1, 1);
    lv_obj_set_grid_cell(label1, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 1, 1);
    lv_obj_set_grid_cell(label2, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_START, 1, 1);

    return meter;

}

EspressoBrewTab::EspressoBrewTab(lv_obj_t* parent)
: m_parent(parent)
{
    lv_obj_set_flex_flow(m_parent, LV_FLEX_FLOW_ROW_WRAP);

    font_large = LV_FONT_DEFAULT;

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);

    lv_style_init(&style_bullet);
    lv_style_set_border_width(&style_bullet, 0);
    lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

    m_meter1 = create_meter_box(parent, "Temperature", "Current", "Target", "");
    lv_obj_add_flag(lv_obj_get_parent(m_meter1), LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_center(m_meter1);

    /*Add a special circle to the needle's pivot*/
    lv_obj_set_style_size(m_meter1, 15, LV_PART_INDICATOR);
    lv_obj_set_style_radius(m_meter1, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(m_meter1, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(m_meter1, lv_palette_darken(LV_PALETTE_GREY, 4), LV_PART_INDICATOR);

    lv_meter_scale_t* scale = lv_meter_add_scale(m_meter1);
    lv_meter_set_scale_ticks(m_meter1, scale, 41, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(m_meter1, scale, 8, 4, 15, lv_color_black(), 10);

    lv_meter_set_scale_range(m_meter1, scale, 0, 200, 270, 135);

    m_indic[indic_temp] = lv_meter_add_needle_line(m_meter1, scale, 4, lv_palette_main(LV_PALETTE_RED), -10);

    m_indic[indic_range_0] = lv_meter_add_arc(m_meter1, scale, 5, lv_palette_main(LV_PALETTE_GREEN), 0);
    m_indic[indic_range_1] = lv_meter_add_scale_lines(m_meter1, scale, lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_GREEN), false, 0);

    lv_meter_indicator_t* indic = lv_meter_add_arc(m_meter1, scale, 3, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_meter_set_indicator_start_value(m_meter1, indic, 0);
    lv_meter_set_indicator_end_value(m_meter1, indic, 40);

    indic = lv_meter_add_scale_lines(m_meter1, scale, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_BLUE), false, 1);
    lv_meter_set_indicator_start_value(m_meter1, indic, 0);
    lv_meter_set_indicator_end_value(m_meter1, indic, 40);

    indic = lv_meter_add_arc(m_meter1, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(m_meter1, indic, 160);
    lv_meter_set_indicator_end_value(m_meter1, indic, 200);

    indic = lv_meter_add_scale_lines(m_meter1, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
    lv_meter_set_indicator_start_value(m_meter1, indic, 160);
    lv_meter_set_indicator_end_value(m_meter1, indic, 200);

    indic = lv_meter_add_scale_lines(m_meter1, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
    lv_meter_set_indicator_start_value(m_meter1, indic, 160);
    lv_meter_set_indicator_end_value(m_meter1, indic, 200);

    lv_obj_update_layout(parent);
    lv_coord_t meter_w = lv_obj_get_width(m_meter1);
    lv_obj_set_size(m_meter1, 170, 170);

    lv_obj_t* panel2 = lv_obj_create(parent);
    m_sw1 = lv_switch_create(panel2);
}

void EspressoBrewTab::setBoiler(BoilerController* boiler)
{
    m_boilerController = boiler;
    m_boilerController->registerBoilerTemperatureDelegate(this);

    lv_obj_add_event_cb(m_sw1, switch_event_cb, LV_EVENT_ALL, (void*)boiler);
}

void EspressoBrewTab::onBoilerTargetTempChanged(float temp)
{
    //auto round = [](int val) { val + 10/2; val -= val % 10; return val; };
    auto start = 0;//round(temp - 10);
    auto end = 0;//round(temp + 10);

    for (auto i = 0; i < 2; i++)
    {
        lv_meter_set_indicator_start_value(m_meter1, m_indic[indic_range_0 + i], start);
        lv_meter_set_indicator_end_value(m_meter1,  m_indic[indic_range_0 + i], end);
    }

    lv_obj_t * card = lv_obj_get_parent(m_meter1);
    lv_obj_t * label = lv_obj_get_child(card, -1);
    lv_label_set_text_fmt(label, "Target %d°c", (int)temp);
}

void EspressoBrewTab::onBoilerCurrentTempChanged(float temp)
{
    lv_meter_set_indicator_end_value(m_meter1, m_indic[indic_temp], static_cast<int>(temp));

    lv_obj_t * card = lv_obj_get_parent(m_meter1);
    lv_obj_t * label = lv_obj_get_child(card, -3);
    lv_label_set_text_fmt(label, "Current %d°c", (int)temp);


}
