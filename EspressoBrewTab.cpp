#include "EspressoBrewTab.hpp"
#include "SettingsManager.hpp"

namespace
{
    static lv_style_t style_title;
    static lv_style_t style_bullet;

    constexpr int kTimerPeriodMs = 50;
    constexpr int kShotTimeSec = 5;
    constexpr int kArcMax = (1000 / kTimerPeriodMs) * kShotTimeSec + 1; 
    constexpr int kArcAngleIncrement = 360 / (kArcMax);

    static void timer_cb(lv_timer_t* t)
    {
        auto [resetSwitch, arc, time] = *static_cast<std::tuple<lv_obj_t*, lv_obj_t*, uint64_t*>*>(t->user_data);

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

    static void temp_switch_event_cb(lv_event_t* e)
    {
        BoilerController* boiler = (BoilerController*)e->user_data;
        lv_obj_t* obj = lv_event_get_target(e);

        auto& settings = SettingsManager::get();

        bool steam = lv_obj_has_state(obj, LV_STATE_CHECKED);
        boiler->setBoilerTargetTemp(steam ? settings["SteamTemp"].getAs<int>() : settings["BrewTemp"].getAs<int>());

        lv_obj_t* label = lv_obj_get_child(obj, 0);
        lv_label_set_text_fmt(label, "%s", steam ? "Steam" : "Brew");
    }


    struct ResetSwitchData 
    {
        lv_obj_t*   switch1;
        lv_obj_t*   arc;
        lv_timer_t* timer;
        uint64_t*   time;
    };



    static void reset_switch_event_cb(lv_event_t* e)
    {
        if (e->code != LV_EVENT_RELEASED)
            return;

        auto data = *static_cast<ResetSwitchData*>(lv_event_get_user_data(e));

        // reset time
        *(data.time) = 0;

        // pause timer
        lv_timer_reset(data.timer);
        lv_timer_pause(data.timer);

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

        auto [switch1, timer] = *static_cast<std::pair<lv_obj_t*, lv_timer_t*>*>(lv_event_get_user_data(e));

        if (lv_obj_has_state(obj, LV_STATE_CHECKED))
        {
            lv_label_set_text(label, "Stop");

            if (lv_obj_get_state(switch1) == LV_STATE_DISABLED)
                lv_obj_clear_state(switch1, LV_STATE_DISABLED);


            lv_timer_resume(timer);
        }
        else
        {
            lv_label_set_text(label, "Start");

            lv_timer_pause(timer);
        }
    }

    static lv_obj_t* create_meter_box(lv_obj_t* parent, const char* text1, const char* text2)
    {
        lv_obj_t* cont = lv_obj_create(parent);
        lv_obj_set_size(cont, 215, 240);
        lv_obj_set_style_pad_row(cont, 0, 0);

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

    // Panel 1 -- Temperature Guage
    m_meter1 = create_meter_box(parent, "Current", "Target");
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
    m_indic[indic_arc] = lv_meter_add_arc(m_meter1, scale, 6, lv_palette_main(LV_PALETTE_GREEN), -2);

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
    lv_obj_set_size(m_meter1, 170, 170);

    // Panel 2 - Timer and brew/steam setting
    lv_obj_t* panel2 = lv_obj_create(parent);
    lv_obj_set_size(panel2, 215, 240);
    lv_obj_set_style_pad_row(panel2, 13, 0);

#if 0
    auto switch1 = lv_btn_create(panel2);
    lv_obj_align(switch1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(switch1, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(switch1, LV_SIZE_CONTENT);

    auto swLabel1 = lv_label_create(switch1);
    lv_label_set_text(swLabel1, "Brew");
    lv_obj_center(swLabel1);
#endif

    m_switch2 = lv_btn_create(panel2);
    lv_obj_align(m_switch2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_state(m_switch2, LV_STATE_DISABLED);
    lv_obj_add_flag(m_switch2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(m_switch2, LV_SIZE_CONTENT);

    auto swLabel2 = lv_label_create(m_switch2);
    lv_label_set_text(swLabel2, "Start");
    lv_obj_center(swLabel2);

    auto switch3 = lv_btn_create(panel2);
    lv_obj_align(switch3, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_state(switch3, LV_STATE_DISABLED);
    lv_obj_set_height(switch3, LV_SIZE_CONTENT);

    auto swLabel3 = lv_label_create(switch3);
    lv_label_set_text(swLabel3, "Reset");
    lv_obj_center(swLabel3);

    lv_obj_t* arc = lv_arc_create(panel2);

    lv_arc_set_rotation(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_angles(arc, 0, 0);
    lv_arc_set_range(arc, 0, kArcMax);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
    lv_obj_center(arc);
    lv_obj_set_size(arc, 155, 155);

    m_arcLabel = lv_label_create(arc);
    lv_label_set_text(m_arcLabel, "Heating");
    lv_obj_center(m_arcLabel);
    lv_obj_set_style_text_font(m_arcLabel, &lv_font_montserrat_28, 0);

    static lv_coord_t grid_col_dsc[] = {30, 60, 60, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(panel2, grid_col_dsc, grid_row_dsc);
    lv_obj_set_grid_cell(arc, LV_GRID_ALIGN_END, 0, 3, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(m_switch2, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_set_grid_cell(switch3, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    auto* timerData = new std::tuple<lv_obj_t*, lv_obj_t*, uint64_t*>(switch3, arc, &m_stopwatchTime);
    m_timer = lv_timer_create(timer_cb, kTimerPeriodMs, timerData);
    lv_timer_pause(m_timer);

    auto* resetSwitchData = new ResetSwitchData
    {
        m_switch2,
        arc,
        m_timer,
        &m_stopwatchTime,
    };

    lv_obj_add_event_cb(switch3, reset_switch_event_cb, LV_EVENT_ALL, resetSwitchData);

    auto* timerSwitchData = new std::pair<lv_obj_t*, lv_timer_t*>(switch3, m_timer);
    lv_obj_add_event_cb(m_switch2, timer_switch_event_cb, LV_EVENT_ALL, timerSwitchData);

    m_boilerController->registerBoilerTemperatureDelegate(this);
}

void EspressoBrewTab::onBoilerTargetTempChanged(float temp)
{
    m_targetTemp = temp;

    auto round = [](int val) { val = val + 5/2; val -= val % 5; return val; };
    auto start = round(temp - 5);
    auto end = round(temp + 5);

    for (auto i = 0; i < 1; i++)
    {
        lv_meter_set_indicator_start_value(m_meter1, m_indic[indic_arc + i], start);
        lv_meter_set_indicator_end_value(m_meter1,  m_indic[indic_arc + i], end);
    }

    lv_obj_t* card = lv_obj_get_parent(m_meter1);
    lv_obj_t* label = lv_obj_get_child(card, -1);
    lv_label_set_text_fmt(label, "Target %d°c", (int)temp);
}

void EspressoBrewTab::onBoilerCurrentTempChanged(float temp)
{
    lv_meter_set_indicator_end_value(m_meter1, m_indic[indic_temp], static_cast<int>(temp));

    lv_obj_t* card = lv_obj_get_parent(m_meter1);
    lv_obj_t* label = lv_obj_get_child(card, -3);
    lv_label_set_text_fmt(label, "Current %d°c", (int)temp);

    // TODO: BoilerController should handle this, and notify on state change
    switch (m_boilerState)
    {
        case BoilerController::BoilerState::Heating:
            if (temp >= m_targetTemp)
            {
                m_boilerState = BoilerController::BoilerState::Ready;
                lv_label_set_text(m_arcLabel, "Ready");
                lv_obj_clear_state(m_switch2, LV_STATE_DISABLED);
            }
            break;

        case BoilerController::BoilerState::Ready:
            break;
    }

}
