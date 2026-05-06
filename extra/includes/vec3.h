//
// Created by TechSolutions on 4/21/2026.
//

#ifndef FINALPROJECT_VEC3_H
#define FINALPROJECT_VEC3_H
#include <initializer_list>
#include <stdexcept>
#include "quaternion.h"


namespace rndr {
    struct vec3 {
        float x, y, z;

        vec3() = default;

        vec3(const std::initializer_list<float> list) {
            if (list.size() != 3) throw std::invalid_argument("Vec3 must have exactly 3 elements");
            x = *list.begin();
            y = *(list.begin() + 1);
            z = *(list.begin() + 2);
        }

        vec3& operator+=(const vec3& obj) {
            x += obj.x;
            y += obj.y;
            z += obj.z;
            return *this;
        }
        vec3& operator-=(const vec3& obj) {
            x -= obj.x;
            y -= obj.y;
            z -= obj.z;
            return *this;
        }
        vec3& operator*=(const float& num) {
            x *= num;
            y *= num;
            z *= num;
            return *this;
        }
        vec3 operator+(const vec3& obj) const {
            vec3 temp = *this;
            temp += obj;
            return temp;
        }
        vec3 operator-(const vec3& obj) const {
            vec3 temp = *this;
            temp -= obj;
            return temp;
        }
        vec3 operator*(const float& num) const {
            vec3 temp = *this;
            temp *= num;
            return temp;
        }
        bool operator==(const vec3& obj) const {
            return x == obj.x && y == obj.y && z == obj.z;
        }
        float& operator[](const std::size_t idx) {
            switch (idx) {
                case 0:
                    return x;
                case 1:
                    return y;
                case 2:
                    return z;
                default:
                    throw std::out_of_range("Invalid index");
            }
        }

        static vec3 rotate(const vec3& v, Quaternion q) {
            float magSq = q.x * q.x + q.y * q.y + q.z * q.z;
            if (std::abs(magSq - 1.0f) > 0.000001f) {
                q.normalize();
            }

            vec3 r{q.x, q.y, q.z};
            vec3 t = cross(r, v) * 2.0f;
            return v + cross(r, t) + t * q.w;
        }

        static vec3 cross(const vec3& v, const vec3& w) {
            return {v.y * w.z - v.z * w.y,
                v.z * w.x - v.x * w.z,
                v.x * w.y - v.y * w.x};
        }
    };
}

#endif //FINALPROJECT_VEC3_H