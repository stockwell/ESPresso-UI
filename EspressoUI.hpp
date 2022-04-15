#pragma once

#include <memory>

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "BoilerController.hpp"
#include "EspressoBrewTab.hpp"
#include "EspressoSettingsTab.hpp"

class EspressoUI
{
public:
    EspressoUI() = default;
    ~EspressoUI() = default;

    void init(BoilerController* boiler);

private:
    enum class DisplaySize {
        Small,
        Medium,
        Large,
    };

    std::unique_ptr<EspressoBrewTab>     m_brewTab;
};
