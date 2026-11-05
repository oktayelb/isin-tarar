#include "camera.h"
#include "ppm_writer.h"
#include "scene_parser.h"
#include "surface.h"
#include "tracer.h"
#include "vector3d.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using InputFile = std::string;

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "usage: ./raytracer scene.json" << std::endl;
        return 1;
    }

    InputFile scene_file = argv[1];
    parser::SceneConfig config = parser::parse(scene_file);

    auto scene = Scene(config);
    auto tracer = Tracer(scene, config);

    for (const parser::CameraConfig& camera_config : config.cameras) {

        std::vector<Color> rgb_values(camera_config.image_height * camera_config.image_width);
        auto camera = Camera(camera_config);
        int core_count = std::thread::hardware_concurrency();
        core_count = core_count != 0 ? core_count : 4;

        auto render_rows = [&rgb_values, &camera, &tracer, &camera_config, core_count](int start_row) {
            for (int i = start_row; i < camera_config.image_height; i += core_count) {
                for (int j = 0; j < camera_config.image_width; j++) {

                    auto ray = camera.shoot_ray(i, j);
                    auto color_at_pixel = tracer.trace(ray, 0);

                    rgb_values[i * camera_config.image_width + j] = color_at_pixel;
                }
            }
        };

        std::vector<std::thread> workers;
        workers.reserve(core_count);

        for (int k = 0; k < core_count; k++)
            workers.emplace_back(render_rows, k);

        for (std::thread& t : workers)
            t.join();

        output::save(camera_config.image_name, camera_config.image_width, camera_config.image_height,
                     rgb_values);
    }

    return 0;
}