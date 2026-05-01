//
// Created by Quinn on 10/28/2025.
//

#ifndef FINALPROJECT_WIREFRAME_H
#define FINALPROJECT_WIREFRAME_H

#include <fstream>
#include <vector>
#include "includes/vec3.h"
#include "includes/quaternion.h"
#include "includes/triangle.h"


using rndr::vec3, rndr::Triangle, rndr::Quaternion;

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
    Quaternion rotation{};
    int rotateFlags = 0;

    void setMidpoint();

public:
    bool isExpired = false;

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

    void updateLocation(const vec3& change);

    [[nodiscard]] const vec3& getLocation() const { return midpoint; }

    [[nodiscard]] const Quaternion& getRotation() const { return rotation; }

    void toggleRotation(int axis);

    void rotate(float deltaTime, float TPS);

    [[nodiscard]] const std::vector<vec3>& getVertices() const;

    [[nodiscard]] const std::vector<Triangle>& getFaces() const;
};


#endif //FINALPROJECT_WIREFRAME_H