#include "EspressoUI.hpp"

void EspressoUI::init(BoilerController* boiler)
{
	DisplaySize disp_size = DisplaySize::Large;

	if (LV_HOR_RES <= 320)
		disp_size = DisplaySize::Small;
	else if (LV_HOR_RES < 720)
		disp_size = DisplaySize::Medium;

	lv_coord_t tab_h;
	const lv_font_t* font_large = LV_FONT_DEFAULT;
	const lv_font_t* font_normal = LV_FONT_DEFAULT;

	switch (disp_size)
	{
		case DisplaySize::Large:
			tab_h = 70;
			font_large = &lv_font_montserrat_20;
			font_normal = &lv_font_montserrat_18;
			break;

		case DisplaySize::Medium:
			tab_h = 45;
			font_large = &lv_font_montserrat_16;
			font_normal = &lv_font_montserrat_14;
			break;

		case DisplaySize::Small:
			tab_h = 45;
			font_large = &lv_font_montserrat_16;
			font_normal = &lv_font_montserrat_14;
			break;
	}

	lv_theme_default_init(NULL,
		lv_palette_main(LV_PALETTE_BLUE),
		lv_palette_main(LV_PALETTE_RED),
		LV_THEME_DEFAULT_DARK,
		font_normal);

	auto* tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);

	lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tv);
	lv_obj_set_style_pad_left(tab_btns, LV_HOR_RES / 2, 0);

	lv_obj_t* logo = lv_img_create(tab_btns);
	LV_IMG_DECLARE(espresso_logo);
	lv_img_set_src(logo, &espresso_logo);
	lv_obj_align(logo, LV_ALIGN_LEFT_MID, -LV_HOR_RES / 2 + 5, 2);

	lv_obj_t* label1 = lv_label_create(tab_btns);
	lv_label_set_text(label1, "ESPresso v0.01");
	lv_obj_align_to(label1, logo, LV_ALIGN_OUT_RIGHT_TOP, -55, 10);
	lv_obj_set_style_text_font(label1, font_large, 0);

	lv_obj_t* label2 = lv_label_create(tab_btns);
	lv_label_set_text(label2, "Gaggia Classic Pro");
	lv_obj_set_style_text_font(label2, font_normal, 0);
	lv_obj_set_style_text_opa(label2, LV_OPA_70, 0);
	lv_obj_align_to(label2, logo, LV_ALIGN_OUT_RIGHT_TOP, -35, 32);

	lv_obj_t* t1 = lv_tabview_add_tab(tv, "Brew");
	lv_obj_t* t2 = lv_tabview_add_tab(tv, "Settings");

	lv_obj_set_style_text_font(tv, font_large, 0);

	m_brewTab = std::make_unique<EspressoBrewTab>(t1, boiler);
	m_settingsTab = std::make_unique<EspressoSettingsTab>(t2);
}
