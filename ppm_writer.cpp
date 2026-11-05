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

    void save(const std::string& file_name, int width, int height, const std::vector<Color>& pixels) {
        size_t expected_count = static_cast<size_t>(width) * height;
        if (pixels.size() != expected_count) {
            throw std::runtime_error("pixels must hold width * height values for " + file_name);
        }

        std::ofstream file(file_name);
        if (!file) {
            throw std::runtime_error("cannot open output file: " + file_name);
        }

        file << "P3\n" << width << " " << height << "\n255\n";
        for (int row = 0; row < height; row++) {
            for (int column = 0; column < width; column++) {
                const Color& pixel = pixels[static_cast<size_t>(row) * width + column];
                file << to_color_value(pixel.r) << ' '
                     << to_color_value(pixel.g) << ' '
                     << to_color_value(pixel.b);
                file << (column + 1 == width ? '\n' : ' ');
            }
        }

        if (!file) {
            throw std::runtime_error("failed while writing output file: " + file_name);
        }
    }
}
