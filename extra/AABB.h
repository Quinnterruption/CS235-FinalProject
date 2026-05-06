//
// Created by TechSolutions on 5/1/2026.
//

#ifndef FINALPROJECT_AABB_H
#define FINALPROJECT_AABB_H

#include "includes/vec3.h"


class AABB {

public:
    virtual ~AABB() = default;

    AABB() = default;

    rndr::vec3 min = rndr::vec3(std::numeric_limits<float>::max());
    rndr::vec3 max = rndr::vec3(std::numeric_limits<float>::min());

    AABB(const AABB& obj);

    AABB& operator=(const AABB& obj);

    [[nodiscard]] virtual rndr::vec3 getMin() const { return min; }

    [[nodiscard]] virtual rndr::vec3 getMax() const { return max; }

    [[nodiscard]] bool intersects(const rndr::vec3& coords) const;

    void calculateMinMax(const rndr::vec3& coords);

    void updateMinMax(const rndr::vec3& change);
};


#endif //FINALPROJECT_AABB_H