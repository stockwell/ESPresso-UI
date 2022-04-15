#pragma once

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

class EspressoSettingsTab 
{
public:
    EspressoSettingsTab(lv_obj_t* parent);
    ~EspressoSettingsTab() = default;

private:
    lv_obj_t*   m_parent;
};
