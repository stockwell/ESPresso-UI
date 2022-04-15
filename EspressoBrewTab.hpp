#pragma once

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "BoilerController.hpp"

class EspressoBrewTab : public BoilerTemperatureDelegate
{
public:
    EspressoBrewTab(lv_obj_t* parent);
    virtual ~EspressoBrewTab() = default;

    void setBoiler(BoilerController* boiler);

    // BoilerTemperatureDelegate i/f
    void onBoilerCurrentTempChanged(float temp) override;
    void onBoilerTargetTempChanged(float temp) override;

private:
    enum
    {
        indic_temp,
        indic_arc,
    };

    lv_obj_t*               m_meter1;
    lv_meter_indicator_t*   m_indic[2];

    lv_obj_t*               m_sw1;

    BoilerController*       m_boilerController;
};
