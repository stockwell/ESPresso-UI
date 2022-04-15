#include "EspressoUI.hpp"

void EspressoUI::init(BoilerController* boiler)
{
    if (LV_HOR_RES <= 320)
        disp_size = DisplaySize::Small;
    else if (LV_HOR_RES < 720)
        disp_size = DisplaySize::Medium;
    else
        disp_size = DisplaySize::Large;

    font_large = LV_FONT_DEFAULT;
    font_normal = LV_FONT_DEFAULT;

    lv_coord_t tab_h;

    switch (disp_size)
    {
        case DisplaySize::Large:
            break;
        
        case DisplaySize::Medium:
            tab_h = 45;
            font_large     =  &lv_font_montserrat_16;
            font_normal    =  &lv_font_montserrat_14;
            break;
        
        case DisplaySize::Small:
            tab_h = 45;
            font_large     =  &lv_font_montserrat_16;
            font_normal    =  &lv_font_montserrat_14;
            break;
    }

    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, font_normal);

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);

    tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);

    if (disp_size == DisplaySize::Medium)
    {
        lv_obj_t * tab_btns = lv_tabview_get_tab_btns(tv);
        lv_obj_set_style_pad_left(tab_btns, LV_HOR_RES / 2, 0);

        lv_obj_t * logo = lv_img_create(tab_btns);
        LV_IMG_DECLARE(espresso_logo);
        lv_img_set_src(logo, &espresso_logo);
        lv_obj_align(logo, LV_ALIGN_LEFT_MID, -LV_HOR_RES / 2 + 10, 0);

        lv_obj_t* label = lv_label_create(tab_btns);
        lv_label_set_text(label, "Gaggia Classic Pro");
        lv_obj_add_style(label, &style_text_muted, 0);
        lv_obj_align_to(label, logo, LV_ALIGN_OUT_RIGHT_TOP, 5, 20);

    }

    lv_obj_t* t1 = lv_tabview_add_tab(tv, "Brew");
    lv_obj_t* t2 = lv_tabview_add_tab(tv, "Settings");

    m_brewTab = std::make_unique<EspressoBrewTab>(t1);
    m_brewTab->setBoiler(boiler);
}
