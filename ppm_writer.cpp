#include "ppm_writer.h"

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>


namespace output {

    namespace {

        int to_color_value(float value) {
            if (std::isnan(value) || value <= 0.0f) {
                return 0;
            }
            if (value >= 255.0f) {
                return 255;
            }
            return static_cast<int>(std::lround(value));
        }
    }

    void save(const std::string& file_name, int width, int height, const std::vector<float>& rgb_values) {
        size_t values_per_row = static_cast<size_t>(width) * 3;
        if (rgb_values.size() != values_per_row * height) {
            throw std::runtime_error("rgb_values must hold width * height * 3 values for " + file_name);
        }

        std::ofstream file(file_name);
        if (!file) {
            throw std::runtime_error("cannot open output file: " + file_name);
        }

        file << "P3\n" << width << " " << height << "\n255\n";
        for (size_t i = 0; i < rgb_values.size(); i++) {
            file << to_color_value(rgb_values[i]);
            bool is_last_value_of_row = (i + 1) % values_per_row == 0;
            if (is_last_value_of_row) {
                file << '\n';
            } else {
                file << ' ';
            }
        }

        if (!file) {
            throw std::runtime_error("failed while writing output file: " + file_name);
        }
    }
}
