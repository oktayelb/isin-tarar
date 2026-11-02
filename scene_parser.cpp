#include "scene_parser.h"
#include "json.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


namespace parser {

    namespace {

        std::string read_file(const std::string& path) {
            std::ifstream file(path, std::ios::binary);
            if (!file) {
                throw std::runtime_error("cannot open scene file: " + path);
            }
            std::stringstream contents;
            contents << file.rdbuf();
            return contents.str();
        }

        const std::string& require_text(const json::Value& object, const std::string& key) {
            const json::Value& member = object.require(key);
            if (member.kind != json::Kind::Text) {
                throw std::runtime_error("expected a string value for key: " + key);
            }
            return member.text;
        }

        std::vector<const json::Value*> as_element_list(const json::Value* value) {
            std::vector<const json::Value*> elements;
            if (value == nullptr) {
                return elements;
            }
            if (value->kind == json::Kind::Array) {
                for (const json::Value& item : value->array_items) {
                    elements.push_back(&item);
                }
            } else {
                elements.push_back(value);
            }
            return elements;
        }

        std::vector<const json::Value*> find_elements(const json::Value& scene, const std::string& group_key, const std::string& element_key) {
            const json::Value* group = scene.find(group_key);
            if (group == nullptr) {
                return std::vector<const json::Value*>();
            }
            return as_element_list(group->find(element_key));
        }

        template <typename Number>
        std::vector<Number> parse_numbers(const std::string& text, const std::string& key) {
            std::vector<Number> numbers;
            std::istringstream stream(text);
            Number number;
            while (stream >> number) {
                numbers.push_back(number);
            }
            if (!stream.eof()) {
                throw std::runtime_error("invalid number in key: " + key);
            }
            return numbers;
        }

        template <typename Number>
        std::vector<Number> read_numbers(const json::Value& object, const std::string& key, size_t expected_count) {
            std::vector<Number> numbers = parse_numbers<Number>(require_text(object, key), key);
            if (numbers.size() != expected_count) {
                throw std::runtime_error("expected " + std::to_string(expected_count) + " number(s) in key: " + key);
            }
            return numbers;
        }

        float read_float(const json::Value& object, const std::string& key) {
            return read_numbers<float>(object, key, 1)[0];
        }

        int read_int(const json::Value& object, const std::string& key) {
            return read_numbers<int>(object, key, 1)[0];
        }

        Vec3f read_vec3f(const json::Value& object, const std::string& key) {
            std::vector<float> numbers = read_numbers<float>(object, key, 3);
            return Vec3f{numbers[0], numbers[1], numbers[2]};
        }

        Vec3i read_vec3i(const json::Value& object, const std::string& key) {
            std::vector<int> numbers = read_numbers<int>(object, key, 3);
            return Vec3i{numbers[0], numbers[1], numbers[2]};
        }

        int id_to_index(int id, size_t element_count, const std::string& element_kind) {
            if (id < 1 || static_cast<size_t>(id) > element_count) {
                throw std::runtime_error(element_kind + " ID out of range: " + std::to_string(id));
            }
            return id - 1;
        }

        Face make_face(int v0_id, int v1_id, int v2_id, size_t vertex_count) {
            Face face;
            face.v0_index = id_to_index(v0_id, vertex_count, "vertex");
            face.v1_index = id_to_index(v1_id, vertex_count, "vertex");
            face.v2_index = id_to_index(v2_id, vertex_count, "vertex");
            return face;
        }

        CameraConfig parse_camera(const json::Value& element) {
            CameraConfig camera;
            camera.position = read_vec3f(element, "Position");
            camera.gaze = read_vec3f(element, "Gaze");
            camera.up = read_vec3f(element, "Up");

            std::vector<float> near_plane = read_numbers<float>(element, "NearPlane", 4);
            camera.near_plane.left = near_plane[0];
            camera.near_plane.right = near_plane[1];
            camera.near_plane.bottom = near_plane[2];
            camera.near_plane.top = near_plane[3];

            camera.near_distance = read_float(element, "NearDistance");

            std::vector<int> resolution = read_numbers<int>(element, "ImageResolution", 2);
            camera.image_width = resolution[0];
            camera.image_height = resolution[1];

            camera.image_name = require_text(element, "ImageName");
            return camera;
        }

        PointLightConfig parse_point_light(const json::Value& element) {
            PointLightConfig light;
            light.position = read_vec3f(element, "Position");
            light.intensity = read_vec3f(element, "Intensity");
            return light;
        }

        MaterialConfig parse_material(const json::Value& element) {
            MaterialConfig material;
            const json::Value* type = element.find("_type");
            material.is_mirror = type != nullptr && type->text == "mirror";

            material.ambient_reflectance = read_vec3f(element, "AmbientReflectance");
            material.diffuse_reflectance = read_vec3f(element, "DiffuseReflectance");
            material.specular_reflectance = read_vec3f(element, "SpecularReflectance");

            if (element.find("MirrorReflectance") != nullptr) {
                material.mirror_reflectance = read_vec3f(element, "MirrorReflectance");
            } else {
                material.mirror_reflectance = Vec3f{0.0f, 0.0f, 0.0f};
            }

            material.phong_exponent = read_float(element, "PhongExponent");
            return material;
        }

