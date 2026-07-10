#ifndef VEC2_H
#define VEC2_H
#include <cmath>

struct Vec2 {
    float x;
    float y;
    Vec2(): x(0),y(0) {}
    Vec2(float a, float b) : x(a), y(b) {}

    Vec2 operator+(const Vec2& other) const{
        return {x + other.x, y + other.y};
    }

    Vec2 operator*(const float dt) const{
        return {x*dt, y*dt};
    }

    bool operator==(const Vec2 &other) const{
        return x==other.x && y==other.y;
    }

    float length() const{
        float len;
        len = std::sqrt(std::pow(x,2) + std::pow(y,2));
        return len;
    }

    Vec2 normalised() const{
        float div = length();
        return {x/div, y/div};
    }

};
#endif