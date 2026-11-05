#include "tracer.h"
#include <cmath>
#include <limits>
#include <algorithm>

Tracer::Tracer(const Scene& scene, const parser::SceneConfig& config)
    : scene(scene), config(config) {}

Color Tracer::trace(const Ray& ray, int depth)  const {

    auto record =  HitRecord();
    auto is_hit = scene.closest_hit(ray,0.0f, std::numeric_limits<float>::infinity(),record);
    
    if (is_hit)
        return shade(ray,record,depth);
    if (depth == 0)
        return config.background_color;
    
    return Color(0,0,0);

}

Color Tracer::shade(const Ray& ray, const HitRecord& hit, int depth) const {

    const auto& material = config.materials[hit.material_index];
    auto result_color = Color();
    auto a_light = config.ambient_light;
    result_color = material.ambient_reflectance*a_light;
    auto v = -(ray.direction).normalize();
    for ( auto light : config.point_lights){

        if (is_shadowed(hit.point, hit.normal, light.position))
            continue;

        auto to_light = light.position - hit.point;
        auto d = to_light.length();
        auto irradiance  =  light.intensity* (1.0f / (d*d));
        auto normalized_to_light = to_light.normalize();
        auto angle = dot_product(hit.normal,normalized_to_light);
        if (angle  <= 0)
            continue;
        result_color = result_color + material.diffuse_reflectance*irradiance*angle;
    

        auto h = (normalized_to_light + v).normalize();

        result_color = result_color +  material.specular_reflectance*irradiance*pow(std::max(0.0f,dot_product(hit.normal,h)),material.phong_exponent);

  }


    
    if (material.is_mirror && depth < config.max_recursion_depth){
        auto reflection = -v -hit.normal.scale(2*dot_product(-v,hit.normal));
        auto offset =       hit.point + hit.normal.scale(config.shadow_ray_epsilon);
        result_color = result_color + material.mirror_reflectance * trace(Ray(offset,reflection), depth + 1);
    }


    return result_color;
}


bool Tracer::is_shadowed(const Vector3D& point, const Vector3D& normal, const Vector3D& light) const {

    auto shadow_ray_epsilon = config.shadow_ray_epsilon;
    auto offset_origin  =point + normal.scale(shadow_ray_epsilon);
    auto ray = Ray(offset_origin, light - offset_origin );

    return scene.any_hit(ray, 0, 1.0f);
}
