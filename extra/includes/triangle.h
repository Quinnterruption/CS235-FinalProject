//
// Created by Quinn on 4/13/2026.
//

#ifndef FINALPROJECT_TRIANGLE_H
#define FINALPROJECT_TRIANGLE_H

#include <array>


namespace rndr {
    /**
     * A collection of indices of the vertices for a Triangle
     * Each vertex contains 3 indices that coordinate with 3 vector3's in a std::vector
     */
    class Triangle {
        std::array<std::size_t, 3> verticesIndex{};

    public:
        Triangle() = default;

        Triangle(const std::initializer_list<std::size_t> init) {
            if (init.size() != 3) throw std::invalid_argument("Triangle must have exactly 3 elements");
            verticesIndex[0] = *init.begin();
            verticesIndex[1] = *init.begin() + 1;
            verticesIndex[2] = *init.begin() + 2;
        }

        /**
         * Allows the user to read the data from verticesIndex
         * @param idx the index to read
         * @return the vertex's index that the Triangle binds to
         */
        const std::size_t& operator[](const std::size_t idx) const {
            return verticesIndex[idx];
        }

        /**
         * Allows the user to read/write the data from verticesIndex
         * @param idx the index to read/write
         * @return the vertex's index that the Triangle binds to
         */
        std::size_t& operator[](const std::size_t idx) {
            return verticesIndex[idx];
        }
    };
}


#endif //FINALPROJECT_TRIANGLE_H
