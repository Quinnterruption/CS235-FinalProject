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
    std::filesystem::path filePath = std::filesystem::absolute(fileName);

    std::ifstream file(filePath);
    if (!file.is_open()) throw std::invalid_argument("Could not open file");

    while (file.peek() != EOF) {
        std::string line;
        std::getline(file, line);

        // Remove duplicate spaces -> "      " = " "
        std::string::iterator new_end = std::ranges::unique(line,
            [](const char lhs, const char rhs) { return (lhs == rhs) && (lhs == ' '); }).begin();
        line.erase(new_end, line.end());
        if (line[0] == ' ') line.erase(0, 1);   // Remove front space

        bool isVertex = line[0] == 'v';
        bool isFace = line[0] == 'f';

        if (isVertex) {
            line = line.substr(2);
            vec3 vertex;
            for (int i = 0; i < 3; i++) {
                auto space = line.find(' ');
                vertex[i] = std::stod(line.substr(0, space));
                if (space != std::string::npos) line = line.substr(space + 1);
            }
            vertices.emplace_back(vertex);
        } else if (isFace) {
            line = line.substr(2);
            std::array<std::size_t, 3> face;
            for (int i = 0; i < 3; i++) {
                auto space = line.find(' ');
                face[i] = std::stod(line.substr(0, space));
                if (space != std::string::npos) line = line.substr(space + 1);
            }
            faces.emplace_back(face);
        }
    }
    setMidpoint();
    for (auto& vertex : vertices) {
        std::cout << "x " << vertex.x << " y " << vertex.y << " z " << vertex.z << '\n';
    }
    for (auto& face : faces) {
        std::cout << face[0] << ' ' << face[1] << ' ' << face[2] << '\n';
    }
    std::cout << "x " << midpoint.x << " y " << midpoint.y << " z " << midpoint.z << '\n';
}


void WireFrame::setMidpoint() const {
    vec3 midpoint{};
    for (auto& vertex : vertices) {
        midpoint.x += vertex.x;
        midpoint.y += vertex.y;
        midpoint.z += vertex.z;
    }
    midpoint.x /= vertices.size();
    midpoint.y /= vertices.size();
    midpoint.z /= vertices.size();
}


void WireFrame::updateLocation(const vec3& change) {

}


bool WireFrame::operator==(const WireFrame &obj) const {
    return false;
}