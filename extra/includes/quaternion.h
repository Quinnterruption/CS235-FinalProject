//
// Created by Quinn on 4/20/2026.
//

#ifndef FINALPROJECT_QUATERNION_H
#define FINALPROJECT_QUATERNION_H

#include <cmath>

namespace rndr {
    struct Quaternion {
        float x, y, z, w;

        Quaternion(float x = 0, float y = 0, float z = 0, float w = 1) : x(x), y(y), z(z), w(w) {}

        Quaternion operator*(const Quaternion& q) const {
            return {
                w * q.x + x * q.w + y * q.z - z * q.y,
                w * q.y - x * q.z + y * q.w + z * q.x,
                w * q.z + x * q.y - y * q.x + z * q.w,
                w * q.w - x * q.x - y * q.y - z * q.z
            };
        }

        Quaternion& operator*=(const Quaternion& q) {
            return *this = *this * q;
        }

        bool operator==(const Quaternion& q) const {
            return q.x == x && q.y == y && q.z == z && q.w == w;
        }

        [[nodiscard]] Quaternion conjugate() const { return {-x, -y, -z, w}; }

        void normalize() {
            float len = x * x + y * y + z * z + w * w;

            if (std::abs(len - 1.0f) <= 0.000001f) return;

            len = std::sqrt(len);
            if (len > 0.0f) {
                float invLen = 1.0f / len;
                x *= invLen;
                y *= invLen;
                z *= invLen;
                w *= invLen;
            }
        }

        static Quaternion fromAxisAngle(float x, float y, float z, float radians) {
            float halfAngle = radians / 2.0f;
            float s = std::sin(halfAngle);
            return {x * s, y * s, z * s, std::cos(halfAngle)};
        }

        static Quaternion identity() {
            return {0.0f, 0.0f, 0.0f, 1.0f};
        }
    };
}

#endif //FINALPROJECT_QUATERNION_H
