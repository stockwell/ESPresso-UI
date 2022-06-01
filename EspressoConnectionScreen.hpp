#pragma once

#include "lvgl.h"

#include <string>

class EspressoConnectionScreen
{
public:
	EspressoConnectionScreen(const std::string& hostname);
	~EspressoConnectionScreen() = default;

private:
	void init(const std::string& hostname);
};
