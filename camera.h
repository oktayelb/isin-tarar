#pragma once
#include "vector3d.h"
#include "ray.h"
#include <string>


class Camera{

    Vector3D position;
    float near_distance;
    Vector3D gaze;
    Vector3D up;
    float near_plane[4];
public:
    int image_resolution[2];
    std::string image_name;

    Camera(){}
    Camera(){} // init constructor
  
    
    Ray shoot_ray(int row , int column ) const{


        float  s_u = (column + 0.5)*pixel_width;
        float  s_v = (row + 0.5)*pixel_height;

        // compute destination somehow using camera.position and row/column. 
        // we need row and column mapped on the 3d coordinate somehow.

        return Ray(position,destination - position);

    }
};

