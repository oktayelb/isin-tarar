#pragma once

#include "ray.h"
#include "vector3d.h" // ray also imports this, there might be a problem

struct HitRecord{


    Vector3D point;
    Vector3D normal;
    int material_index;
    float t;
};

class Surface{

protected:
    int material_index;

public:

    Surface(int index){this->material_index= index;}
    virtual bool intersect(const Ray& ray, float t_min, float t_max, HitRecord& record) const = 0;
   virtual bool any_hit(const Ray& ray, float t_min, float t_max) const {
        HitRecord ignored;
        return intersect(ray, t_min, t_max, ignored);
    }
    virtual ~Surface(){}


};
// ezgi sen şekilleri bu base classten implement edeceksin

