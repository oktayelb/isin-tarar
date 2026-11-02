#pragma once

#include <string>
#include <vector>


namespace parser {

    struct Vec3f {
        float x;
        float y;
        float z;
    };

    struct Vec3i {
        int x;
        int y;
        int z;
    };

    struct NearPlane {
        float left;
        float right;
        float bottom;
        float top;
    };

    struct CameraConfig {
        Vec3f position;
        Vec3f gaze;
        Vec3f up;
        NearPlane near_plane;
        float near_distance;
        int image_width;
        int image_height;
        std::string image_name;
    };

    struct PointLightConfig {
        Vec3f position;
        Vec3f intensity;
    };

    struct MaterialConfig {
        bool is_mirror;
        Vec3f ambient_reflectance;
        Vec3f diffuse_reflectance;
        Vec3f specular_reflectance;
        Vec3f mirror_reflectance;
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
        Vec3f normal;
    };

    struct CylinderConfig {
        int material_index;
        int center_index;
        Vec3f axis;
        float radius;
        float height;
    };

    struct SceneConfig {
        Vec3i background_color;
        float shadow_ray_epsilon;
        int max_recursion_depth;
        std::vector<CameraConfig> cameras;
        Vec3f ambient_light;
        std::vector<PointLightConfig> point_lights;
        std::vector<MaterialConfig> materials;
        std::vector<Vec3f> vertices;
        std::vector<MeshConfig> meshes;
        std::vector<TriangleConfig> triangles;
        std::vector<SphereConfig> spheres;
        std::vector<PlaneConfig> planes;
        std::vector<CylinderConfig> cylinders;
    };

    SceneConfig parse(const std::string& input_file);
}
