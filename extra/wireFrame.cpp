//
// Created by Quinn on 10/28/2025.
//

#include "wireFrame.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

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

    if ((rotateFlags & rotateX) == rotateX) {   // Rotate around X
        rotation *= quatX;
    }
    if ((rotateFlags & rotateY) == rotateY) {   // Rotate around Y
        rotation *= quatY;
    }
    if ((rotateFlags & rotateZ) == rotateZ) {   // Rotate around Z
        rotation *= quatZ;
    }
    rotation.normalize();
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
            vertices.emplace_back(vertex);
        } else if (isFace) {
            Triangle face{};
            for (int i = 0; i < 3; i++) {
                space = line.find(' ');
                face[i] = stoi(line.substr(0, space)) - 1;
                line = line.substr(space + 1);
            }
            faces.emplace_back(face);
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
    }
    midpoint = {0, 0, 0};
}


void WireFrame::updateLocation(const vec3& change) {
    midpoint += change;
}

const std::vector<vec3>& WireFrame::getVertices() const { return vertices; }

const std::vector<Triangle>& WireFrame::getFaces() const { return faces; }
