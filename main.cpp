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

        auto scene = Scene(config);
    auto tracer = Tracer(scene, config);



    for (const parser::CameraConfig& camera_config : config.cameras) {
        std::vector<Color> rgb_values(camera_config.image_height*camera_config.image_width);
        auto camera = Camera(camera_config);
        for (int i = 0; i < camera_config.image_height; i++){
            for (int j = 0; j < camera_config.image_width; j++){

                auto ray = camera.shoot_ray(i,j);
                auto color_at_pixel = tracer.trace(ray,0);
                
                rgb_values[i*camera_config.image_width +j] = color_at_pixel;
                
            }
        }

        output::save(camera_config.image_name, camera_config.image_width, camera_config.image_height, rgb_values);
    }

    return 0;
}