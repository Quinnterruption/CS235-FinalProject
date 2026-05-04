//
// Created by TechSolutions on 5/1/2026.
//

#include "AABB.h"
#include <iostream>


AABB::AABB(const AABB& obj) {
    min = obj.getMin();
    max = obj.getMax();
}

AABB& AABB::operator=(const AABB& obj) {
    if (this != &obj) {
        min = obj.getMin();
        max = obj.getMax();
    }
    return *this;
}

bool AABB::intersects(const rndr::vec3& coords) const {
    return  (min.x <= coords.x && max.x >= coords.x) &&
            (min.y <= coords.y && max.y >= coords.y) &&
            (min.z <= coords.z && max.z >= coords.z);
}

void AABB::updateMinMax(const rndr::vec3& coords) {
    if (coords.x < min.x) min.x = coords.x;
    if (coords.x > max.x) max.x = coords.x;
    if (coords.y < min.y) min.y = coords.y;
    if (coords.y > max.y) max.y = coords.y;
    if (coords.z < min.z) min.z = coords.z;
    if (coords.z > max.z) max.z = coords.z;
}