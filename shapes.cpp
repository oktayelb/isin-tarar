#include "shapes.h"
#include <algorithm>
#include <cmath>



Sphere::Sphere(const Vector3D& center, float radius, int material_index)
    : Surface(material_index), center(center), radius(radius), radius_squared(radius * radius) {}

bool Sphere::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    Vector3D oc = ray.origin - center;
    float a = dot_product(ray.direction, ray.direction);
    float half_b = dot_product(ray.direction, oc);
    float c = dot_product(oc, oc) - radius_squared;

    float disc = half_b * half_b - a * c;
    if (disc < 0) {
        return false;
    }

    float sqrt_d = std::sqrt(disc);

    float t = (-half_b - sqrt_d) / a;
    if (t <= t_min || t >= t_max) {
        t = (-half_b + sqrt_d) / a;
        if (t <= t_min || t >= t_max) {
            return false;
        }
    }

    hit.t = t;
    hit.point = ray.origin + ray.direction.scale(t);
    hit.normal = (hit.point - center).scale(1.0f / radius);
    hit.material_index = material_index;
    return true;
}



Plane::Plane(const Vector3D& point, const Vector3D& normal, int material_index)
    : Surface(material_index), point(point), normal(normal.normalize()),
      offset(dot_product(this->normal, point)) {}

bool Plane::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    float denominator = dot_product(normal, ray.direction);
    if (std::fabs(denominator) < 1e-8f) {
        return false;
    }

    float t = (offset - dot_product(normal, ray.origin)) / denominator;
    if (t <= t_min || t >= t_max) {
        return false;
    }

    hit.t = t;
    hit.point = ray.origin + ray.direction.scale(t);
    hit.normal = normal;
    hit.material_index = material_index;
    return true;
}


Triangle::Triangle(const Vector3D& a, const Vector3D& b, const Vector3D& c, int material_index)
    : Surface(material_index), a(a), b(b), c(c),
      side_reference(cross_product(c - b, a - b)),
      normal(side_reference.normalize()),
      edge_tolerance(-1e-6f * dot_product(side_reference, side_reference)) {}

bool Triangle::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    const Vector3D& d = ray.direction;

    float denominator = dot_product(d, normal);
    if (std::fabs(denominator) < 1e-8f) {
        return false;
    }
    float t = dot_product(a - ray.origin, normal) / denominator;
    if (t <= t_min || t >= t_max) {
        return false;
    }
    Vector3D p = ray.origin + d.scale(t);
    if (dot_product(cross_product(p - b, a - b), side_reference) < edge_tolerance) return false;  // same side of ab as c?
    if (dot_product(cross_product(p - c, b - c), side_reference) < edge_tolerance) return false;  // same side of bc as a?
    if (dot_product(cross_product(p - a, c - a), side_reference) < edge_tolerance) return false;  // same side of ca as b?

    hit.t = t;
    hit.point = p;
    hit.normal = normal;
    hit.material_index = material_index;
    return true;
}


Mesh::Mesh(std::vector<Triangle> triangle_list, int material_index)
    : Surface(material_index), triangles(std::move(triangle_list)) {}

bool Mesh::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    bool found = false;
    for (const Triangle& triangle : triangles) {
        if (triangle.intersect(ray, t_min, t_max, hit)) {
            found = true;
            t_max = hit.t;
        }
    }

    if (found) hit.material_index = material_index;
    return found;
}



Cylinder::Cylinder(const Vector3D& center, const Vector3D& axis, float radius, float height, int material_index)
    : Surface(material_index), center(center), axis(axis.normalize()),
      radius(radius), radius_squared(radius * radius), half_height(height / 2.0f) {}

bool Cylinder::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    const Vector3D& d = ray.direction;
    Vector3D oc = ray.origin - center;

    float oc_along = dot_product(oc, axis);
    float d_along = dot_product(d, axis);
    Vector3D oc_perp = oc - axis.scale(oc_along);
    Vector3D d_perp = d - axis.scale(d_along);

    bool found = false;
    float best_t = t_max;
    Vector3D best_normal;
    float a = dot_product(d_perp, d_perp);
    if (a > 1e-12f) { 
        float half_b = dot_product(d_perp, oc_perp);
        float c = dot_product(oc_perp, oc_perp) - radius_squared;
        float discriminant = half_b * half_b - a * c;
        if (discriminant >= 0) {
            float sqrt_d = std::sqrt(discriminant);
            float roots[2] = {(-half_b - sqrt_d) / a, (-half_b + sqrt_d) / a};
            for (float t : roots) {
                if (t <= t_min || t >= best_t) continue;
                float height_at_hit = oc_along + t * d_along;
                if (std::fabs(height_at_hit) > half_height) continue;
                found = true;
                best_t = t;
                best_normal = (oc_perp + d_perp.scale(t)).scale(1.0f / radius);
            }
        }
    }
  if (std::fabs(d_along) > 1e-12f) {
        for (float side : {1.0f, -1.0f}) {
            float t = (side * half_height - oc_along) / d_along;
            if (t <= t_min || t >= best_t) continue;
            Vector3D radial = oc_perp + d_perp.scale(t);
            if (dot_product(radial, radial) > radius_squared) continue;
            found = true;
            best_t = t;
            best_normal = axis.scale(side);
        }
    }

    if (!found) return false;

    hit.t = best_t;
    hit.point = ray.origin + d.scale(best_t);
    hit.normal = best_normal;
    hit.material_index = material_index;
    return true;
}
