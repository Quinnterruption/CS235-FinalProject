//
// Created by Quinn on 10/28/2025.
//

#include "wireFrame.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>

#include "playback.h"

// constexpr float DEGREES = 1.0f;
constexpr float RADIANS = 1.0f * M_PI / 180.0f;


WireFrame::WireFrame() = default;


WireFrame::WireFrame(const std::string& fileName) {
    setWireFrame(fileName);
}

void WireFrame::toggleRotation(const int axis) {
    rotateFlags ^= axis;
}

void WireFrame::rotate(float deltaTime, const float TPS) {
    if (rotateFlags == 0) return;

    float theta = RADIANS * deltaTime * TPS;

    const Quaternion quatX = Quaternion::fromAxisAngle(1.0f, 0.0f, 0.0f, theta);
    const Quaternion quatY = Quaternion::fromAxisAngle(0.0f, 1.0f, 0.0f, theta);
    const Quaternion quatZ = Quaternion::fromAxisAngle(0.0f, 0.0f, 1.0f, theta);
    Quaternion change;

    if ((rotateFlags & rotateX) == rotateX) {   // Rotate around X
        change *= quatX;
    }
    if ((rotateFlags & rotateY) == rotateY) {   // Rotate around Y
        change *= quatY;
    }
    if ((rotateFlags & rotateZ) == rotateZ) {   // Rotate around Z
        change *= quatZ;
    }
    rotate(change, midpoint);
}

void WireFrame::rotate(const Quaternion& q, const vec3& center) {
    rotation *= q;
    rotation.normalize();

    vec3 rotationVec = vec3::rotate(initialMidpoint - center, rotation);
    midpoint = center + rotationVec;
    for (auto& child : children) {
        child->rotate(q, midpoint);
    }
}

void WireFrame::setRotation(const Quaternion& q) {
    rotation = q;
    for (auto& child : children) {
        child->setRotation(q);
    }
}

void WireFrame::setWireFrame(const std::string& fileName) {
    using namespace std;
    clearWireFrame();

    filesystem::path filePath = filesystem::absolute(fileName);

    ifstream file(filePath);
    if (!file.is_open()) throw invalid_argument("Could not open file");

    while (file.peek() != EOF) {
        string line;
        getline(file >> ws, line);

        /* Remove repeated spaces */
        auto newEnd = ranges::unique(line,
            [](const char lhs, const char rhs) { return (lhs == rhs) && (lhs == ' '); }).begin();
        line.erase(newEnd, line.end());

        bool isVertex = line[0] == 'v' && line[1] == ' ';
        bool isNormal = line[0] == 'v' && line[1] == 'n';   // Unimplemented
        bool isFace = line[0] == 'f' && line[1] == ' ';

        auto space = line.find(' ');
        line = line.substr(space + 1);

        if (isVertex) {
            vec3 vertex{};
            for (int i = 0; i < 3; i++) {
                space = line.find(' ');
                vertex[i] = stof(line.substr(0, space));
                line = line.substr(space + 1);
            }
            vertices.push_back(vertex);
        } else if (isFace) {
            Triangle face{};
            for (int i = 0; i < 3; i++) {
                space = line.find(' ');
                face[i] = stoi(line.substr(0, space)) - 1;
                line = line.substr(space + 1);
            }
            faces.push_back(face);
        }
    }

    setMidpoint();
}

void WireFrame::clearWireFrame() {
    vertices.clear();
    faces.clear();
    midpoint = vec3{};
}


void WireFrame::setMidpoint() {
    /* Find midpoint */
    for (const auto& vertex : vertices) {
        midpoint.x += vertex.x;
        midpoint.y += vertex.y;
        midpoint.z += vertex.z;
    }
    midpoint.x /= vertices.size();
    midpoint.y /= vertices.size();
    midpoint.z /= vertices.size();

    /* Offset all points to have a midpoint of 0, 0, 0 */
    for (auto& vertex : vertices) {
        vertex -= midpoint;
        calculateMinMax(vertex);
    }
    midpoint = {0, 0, 0};
    initialMidpoint = {0, 0, 0};
}


void WireFrame::updateLocation(const vec3& change) {
    if (change == vec3{0, 0, 0}) return;

    midpoint += change;
    initialMidpoint += change;

    for (const auto& child : children) {
        child->updateLocation(change);
    }
}


void WireFrame::addChild(const WireFrame& child) {
    children.emplace_back(std::make_unique<WireFrame>(child));
}


WireFrame& WireFrame::operator=(const WireFrame& obj) {
    if (this != &obj) {
        AABB::operator=(obj);

        vertices = obj.vertices;
        faces = obj.faces;
        midpoint = obj.midpoint;
        initialMidpoint = obj.initialMidpoint;
        rotation = obj.rotation;
        rotateFlags = obj.rotateFlags;
        expired = obj.expired;

        // parent = obj.parent;
        children.reserve(obj.children.size());
        for (auto& child : obj.children) {
            addChild(*child);
        }
        // std::ranges::transform(obj.children, std::back_inserter(children), []
        //     (const std::shared_ptr<WireFrame>& child) {
        //     return std::make_shared<WireFrame>(*child);
        // });
    }
    return *this;
}


WireFrame& WireFrame::operator=(WireFrame&& obj) noexcept {
    if (this != &obj) {
        AABB::operator=(obj);
        vertices = std::move(obj.vertices);
        faces = std::move(obj.faces);
        midpoint = obj.midpoint;
        initialMidpoint = obj.initialMidpoint;
        rotation = obj.rotation;
        rotateFlags = obj.rotateFlags;
        expired = obj.expired;

        children = std::move(obj.children);
    }
    return *this;
}


// Add move constructor?
// WireFrame::WireFrame(WireFrame&& obj) noexcept : vertices(std::move(obj.vertices))


WireFrame::WireFrame(const WireFrame& obj) : AABB(obj) {
    if (this != &obj) {
        vertices = obj.vertices;
        faces = obj.faces;
        midpoint = obj.midpoint;
        initialMidpoint = obj.initialMidpoint;
        rotation = obj.rotation;
        rotateFlags = obj.rotateFlags;
        expired = obj.expired;

        // parent = obj.parent;
        children.reserve(obj.children.size());
        for (auto& child : obj.children) {
            addChild(*child);
        }
        // std::ranges::transform(obj.children, std::back_inserter(children), []
        //     (const std::shared_ptr<WireFrame>& child) {
        //     return std::make_shared<WireFrame>(*child);
        // });
    }
}


const std::vector<vec3>& WireFrame::getVertices() const { return vertices; }

const std::vector<Triangle>& WireFrame::getFaces() const { return faces; }

const std::vector<std::unique_ptr<WireFrame>>& WireFrame::getChildren() const { return children; }

// const std::weak_ptr<WireFrame>& WireFrame::getParent() const { return parent; }

bool WireFrame::intersects(const std::pair<float, float>& screenCoords, const rndr::Raycast& ray) const {
    vec3 coords = ray.project(screenCoords, midpoint.z + min.z) - midpoint;

    if (AABB::intersects(coords)) return true;

    // for (auto& child : children) {
    //     if (child->intersects(screenCoords, ray)) return true;
    // }
    return std::ranges::any_of(children, [screenCoords, ray](const std::unique_ptr<WireFrame>& child) {
        return child->intersects(screenCoords, ray);
    });
}
