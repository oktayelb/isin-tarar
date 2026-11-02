#pragma once
#include "vector3d.h"

class Ray{

public:
    Vector3D origin;
    Vector3D direction;

    Ray(){}
    Ray(const Vector3D& org , const Vector3D& dir){
        this->origin = org;
        this->direction = dir;
    }

};
