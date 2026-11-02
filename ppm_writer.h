#pragma once

#include <string>
#include <vector>


namespace output {

    void save(const std::string& file_name, int width, int height, const std::vector<float>& rgb_values);
}
