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
            Quaternion vecAsQuat(v.x, v.y, v.z, 0.0f);
            q.normalize();
            Quaternion qConj = q.conjugate();
            Quaternion rotated = q * vecAsQuat * qConj;
            return {rotated.x, rotated.y, rotated.z};
        }
    };
}

#endif //FINALPROJECT_VEC3_H