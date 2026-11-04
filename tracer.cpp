#include "tracer.h"
#include <cmath>
#include <limits>

Tracer::Tracer(const Scene& scene, const parser::SceneConfig& config)
    : scene(scene), config(config) {}

Color Tracer::trace(const Ray& ray, int depth)  const {}

Color Tracer::shade(const Ray& ray, const HitRecord& hit, int depth) const {}

bool Tracer::is_shadowed(const Vector3D& point, const Vector3D& normal, const Vector3D& light) {}
