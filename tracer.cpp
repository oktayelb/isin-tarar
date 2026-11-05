#include "tracer.h"
#include <cmath>
#include <limits>

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


}


bool Tracer::is_shadowed(const Vector3D& point, const Vector3D& normal, const Vector3D& light) const {

    auto shadow_ray_epsilon = config.shadow_ray_epsilon;
    auto offset_origin  =point + normal.scale(shadow_ray_epsilon);
    auto ray = Ray(offset_origin, light - offset_origin );

    return scene.any_hit(ray, 0, 1.0f);
}