        std::vector<Vec3f> parse_vertices(const json::Value& scene) {
            std::vector<Vec3f> vertices;
            const json::Value* vertex_data = scene.find("VertexData");
            if (vertex_data == nullptr) {
                return vertices;
            }

            std::vector<float> coordinates = parse_numbers<float>(require_text(*vertex_data, "_data"), "VertexData");
            if (coordinates.size() % 3 != 0) {
                throw std::runtime_error("VertexData must contain three coordinates per vertex");
            }
            for (size_t i = 0; i < coordinates.size(); i += 3) {
                vertices.push_back(Vec3f{coordinates[i], coordinates[i + 1], coordinates[i + 2]});
            }
            return vertices;
        }

        MeshConfig parse_mesh(const json::Value& element, size_t vertex_count, size_t material_count) {
            MeshConfig mesh;
            mesh.material_index = id_to_index(read_int(element, "Material"), material_count, "material");

            const json::Value& faces = element.require("Faces");
            std::vector<int> vertex_ids = parse_numbers<int>(require_text(faces, "_data"), "Faces");
            if (vertex_ids.size() % 3 != 0) {
                throw std::runtime_error("Faces must contain three vertex IDs per triangle");
            }
            for (size_t i = 0; i < vertex_ids.size(); i += 3) {
                mesh.faces.push_back(make_face(vertex_ids[i], vertex_ids[i + 1], vertex_ids[i + 2], vertex_count));
            }
            return mesh;
        }

        TriangleConfig parse_triangle(const json::Value& element, size_t vertex_count, size_t material_count) {
            TriangleConfig triangle;
            triangle.material_index = id_to_index(read_int(element, "Material"), material_count, "material");
            std::vector<int> vertex_ids = read_numbers<int>(element, "Indices", 3);
            triangle.face = make_face(vertex_ids[0], vertex_ids[1], vertex_ids[2], vertex_count);
            return triangle;
        }

        SphereConfig parse_sphere(const json::Value& element, size_t vertex_count, size_t material_count) {
            SphereConfig sphere;
            sphere.material_index = id_to_index(read_int(element, "Material"), material_count, "material");
            sphere.center_index = id_to_index(read_int(element, "Center"), vertex_count, "vertex");
            sphere.radius = read_float(element, "Radius");
            return sphere;
        }

        PlaneConfig parse_plane(const json::Value& element, size_t vertex_count, size_t material_count) {
            PlaneConfig plane;
            plane.material_index = id_to_index(read_int(element, "Material"), material_count, "material");
            plane.center_index = id_to_index(read_int(element, "Center"), vertex_count, "vertex");
            plane.normal = read_vec3f(element, "Normal");
            return plane;
        }

        CylinderConfig parse_cylinder(const json::Value& element, size_t vertex_count, size_t material_count) {
            CylinderConfig cylinder;
            cylinder.material_index = id_to_index(read_int(element, "Material"), material_count, "material");
            cylinder.center_index = id_to_index(read_int(element, "Center"), vertex_count, "vertex");
            cylinder.axis = read_vec3f(element, "Axis");
            cylinder.radius = read_float(element, "Radius");
            cylinder.height = read_float(element, "Height");
            return cylinder;
        }
    }

    SceneConfig parse(const std::string& input_file) {
        json::Value document = json::parse_document(read_file(input_file));
        const json::Value& scene = document.require("Scene");

        SceneConfig config;
        config.background_color = read_vec3i(scene, "BackgroundColor");
        config.shadow_ray_epsilon = read_float(scene, "ShadowRayEpsilon");
        config.max_recursion_depth = read_int(scene, "MaxRecursionDepth");

        for (const json::Value* element : find_elements(scene, "Cameras", "Camera")) {
            config.cameras.push_back(parse_camera(*element));
        }
        if (config.cameras.empty()) {
            throw std::runtime_error("scene file has no camera");
        }

        config.ambient_light = Vec3f{0.0f, 0.0f, 0.0f};
        const json::Value* lights = scene.find("Lights");
        if (lights != nullptr && lights->find("AmbientLight") != nullptr) {
            config.ambient_light = read_vec3f(*lights, "AmbientLight");
        }
        for (const json::Value* element : find_elements(scene, "Lights", "PointLight")) {
            config.point_lights.push_back(parse_point_light(*element));
        }

        for (const json::Value* element : find_elements(scene, "Materials", "Material")) {
            config.materials.push_back(parse_material(*element));
        }

        config.vertices = parse_vertices(scene);

        size_t vertex_count = config.vertices.size();
        size_t material_count = config.materials.size();

        for (const json::Value* element : find_elements(scene, "Objects", "Mesh")) {
            config.meshes.push_back(parse_mesh(*element, vertex_count, material_count));
        }
        for (const json::Value* element : find_elements(scene, "Objects", "Triangle")) {
            config.triangles.push_back(parse_triangle(*element, vertex_count, material_count));
        }
        for (const json::Value* element : find_elements(scene, "Objects", "Sphere")) {
            config.spheres.push_back(parse_sphere(*element, vertex_count, material_count));
        }
        for (const json::Value* element : find_elements(scene, "Objects", "Plane")) {
            config.planes.push_back(parse_plane(*element, vertex_count, material_count));
        }
        for (const json::Value* element : find_elements(scene, "Objects", "Cylinder")) {
            config.cylinders.push_back(parse_cylinder(*element, vertex_count, material_count));
        }

        return config;
    }
}
