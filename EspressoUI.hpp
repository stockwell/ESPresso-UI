#pragma once

#include <memory>

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "BoilerController.hpp"
#include "EspressoBrewTab.hpp"

class EspressoUI
{
public:
    EspressoUI()  = default;
    ~EspressoUI() = default;

    void init(BoilerController* boiler);

private:
    enum class DisplaySize {
        Small,
        Medium,
        Large,
    };

    DisplaySize         disp_size;
    const lv_font_t*    font_large;
    const lv_font_t*    font_normal;
    lv_style_t          style_text_muted;
    lv_style_t          style_title;

    lv_obj_t*           tv;

    std::unique_ptr<EspressoBrewTab>     m_brewTab;
};
