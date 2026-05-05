//
// Created by Quinn on 10/28/2025.
//

#ifndef FINALPROJECT_WIREFRAME_H
#define FINALPROJECT_WIREFRAME_H

#include <fstream>
#include <memory>
#include <vector>
#include "includes/vec3.h"
#include "includes/quaternion.h"
#include "includes/triangle.h"
#include "AABB.h"
#include "includes/raycast.h"


using rndr::vec3, rndr::Triangle, rndr::Quaternion;

/**
 * Rotation Flags used to check which axes to rotate over
 */
enum rotationFlags {
    rotateX = 1,
    rotateY = 2,
    rotateZ = 4
};

class WireFrame : public AABB {
    std::vector<vec3> vertices;
    std::vector<Triangle> faces;
    vec3 midpoint{};
    Quaternion rotation{};
    int rotateFlags = 0;
    bool expired = false;

    // Linked List Handling
    // std::weak_ptr<WireFrame> parent;
    std::vector<std::unique_ptr<WireFrame>> children;

    void setMidpoint();

public:

    WireFrame();

    WireFrame(const std::string& fileName);

    WireFrame(const WireFrame& obj);

    WireFrame& operator=(const WireFrame& obj);

    WireFrame& operator=(WireFrame&& obj) noexcept;

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

    void rotate(const Quaternion& q);

    void setRotation(const Quaternion& q);

    [[nodiscard]] const std::vector<vec3>& getVertices() const;

    [[nodiscard]] const std::vector<Triangle>& getFaces() const;

    const std::vector<std::unique_ptr<WireFrame>>& getChildren() const;

    // const std::weak_ptr<WireFrame>& getParent() const;

    bool intersects(const std::pair<float, float>& screenCoords, const rndr::Raycast& ray) const;

    void addChild(const WireFrame& child);

    bool isExpired() const { return expired; }

    void expire() { expired = true; }
};


#endif //FINALPROJECT_WIREFRAME_H