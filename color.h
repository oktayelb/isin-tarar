#pragma once

class Color {

    public:
        float r;
        float g;
        float b;

    Color operator+(const Color& other) const {
        return Color(r + other.r, g + other.g, b + other.b);
    }
    Color operator*(const float scalar) const {
        return Color(r*scalar, g*scalar, b*scalar);
    }
    Color operator*(const Color& other) const {
        return Color(r * other.r, g * other.g, b * other.b);

    }

    Color(){
        this->r = 0;
        this->g = 0;
        this->b = 0;
    }
    Color(float r, float g , float b){
        this->r = r;
        this->g = g;
        this->b = b;
    }
};
