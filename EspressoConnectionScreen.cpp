#include "EspressoConnectionScreen.hpp"

static void anim_text_opa_cb(void* var, int32_t v)
{
	auto label = static_cast<lv_obj_t*>(var);
	lv_obj_set_style_text_opa(label, v, LV_PART_MAIN);
}

EspressoConnectionScreen::EspressoConnectionScreen(const std::string& hostname)
{
	const lv_font_t* font_large  = &lv_font_montserrat_20;
	const lv_font_t* font_normal = &lv_font_montserrat_18;

	lv_theme_default_init(NULL,
		lv_palette_main(LV_PALETTE_BLUE),
		lv_palette_main(LV_PALETTE_RED),
		LV_THEME_DEFAULT_DARK,
		font_normal);

	init(hostname);
}

void EspressoConnectionScreen::init(const std::string& hostname)
{
	lv_obj_t* spinner = lv_spinner_create(lv_scr_act(), 1250, 40);
	lv_obj_set_size(spinner, 175, 175);
	lv_obj_center(spinner);

	lv_obj_t* label = lv_label_create(lv_scr_act());
	lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
	lv_label_set_text(label, std::string("Connecting to http://" + hostname + "...").c_str());
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 150);

	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, label);
	lv_anim_set_values(&a, 0xFF, 0x20);
	lv_anim_set_time(&a, 3000);
	lv_anim_set_playback_delay(&a, 100);
	lv_anim_set_playback_time(&a, 1000);
	lv_anim_set_repeat_delay(&a, 500);
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);

	lv_anim_set_exec_cb(&a, anim_text_opa_cb);
	lv_anim_start(&a);
}
