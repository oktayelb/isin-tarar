#pragma once

#include "color.h"
#include <string>
#include <vector>


namespace output {

    void save(const std::string& file_name, int width, int height, const std::vector<Color>& pixels);
}
