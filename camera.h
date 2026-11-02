#pragma once
#include "vector3d.h"
#include "ray.h"
#include "scene_parser.h"
#include <string>


class Camera{

    Vector3D position;
    float near_distance;
    Vector3D gaze;
    Vector3D up;
    float near_plane[4];
    float pixel_width;
    float pixel_height;
    Vector3D w;
    Vector3D v;
    Vector3D u;
    Vector3D q;
public:
    int image_resolution[2];
    std::string image_name;


    Camera(

    const parser::CameraConfig& config

    ){
        
        this->position = config.position;
        this->gaze = config.gaze;
        this->up = config.up;
        this->near_distance = config.near_distance;
        this->near_plane[0] = config.near_plane.left;
        this->near_plane[1] = config.near_plane.right;
        this->near_plane[2] = config.near_plane.bottom;
        this->near_plane[3] = config.near_plane.top;
        this->image_resolution[0] = config.image_width;
        this->image_resolution[1] = config.image_height;
        this->image_name = config.image_name;


        this->pixel_width = (near_plane[1] - near_plane[0])/image_resolution[0];
        this->pixel_height  = (near_plane[3] - near_plane[2])/image_resolution[1];

        this->w  = -gaze.normalize();
        this->u  = cross_product(up,this->w).normalize();
        this->v = cross_product(this->w,this->u);

        auto m  = position -w.scale(near_distance);
       this->q = m + u.scale(near_plane[0]) + v.scale(near_plane[3]);
    }
  
    
    Ray shoot_ray(int row , int column ) const{


        float  s_u = (column + 0.5)*pixel_width;
        float  s_v = (row + 0.5)*pixel_height;

        auto destination =  this->u.scale(s_u) - this->v.scale(s_v) + q;

        return Ray(position,destination - position);

    }
};

