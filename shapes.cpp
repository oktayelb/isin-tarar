#include "shapes.h"
#include <algorithm>
#include <cmath>


Sphere::Sphere(const Vector3D& center, float radius, int material_index)
    : Surface(material_index),
      center(center),
      radius(radius),
      radius_squared(radius*radius) {}

bool Sphere::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    Vector3D oc = ray.origin- center;

    float a = dot_product(ray.direction, ray.direction);
    float halfb = dot_product(ray.direction, oc);
    float c = dot_product(oc, oc)- radius_squared;

    float discriminant= halfb*halfb- a*c;

    if (discriminant<0) {
        return false;
    }

    float sqrt_discriminant= std::sqrt(discriminant);

    float t= (-halfb- sqrt_discriminant)/a;

    if (t <= t_min || t >= t_max) {
        t= (-halfb+sqrt_discriminant)/a;

        if (t <= t_min || t >= t_max) {
            return false;
        }
    }

    hit.t= t;
    hit.point= ray.origin+ray.direction.scale(t);
    hit.normal= (hit.point- center).scale(1.0f/radius);
    hit.material_index= material_index;

    return true;
}


Plane::Plane(const Vector3D& point, const Vector3D& normal, int material_index)
    : Surface(material_index),
      point(point),
      normal(normal.normalize()),
      offset(dot_product(this->normal, point)) {}

bool Plane::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    float denominator= dot_product(normal, ray.direction);

    if (std::fabs(denominator) < 1e-8f) {
        return false;
    }

    float t= (offset- dot_product(normal, ray.origin))/denominator;

    if (t <= t_min || t >= t_max) {
        return false;
    }

    hit.t= t;
    hit.point= ray.origin+ray.direction.scale(t);
    hit.normal= normal;
    hit.material_index= material_index;

    return true;
}


Triangle::Triangle(const Vector3D& a, const Vector3D& b, const Vector3D& c, int material_index)
    : Surface(material_index),
      a(a),
      edge1(b- a),
      edge2(c- a),
      normal(cross_product(edge1, edge2).normalize()) {}

bool Triangle::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    const float bary_tolerance= 1e-6f;
    const Vector3D& d= ray.direction;

    Vector3D p= cross_product(d, edge2);
    float det= dot_product(edge1, p);

    if (std::fabs(det) < 1e-12f) {
        return false;
    }

    float inverse_det= 1.0f/det;

    Vector3D to_origin= ray.origin- a;

    float u= dot_product(to_origin, p)*inverse_det;

    if (u < -bary_tolerance || u > 1.0f+bary_tolerance) {
        return false;
    }

    Vector3D q= cross_product(to_origin, edge1);
    float v= dot_product(d, q)*inverse_det;

    if (v< -bary_tolerance || u+v > 1.0f+bary_tolerance) {
        return false;
    }

    float t= dot_product(edge2, q)*inverse_det;

    if (t <= t_min || t >= t_max) {
        return false;
    }

    hit.t= t;
    hit.point= ray.origin+d.scale(t);
    hit.normal= normal;
    hit.material_index= material_index;

    return true;
}


Mesh::Mesh(std::vector<Triangle> triangle_list, int material_index)
    : Surface(material_index),
      triangles(std::move(triangle_list)) {}

bool Mesh::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    bool found= false;

    for (const Triangle& triangle : triangles) {
        if (triangle.intersect(ray, t_min, t_max, hit)) {
            found= true;
            t_max = hit.t;
        }
    }

    if (found) {
        hit.material_index = material_index;
    }

    return found;
}

bool Mesh::any_hit(const Ray& ray, float t_min, float t_max) const {
    HitRecord ignored;

    for (const Triangle& triangle : triangles) {
        if (triangle.intersect(ray, t_min, t_max, ignored)) {
            return true;
        }
    }

    return false;
}


Cylinder::Cylinder(const Vector3D& center, const Vector3D& axis,
                   float radius, float height, int material_index)
    : Surface(material_index),
      center(center),
      axis(axis.normalize()),
      radius(radius),
      radius_squared(radius*radius),
      half_height(height/2.0f) {}

bool Cylinder::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    const Vector3D& d = ray.direction;
    Vector3D oc = ray.origin-center;

    float oc_axis = dot_product(oc, axis);
    float d_axis = dot_product(d, axis);

    Vector3D oc_perp = oc-axis.scale(oc_axis);
    Vector3D d_perp = d-axis.scale(d_axis);

    bool found = false;
    float closest_t = t_max;
    Vector3D closest_normal;

    float a = dot_product(d_perp, d_perp);

    if (a > 1e-12f) {
        float halfb = dot_product(d_perp, oc_perp);
        float c = dot_product(oc_perp, oc_perp)-radius_squared;

        float discriminant = halfb*halfb-a*c;

        if (discriminant >= 0) {
            float sqrt_discriminant = std::sqrt(discriminant);

            float roots[2] = {
                (-halfb- sqrt_discriminant)/a,
                (-halfb+sqrt_discriminant)/a
            };

            for (float t : roots) {
                if (t <= t_min || t >= closest_t) {
                    continue;
                }

                float height_at_hit = oc_axis+t*d_axis;

                if (std::fabs(height_at_hit) > half_height) {
                    continue;
                }

                found = true;
                closest_t = t;

                Vector3D point_perp = oc_perp+d_perp.scale(t);
                closest_normal = point_perp.scale(1.0f/radius);
            }
        }
    }

    if (std::fabs(d_axis) > 1e-12f) {
        for (float side : {1.0f, -1.0f}) {
            float t = (side*half_height- oc_axis)/d_axis;

            if (t <= t_min || t >= closest_t) {
                continue;
            }

            Vector3D radial = oc_perp+d_perp.scale(t);

            if (dot_product(radial, radial) > radius_squared) {
                continue;
            }

            found = true;
            closest_t = t;
            closest_normal = axis.scale(side);
        }
    }

    if (!found) {
        return false;
    }

    hit.t = closest_t;
    hit.point = ray.origin+d.scale(closest_t);
    hit.normal = closest_normal;
    hit.material_index = material_index;

    return true;
}