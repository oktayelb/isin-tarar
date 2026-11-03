#pragma once
#include "surface.h"
#include "scene_parser.h"
#include <memory>
#include <vector>

// All the geometry of a scene, built from the parsed config.
class Scene {
    std::vector<std::unique_ptr<Surface>> surfaces;

public:
    explicit Scene(const parser::SceneConfig& config);

    // Closest hit with t_min < t < t_max. Use for camera rays and reflection rays.
    bool closest_hit(const Ray& ray, float t_min, float t_max, HitRecord& hit) const;

    // True as soon as anything is hit with t_min < t < t_max. Use for shadow rays:
    // with direction = light - point, pass t_max = 1 so only blockers before the light count.
    bool any_hit(const Ray& ray, float t_min, float t_max) const;
};
