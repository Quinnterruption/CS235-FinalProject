//
// Created by Quinn on 10/28/2025.
//

#include "wireFrame.h"
#include <cmath>

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


int WireFrame::getRotation() {
    return rotateFlags;
}

void WireFrame::setRotation(const int axis) {
    rotateFlags = axis;
}

void WireFrame::toggleRotation(const int axis) {
    rotateFlags ^= axis;
}

void WireFrame::rotate() {
    if (rotateFlags == 0) return;   // Prevent unnecessary calculations

    // Create default 3D matrix
    matrix3 rotateMatrix = {{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};
    if ((rotateFlags & rotateX) == rotateX) {   // Check for rotation around X-axis
        rotateMatrix = matrixMult(rotateMatrix, xRotate);   // Multiply matrices
    }
    if ((rotateFlags & rotateY) == rotateY) {   // Check for rotation around Y-axis
        rotateMatrix = matrixMult(rotateMatrix, yRotate);   // Multiply matrices
    }
    if ((rotateFlags & rotateZ) == rotateZ) {   // Check for rotation around Z-axis
        rotateMatrix = matrixMult(rotateMatrix, zRotate);   // Multiply matrices
    }

    array<coord, 8> newCoord;  // New coordinate array
    for (int i = 0; i < newCoord.size(); i++) {
        coordinates[i] -= midPoint; // Center the object around the origin of the screen (top left)

        for (int j = 0; j < 3; j++) {
            // Matrix multiplication to get the new coordinates
            newCoord[i][j] = coordinates[i][0] * rotateMatrix[j][0] + coordinates[i][1] * rotateMatrix[j][1] + coordinates[i][2] * rotateMatrix[j][2];
        }
        newCoord[i] += midPoint;    // Center the object around the origin of the object (midpoint)
    }
    coordinates = newCoord; // Set the coordinates
}

coord WireFrame::getOrigin() {
    double x = (coordinates[0][0] + coordinates[5][0]) / 2.0;
    double y = (coordinates[0][1] + coordinates[5][1]) / 2.0;
    double z = (coordinates[0][2] + coordinates[5][2]) / 2.0;

    return {x, y, z};
}

matrix3 WireFrame::matrixMult(const matrix3& first, const matrix3& second) {
    matrix3 result = {};    // Create new 3D matrix
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            // Basic matrix multiplication
            result[i][j] = first[i][0] * second[0][j] + first[i][1] * second[1][j] + first[i][2] * second[2][j];
        }
    }
    return result;
}

void WireFrame::updateLocation(const coord &amount) {
    if (amount == coord{0, 0, 0}) return;
    for (int i = 0; i < 8; i++) {
        coordinates[i] += amount;
    }
    midPoint += amount;
}

WireFrame::WireFrame(const std::initializer_list<coord> init) {
    if (init.size() != 8) {
        throw std::invalid_argument("Coordinates must have exactly 8 elements");
    }
    std::copy(init.begin(), init.end(), coordinates.begin());
    width = abs(static_cast<int>(coordinates[0][0] - coordinates[1][0]));
    height = abs(static_cast<int>(coordinates[3][1] - coordinates[4][1]));
    depth = abs(static_cast<int>(coordinates[1][2] - coordinates[2][2]));
    midPoint = getOrigin();
}

WireFrame::WireFrame(const coord &topLeft, double width, double height, double depth) : width(width), height(height), depth(depth){
    for (coord& coordinate : coordinates) {
        coordinate = topLeft;
    }
    coordinates[1] += {width, 0, 0};
    coordinates[2] += {width, 0, depth};
    coordinates[3] += {0, 0, depth};
    coordinates[4] += {0, height, depth};
    coordinates[5] += {width, height, depth};
    coordinates[6] += {width, height, 0};
    coordinates[7] += {0, height, 0};
    midPoint = getOrigin();
}

bool WireFrame::operator==(const WireFrame &obj) const {
    return width == obj.width && height == obj.height && depth == obj.depth &&
        midPoint == obj.midPoint && rotateFlags == obj.rotateFlags;
}