#pragma once
#include "surface.h"
#include <vector>

class Sphere : public Surface {
    Vector3D center;
    float radius;
    float radius_squared;

public:
    Sphere(const Vector3D& center, float radius, int material_index);

    bool intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const override;
};

class Plane : public Surface {
    Vector3D point;    // any point on the plane
    Vector3D normal;   // unit normal (normalized in the constructor)
    float offset;      // n . point, precomputed

public:
    Plane(const Vector3D& point, const Vector3D& normal, int material_index);

    bool intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const override;
};

class Triangle : public Surface {
    Vector3D a, b, c;      // vertices, counter-clockwise
    Vector3D side_reference;  // (c - b) x (a - b), not normalized; used by the inside test
    Vector3D normal;          // unit normal from CCW order, computed once
    float edge_tolerance;     // small negative slack for the inside test, scaled to the triangle's size

public:
    Triangle(const Vector3D& a, const Vector3D& b, const Vector3D& c, int material_index);

    bool intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const override;

    const Vector3D& vertex_a() const { return a; }
    const Vector3D& vertex_b() const { return b; }
    const Vector3D& vertex_c() const { return c; }
};

// Axis-aligned bounding box.
struct AABB {
    Vector3D min_corner;
    Vector3D max_corner;

    AABB();                                   // empty box (min = +inf, max = -inf)
    void expand(const Vector3D& point);
    void expand(const AABB& other);
    int longest_axis() const;                 // 0 = x, 1 = y, 2 = z
    bool hit(const Ray& ray, float t_min, float t_max) const;
};

// A mesh is many triangles with one material, stored in a BVH
// (bounding volume hierarchy) so a ray only tests triangles whose boxes it enters.
class Mesh : public Surface {
    struct Node {
        AABB box;
        int left = -1;     // child node indices (inner nodes)
        int right = -1;
        int first = 0;     // range of triangles [first, first + count) (leaves)
        int count = 0;     // > 0 means this node is a leaf
    };

    std::vector<Triangle> triangles;
    std::vector<Node> nodes;   // nodes[0] is the root

    int build(int first, int count);

public:
    Mesh(std::vector<Triangle> triangles, int material_index);

    bool intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const override;
};

// Finite, closed right circular cylinder: side + two caps.
class Cylinder : public Surface {
    Vector3D center;      // midpoint of the axis
    Vector3D axis;        // unit vector toward the top cap
    float radius;
    float radius_squared;
    float half_height;

public:
    Cylinder(const Vector3D& center, const Vector3D& axis, float radius, float height, int material_index);

    bool intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const override;
};
