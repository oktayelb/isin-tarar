#include "scene_parser.h"
#include "ppm_writer.h"
#include "vector3d.h"
#include "camera.h"
#include "surface.h"
#include "tracer.h"
#include <string>
#include <vector>
#include <iostream>


using InputFile = std::string;
 
int main(int argc, char* argv[]){

    if (argc != 2) {
        std::cerr << "usage: ./raytracer scene.json" << std::endl;
        return 1;
    }

    InputFile scene_file = argv[1];
    parser::SceneConfig config = parser::parse(scene_file);

    // loop where we create the scene



    for (const parser::CameraConfig& camera_config : config.cameras) {
        std::vector<float> rgb_values(static_cast<size_t>(camera_config.image_width) * camera_config.image_height * 3);
        output::save(camera_config.image_name, camera_config.image_width, camera_config.image_height, rgb_values);
    }

    return 0;
}