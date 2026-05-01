//
// Created by TechSolutions on 5/1/2026.
//

#ifndef FINALPROJECT_AABB_H
#define FINALPROJECT_AABB_H

#include "includes/vec3.h"


class AABB {

public:
    rndr::vec3 min{0, 0, 0};
    rndr::vec3 max{0, 0, 0};

    bool intersects(const rndr::vec3& coords) const;

    void updateMinMax(const rndr::vec3& coords);
};


#endif //FINALPROJECT_AABB_H