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

        [[nodiscard]] Quaternion conjugate() const { return {-x, -y, -z, w}; }

        void normalize() {
            float len = std::sqrt(x * x + y * y + z * z + w * w);
            x /= len;
            y /= len;
            z /= len;
            w /= len;
        }
    };

    inline Quaternion fromAxisAngle(float x, float y, float z, float radians) {
        float halfAngle = radians / 2.0f;
        float s = std::sin(halfAngle);
        return {x * s, y * s, z * s, std::cos(halfAngle)};
    }
}

#endif //FINALPROJECT_QUATERNION_H
