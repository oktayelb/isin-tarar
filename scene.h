#pragma once
#include "surface.h"
#include "scene_parser.h"
#include <memory>
#include <vector>

class Scene {
    std::vector<std::unique_ptr<Surface>> surfaces;

public:
    explicit Scene(const parser::SceneConfig& config);
    bool closest_hit(const Ray& ray, float t_min, float t_max, HitRecord& hit) const;
    bool any_hit(const Ray& ray, float t_min, float t_max) const;
};
