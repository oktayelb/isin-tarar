#pragma once

#include "surface.h"
#include <vector>


class Sphere : public Surface {
    Vector3D center;
    float radius;
    float radius_squared;

public:
    Sphere(const Vector3D& center, float radius, int material_index);

    bool intersect(const Ray& ray, float t_min, float t_max,
                   HitRecord& hit) const override;
};


class Plane : public Surface {
    Vector3D point;
    Vector3D normal;
    float offset;

public:
    Plane(const Vector3D& point, const Vector3D& normal, int material_index);

    bool intersect(const Ray& ray, float t_min, float t_max,
                   HitRecord& hit) const override;
};


class Triangle : public Surface {
    Vector3D a;
    Vector3D edge1; // b-a
    Vector3D edge2; // c-a
    Vector3D normal;

public:
    Triangle(const Vector3D& a, const Vector3D& b, const Vector3D& c,
             int material_index);

    bool intersect(const Ray& ray, float t_min, float t_max,
                   HitRecord& hit) const override;

    Vector3D vertex_a() const { return a; }
    Vector3D vertex_b() const { return a+edge1;}
    Vector3D vertex_c() const { return a+edge2;}
};


class Mesh : public Surface {
    std::vector<Triangle> triangles;

public:
    Mesh(std::vector<Triangle> triangles, int material_index);

    bool intersect(const Ray& ray, float t_min, float t_max,
                   HitRecord& hit) const override;

    bool any_hit(const Ray& ray, float t_min, float t_max) const override;
};


class Cylinder : public Surface {
    Vector3D center;
    Vector3D axis;
    float radius;
    float radius_squared;
    float half_height;

public:
    Cylinder(const Vector3D& center, const Vector3D& axis,
             float radius, float height, int material_index);

    bool intersect(const Ray& ray, float t_min, float t_max,
                   HitRecord& hit) const override;
};