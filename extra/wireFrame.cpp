//
// Created by Quinn on 10/28/2025.
//

#include "wireFrame.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

constexpr double DEGREES = 2;
constexpr double THETA = DEGREES * M_PI / 180.0;

const static matrix3 xRotate = {{{1.0, 0.0, 0.0},
                        {0.0, cos(THETA), -sin(THETA)},
                        {0.0, sin(THETA), cos(THETA)}}};
const static matrix3 yRotate = {{{cos(THETA), 0.0, sin(THETA)},
                        {0.0, 1.0, 0.0},
                        {-sin(THETA), 0.0, cos(THETA)}}};
const static matrix3 zRotate = {{{cos(THETA), -sin(THETA), 0.0},
                        {sin(THETA), cos(THETA), 0.0},
                        {0.0, 0.0, 1.0}}};


WireFrame::WireFrame() = default;


WireFrame::WireFrame(const std::string& fileName) {
    setWireFrame(fileName);
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
                vertex[i] = stod(line.substr(0, space));
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
    // Could probably combine this with the parsing from the file
    for (const auto& vertex : vertices) {
        midpoint.x += vertex.x;
        midpoint.y += vertex.y;
        midpoint.z += vertex.z;
    }
    midpoint.x /= vertices.size();
    midpoint.y /= vertices.size();
    midpoint.z /= vertices.size();
}


void WireFrame::updateLocation(const vec3& change) {
    midpoint += change;
    for (auto& vertex : vertices) {
        vertex += change;
    }
}


const std::vector<vec3>& WireFrame::getVertices() const { return vertices; }

const std::vector<Triangle>& WireFrame::getFaces() const { return faces; }
