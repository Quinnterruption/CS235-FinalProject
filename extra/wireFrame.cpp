//
// Created by Quinn on 10/28/2025.
//

#include "wireFrame.h"
#include <cmath>

constexpr double DEGREES = 2;
constexpr double THETA = DEGREES * M_PI / 180.0;

const matrix3 xRotate = {{{1.0, 0.0, 0.0},
                        {0.0, cos(THETA), -sin(THETA)},
                        {0.0, sin(THETA), cos(THETA)}}};
const matrix3 yRotate = {{{cos(THETA), 0.0, sin(THETA)},
                        {0.0, 1.0, 0.0},
                        {-sin(THETA), 0.0, cos(THETA)}}};
const matrix3 zRotate = {{{cos(THETA), -sin(THETA), 0.0},
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
    if (rotateFlags == 0) return;

    matrix3 rotateMatrix = {{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};
    if ((rotateFlags & rotateX) == rotateX) {
        rotateMatrix = matrixMult(rotateMatrix, xRotate);
    }
    if ((rotateFlags & rotateY) == rotateY) {
        rotateMatrix = matrixMult(rotateMatrix, yRotate);
    }
    if ((rotateFlags & rotateZ) == rotateZ) {
        rotateMatrix = matrixMult(rotateMatrix, zRotate);
    }

    std::array<coord, 8> newCoord;
    for (int i = 0; i < 8; i++) {
        coordinates[i] -= midPoint;

        for (int j = 0; j < 3; j++) {
            newCoord[i][j] = coordinates[i][0] * rotateMatrix[j][0] + coordinates[i][1] * rotateMatrix[j][1] + coordinates[i][2] * rotateMatrix[j][2];
        }
        newCoord[i] += midPoint;
    }
    coordinates = newCoord;
}

coord WireFrame::getOrigin() {
    double x = (coordinates[0][0] + coordinates[5][0]) / 2.0;
    double y = (coordinates[0][1] + coordinates[5][1]) / 2.0;
    double z = (coordinates[0][2] + coordinates[5][2]) / 2.0;

    return {x, y, z};
}

matrix3 WireFrame::matrixMult(const matrix3& first, const matrix3& second) {
    matrix3 result = {};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
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
    width = abs(coordinates[0][0] - coordinates[1][0]);
    height = abs(coordinates[3][1] - coordinates[4][1]);
    depth = abs(coordinates[1][2] - coordinates[2][2]);
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
    return width == obj.width && midPoint == obj.midPoint && rotateFlags == obj.rotateFlags;
}