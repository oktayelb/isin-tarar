#pragma once

#include "vector3d.h"
#include "color.h"
#include <string>
#include <vector>


namespace parser {

    struct NearPlane {
        float left;
        float right;
        float bottom;
        float top;
    };

    struct CameraConfig {
        Vector3D position;
        Vector3D gaze;
        Vector3D up;
        NearPlane near_plane;
        float near_distance;
        int image_width;
        int image_height;
        std::string image_name;
    };

    struct PointLightConfig {
        Vector3D position;
        Color intensity;
    };

    struct MaterialConfig {
        bool is_mirror;
        Color ambient_reflectance;
        Color diffuse_reflectance;
        Color specular_reflectance;
        Color mirror_reflectance;
        float phong_exponent;
    };

    struct Face {
        int v0_index;
        int v1_index;
        int v2_index;
    };

    struct MeshConfig {
        int material_index;
        std::vector<Face> faces;
    };

    struct TriangleConfig {
        int material_index;
        Face face;
    };

    struct SphereConfig {
        int material_index;
        int center_index;
        float radius;
    };

    struct PlaneConfig {
        int material_index;
        int center_index;
        Vector3D normal;
    };

    struct CylinderConfig {
        int material_index;
        int center_index;
        Vector3D axis;
        float radius;
        float height;
    };

    struct SceneConfig {
        Color background_color;
        float shadow_ray_epsilon;
        int max_recursion_depth;
        std::vector<CameraConfig> cameras;
        Color ambient_light;
        std::vector<PointLightConfig> point_lights;
        std::vector<MaterialConfig> materials;
        std::vector<Vector3D> vertices;
        std::vector<MeshConfig> meshes;
        std::vector<TriangleConfig> triangles;
        std::vector<SphereConfig> spheres;
        std::vector<PlaneConfig> planes;
        std::vector<CylinderConfig> cylinders;
    };

    SceneConfig parse(const std::string& input_file);
}
