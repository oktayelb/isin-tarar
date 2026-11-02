#pragma once
#include <cmath>
class Vector3D {

    public:
        float x;
        float y;
        float z;


    Vector3D operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }

    Vector3D operator-(const Vector3D& other) const {
        return Vector3D(x - other.x, y - other.y, z - other.z);
    }

    Vector3D operator-()const {
        return Vector3D(-x, -y, -z);
    }

    
    Vector3D(float x, float y , float z){
        this->x = x;
        this->y = y;
        this->z = z;
    }
    Vector3D(){
        this->x = 0;
        this->y = 0;
        this->z = 0;

    }

    float length()const{
        return std::sqrt(x*x +y*y+ z*z);
    }

    Vector3D normalize()const{
        float len = this->length();
        if (len == 0) 
            return Vector3D(0,0,0);
        return Vector3D(x/len,y/len,z/len);
    }

    Vector3D scale (float scalar) const {
        return Vector3D(x*scalar, y*scalar, z*scalar);
    }


};


inline float dot_product(Vector3D v1, Vector3D v2){

    return (v1.x*v2.x) +(v1.y*v2.y)+ (v1.z*v2.z);
}

inline Vector3D cross_product(Vector3D v1, Vector3D v2){

    auto output = Vector3D();
    output.x = (v1.y*v2.z -v1.z*v2.y );
    output.y = (v1.z*v2.x -v1.x*v2.z );
    output.z = (v1.x*v2.y -v1.y*v2.x );

    return output;
}
