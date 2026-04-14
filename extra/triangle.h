//
// Created by Quinn on 4/13/2026.
//

#ifndef FINALPROJECT_TRIANGLE_H
#define FINALPROJECT_TRIANGLE_H

#include <array>


/**
 * A collection of indices of the vertices for a Triangle
 * Each vertex contains 3 indices that coordinate with 3 vector3's in a std::vector
 */
class Triangle {
    std::array<std::size_t, 3> vertices;

public:
    Triangle() = default;

    explicit Triangle(const std::array<std::size_t, 3>& vertices) : vertices{vertices} {}

    const std::size_t& operator[](const std::size_t idx) const {
        return vertices[idx];
    }
};



#endif //FINALPROJECT_TRIANGLE_H
