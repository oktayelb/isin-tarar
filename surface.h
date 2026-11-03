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
    bool virtual  intersect(const Ray& ray ,  HitRecord& record) const  = 0;
    virtual ~Surface(){}


};
// ezgi sen şekilleri bu base classten implement edeceksin

