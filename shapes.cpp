#include "shapes.h"
#include <algorithm>
#include <cmath>
#include <limits>


// ---------------------------------------------------------------- Sphere

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


// ---------------------------------------------------------------- Plane

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


// ---------------------------------------------------------------- AABB

static float axis_value(const Vector3D& v, int axis) {
    return axis == 0 ? v.x : (axis == 1 ? v.y : v.z);
}

AABB::AABB() {
    float inf = std::numeric_limits<float>::infinity();
    min_corner = Vector3D(inf, inf, inf);
    max_corner = Vector3D(-inf, -inf, -inf);
}

void AABB::expand(const Vector3D& p) {
    min_corner = Vector3D(std::min(min_corner.x, p.x), std::min(min_corner.y, p.y), std::min(min_corner.z, p.z));
    max_corner = Vector3D(std::max(max_corner.x, p.x), std::max(max_corner.y, p.y), std::max(max_corner.z, p.z));
}

void AABB::expand(const AABB& other) {
    expand(other.min_corner);
    expand(other.max_corner);
}

int AABB::longest_axis() const {
    Vector3D size = max_corner - min_corner;
    if (size.x >= size.y && size.x >= size.z) return 0;
    return size.y >= size.z ? 1 : 2;
}

// Slab test: on each axis the ray is inside the box for t in [t_near, t_far].
// The ray hits the box if those three intervals (and [t_min, t_max]) overlap.
bool AABB::hit(const Ray& ray, float t_min, float t_max) const {
    for (int axis = 0; axis < 3; axis++) {
        float origin = axis_value(ray.origin, axis);
        float inverse_direction = 1.0f / axis_value(ray.direction, axis);   // +-inf if parallel: still works
        float t_near = (axis_value(min_corner, axis) - origin) * inverse_direction;
        float t_far = (axis_value(max_corner, axis) - origin) * inverse_direction;
        if (inverse_direction < 0) std::swap(t_near, t_far);
        t_min = t_near > t_min ? t_near : t_min;
        t_max = t_far < t_max ? t_far : t_max;
        if (t_max < t_min) return false;
    }
    return true;
}


// ---------------------------------------------------------------- Mesh

static const int MAX_TRIANGLES_PER_LEAF = 4;

Mesh::Mesh(std::vector<Triangle> triangle_list, int material_index)
    : Surface(material_index), triangles(std::move(triangle_list)) {
    if (triangles.empty()) return;
    nodes.reserve(2 * triangles.size());
    build(0, static_cast<int>(triangles.size()));
}

static Vector3D centroid(const Triangle& t) {
    return (t.vertex_a() + t.vertex_b() + t.vertex_c()).scale(1.0f / 3.0f);
}

// Builds the node for triangles[first, first + count) and returns its index.
// Splits along the longest axis of the triangle centers, at the median.
int Mesh::build(int first, int count) {
    int index = static_cast<int>(nodes.size());
    nodes.push_back(Node());

    AABB box;
    AABB centers;
    for (int i = first; i < first + count; i++) {
        box.expand(triangles[i].vertex_a());
        box.expand(triangles[i].vertex_b());
        box.expand(triangles[i].vertex_c());
        centers.expand(centroid(triangles[i]));
    }
    nodes[index].box = box;

    if (count <= MAX_TRIANGLES_PER_LEAF) {
        nodes[index].first = first;
        nodes[index].count = count;
        return index;
    }

    // Put the half with smaller centers on the left, the rest on the right.
    int axis = centers.longest_axis();
    int middle = first + count / 2;
    std::nth_element(triangles.begin() + first, triangles.begin() + middle, triangles.begin() + first + count,
        [axis](const Triangle& p, const Triangle& q) {
            return axis_value(centroid(p), axis) < axis_value(centroid(q), axis);
        });

    int left = build(first, middle - first);              // careful: push_back may move `nodes`,
    int right = build(middle, first + count - middle);    // so write through the index afterwards
    nodes[index].left = left;
    nodes[index].right = right;
    return index;
}

// Walk the tree with a stack. Every time we find a closer hit we shrink t_max,
// so boxes that are farther than the best hit so far get skipped.
bool Mesh::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    if (nodes.empty()) return false;

    bool found = false;
    int stack[64];
    int stack_size = 0;
    stack[stack_size++] = 0;

    while (stack_size > 0) {
        const Node& node = nodes[stack[--stack_size]];
        if (!node.box.hit(ray, t_min, t_max)) continue;

        if (node.count > 0) {
            for (int i = node.first; i < node.first + node.count; i++) {
                if (triangles[i].intersect(ray, t_min, t_max, hit)) {
                    found = true;
                    t_max = hit.t;
                }
            }
        } else {
            stack[stack_size++] = node.left;
            stack[stack_size++] = node.right;
        }
    }

    if (found) hit.material_index = material_index;
    return found;
}


// ---------------------------------------------------------------- Cylinder

Cylinder::Cylinder(const Vector3D& center, const Vector3D& axis, float radius, float height, int material_index)
    : Surface(material_index), center(center), axis(axis.normalize()),
      radius(radius), radius_squared(radius * radius), half_height(height / 2.0f) {}

// Candidates: up to two side hits and two cap hits. Keep the smallest t in range.
bool Cylinder::intersect(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    const Vector3D& d = ray.direction;
    Vector3D oc = ray.origin - center;

    // Split o - c and d into a part along the axis and a part perpendicular to it.
    float oc_along = dot_product(oc, axis);
    float d_along = dot_product(d, axis);
    Vector3D oc_perp = oc - axis.scale(oc_along);
    Vector3D d_perp = d - axis.scale(d_along);

    bool found = false;
    float best_t = t_max;
    Vector3D best_normal;

    // Side (infinite tube): |oc_perp + t * d_perp|^2 = r^2, a circle in the perpendicular plane.
    //   (d_perp.d_perp) t^2 + 2 (d_perp.oc_perp) t + (oc_perp.oc_perp - r^2) = 0
    float a = dot_product(d_perp, d_perp);
    if (a > 1e-12f) {   // a == 0 means the ray is parallel to the axis: it can only hit the caps
        float half_b = dot_product(d_perp, oc_perp);
        float c = dot_product(oc_perp, oc_perp) - radius_squared;
        float discriminant = half_b * half_b - a * c;
        if (discriminant >= 0) {
            float sqrt_d = std::sqrt(discriminant);
            float roots[2] = {(-half_b - sqrt_d) / a, (-half_b + sqrt_d) / a};
            for (float t : roots) {
                if (t <= t_min || t >= best_t) continue;
                float height_at_hit = oc_along + t * d_along;    // axial coordinate of the hit
                if (std::fabs(height_at_hit) > half_height) continue;   // outside the finite part
                found = true;
                best_t = t;
                // Radial normal: the perpendicular part of (hit - center), divided by r.
                best_normal = (oc_perp + d_perp.scale(t)).scale(1.0f / radius);
            }
        }
    }

    // Caps: planes at height +h/2 (normal +A) and -h/2 (normal -A).
    // Axial coordinate along the ray is oc_along + t * d_along; set it to +-h/2 and solve for t.
    if (std::fabs(d_along) > 1e-12f) {
        for (float side : {1.0f, -1.0f}) {
            float t = (side * half_height - oc_along) / d_along;
            if (t <= t_min || t >= best_t) continue;
            Vector3D radial = oc_perp + d_perp.scale(t);          // hit point minus cap center
            if (dot_product(radial, radial) > radius_squared) continue;   // outside the disk
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
