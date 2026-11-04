#pragma once

#include "ray.h"
#include "color.h"
#include "scene_parser.h"
#include "scene.h"

class Tracer {

    const Scene& scene;
    const parser::SceneConfig& config;

public:

    Tracer(const Scene& scene, const parser::SceneConfig& config);

    Color trace(const Ray& ray, int depth) const;

private:

    Color shade(const Ray& ray, const HitRecord& hit, int depth) const;
    bool is_shadowed(const Vector3D& point, const Vector3D& normal, const Vector3D& light) const;

};
