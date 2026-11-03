#include "scene.h"
#include "shapes.h"

Scene::Scene(const parser::SceneConfig& config) {
    const std::vector<Vector3D>& v = config.vertices;   // parser already turned IDs into 0-based indices

    for (const parser::SphereConfig& sphere : config.spheres) {
        surfaces.emplace_back(new Sphere(v[sphere.center_index], sphere.radius, sphere.material_index));
    }

    for (const parser::PlaneConfig& plane : config.planes) {
        surfaces.emplace_back(new Plane(v[plane.center_index], plane.normal, plane.material_index));
    }

    for (const parser::CylinderConfig& cylinder : config.cylinders) {
        surfaces.emplace_back(new Cylinder(v[cylinder.center_index], cylinder.axis, cylinder.radius,
                                           cylinder.height, cylinder.material_index));
    }

    for (const parser::TriangleConfig& triangle : config.triangles) {
        const parser::Face& f = triangle.face;
        surfaces.emplace_back(new Triangle(v[f.v0_index], v[f.v1_index], v[f.v2_index], triangle.material_index));
    }

    for (const parser::MeshConfig& mesh : config.meshes) {
        std::vector<Triangle> triangles;
        triangles.reserve(mesh.faces.size());
        for (const parser::Face& f : mesh.faces) {
            triangles.emplace_back(v[f.v0_index], v[f.v1_index], v[f.v2_index], mesh.material_index);
        }
        surfaces.emplace_back(new Mesh(std::move(triangles), mesh.material_index));
    }
}

// Test every surface; each time one is closer, shrink t_max so later tests can exit early.
bool Scene::closest_hit(const Ray& ray, float t_min, float t_max, HitRecord& hit) const {
    bool found = false;
    for (const std::unique_ptr<Surface>& surface : surfaces) {
        if (surface->intersect(ray, t_min, t_max, hit)) {
            found = true;
            t_max = hit.t;
        }
    }
    return found;
}

bool Scene::any_hit(const Ray& ray, float t_min, float t_max) const {
    HitRecord ignored;
    for (const std::unique_ptr<Surface>& surface : surfaces) {
        if (surface->intersect(ray, t_min, t_max, ignored)) {
            return true;
        }
    }
    return false;
}
