//
// Created by Quinn on 10/28/2025.
//

#ifndef FINALPROJECT_WIREFRAME_H
#define FINALPROJECT_WIREFRAME_H
#include <iostream>
#include <array>
#include <fstream>
#include <vector>
#include "triangle.h"

#define matrix3 std::array<std::array<double, 3>, 3>

using std::array;

struct vec3 {
    double x, y, z;

    vec3() = default;

    vec3(const std::initializer_list<double> list) {
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
    double& operator[](const std::size_t idx) {
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
};

/**
 * Rotation Flags used to check which axes to rotate over
 */
enum rotationFlags {
    rotateX = 1,
    rotateY = 2,
    rotateZ = 4
};

class WireFrame {
    std::vector<vec3> vertices;
    std::vector<Triangle> faces;
    vec3 midpoint{};

    void setMidpoint();

public:

    WireFrame();

    WireFrame(const std::string& fileName);

    /**
     * Reads an obj file and creates a WireFrame from the data
     * @param fileName the obj file to read from
     */
    void setWireFrame(const std::string& fileName);

    /**
     * Removes all previously initialized vertices, faces, and midpoint
     */
    void clearWireFrame();

    // Functions we have to have
    // setMidpoint -> evaluate the midpoint of ALL points and store as a single vec3
    //      is called once when we construct the wireFrame
    //      when rendering, all points should be adjusted by the midpoint
    // updateLocation -> applies the translation to the midpoint
    //      when rendering, all points should be adjusted by the midpoint
    //
    // Functions we will add later
    // rotate, toggleRotation, setRotation, getRotation
    //      all these functions rely on matrixMult as well

    void updateLocation(const vec3& change);

    const vec3& getLocation() const { return midpoint; }

    [[nodiscard]] const std::vector<vec3>& getVertices() const;

    [[nodiscard]] const std::vector<Triangle>& getFaces() const;
};

// /**
//  * A class to store and edit the WireFrame data
//  */
// class WireFrame {
//     int rotateFlags = 0;
//     int width = 0;
//     int height = 0;
//     int depth = 0;
//
//     /**
//      * @return the origin of the WireFrame
//      */
//     coord getOrigin();
//
//     /**
//      * @param first the first 3D matrix to multiply
//      * @param second the second 3D matrix to multiply
//      * @return the resulting 3D matrix
//      */
//     static matrix3 matrixMult(const matrix3& first, const matrix3& second);
// public:
//     coord midPoint;
//     array<coord, 8> coordinates;
//
//     /**
//      * Default constructor initializes all variables to 0
//      */
//     WireFrame() = default;
//
//     /**
//      * Creates a WireFrame with coordinates according to init
//      * Will be deprecated later
//      * @param init the coordinates of every corner
//      */
//     WireFrame(std::initializer_list<coord> init);
//
//     /**
//      * Creates a WireFrame according to User specifications
//      * @param topLeft the top left coordinate of the WireFrame
//      * @param width the width of the WireFrame
//      * @param height the height of the WireFrame
//      * @param depth the depth of the WireFrame
//      */
//     WireFrame(const coord &topLeft, double width, double height, double depth);
//
//     /* DEPRECATED
//     friend std::ostream& operator << (std::ostream& os, const WireFrame& obj) {
//         std::cout << obj.topLeft << "\n" << obj.botRight << "\n";
//         return os;
//     }*/
//
//     /**
//      * Checks if 2 WireFrames are equal
//      * @param obj the WireFrame to compare
//      * @return true if the WireFrames are equal
//      */
//     bool operator==(const WireFrame& obj) const;
//
//     /**
//      * @return the rotation flags of the current WireFrame
//      */
//     int getRotation();
//
//     /**
//      * Sets the rotation of the current WireFrame
//      * @param axis new rotation flags
//      */
//     void setRotation(int axis);
//
//     /**
//      * Toggles chosen rotations
//      * @param axis chosen axis based on RotationFlags enum
//      */
//     void toggleRotation(int axis);
//
//     /**
//      * Rotates the current WireFrame
//      */
//     void rotate();
//
//     /**
//      * Moves the current WireFrame
//      * @param amount the distance to move in each direction
//      */
//     void updateLocation(const coord &amount);
// };


#endif //FINALPROJECT_WIREFRAME_H