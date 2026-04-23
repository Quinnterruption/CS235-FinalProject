//
// Created by Quinn on 11/1/2025.
//

#include <cmath>
#include "windowBuffer.h"

#include <thread>
#include <future>
#include <algorithm>
#include <cstring>
#include <iostream>

using std::array;

/**
 * Draws a line between two screen coordinates
 * @param x1 the first coordinate's x component
 * @param y1 the first coordinate's y component
 * @param x2 the second coordinate's x component
 * @param y2 the second coordinate's y component
 */
void WindowBuffer::drawLine(int x1, int y1, int x2, int y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    double m = dy / dx;

    double m1 = (h - static_cast<double>(y1)) / (w - static_cast<double>(x1));
    double m2 = (0 - static_cast<double>(y1)) / (w - static_cast<double>(x1));
    double m3 = (0 - static_cast<double>(y1)) / (0 - static_cast<double>(x1));
    double m4 = (h - static_cast<double>(y1)) / (0 - static_cast<double>(x1));

    if (y1 < 0) {
        if (y2 < 0) return;

        // double newX = (0 - y1) / m + x1;
        // if (newX < 0 || newX > w) return;
        // std::cout << "Old: " << x1 << ' ' << y1 << '\n';
        // std::cout << "Old: " << x2 << ' ' << y2 << '\n';
        // x1 = static_cast<int>(newX);
        // y1 = 0; // TODO -> should be the y value at the new x value
        // std::cout << "New: " << x1 << ' ' << y1 << '\n';
        if (y2 < 0) return;             // Both out on top
        if (m < m2 && m > m3) return;   // First out on top and no part of the line intersects screen
    }
    if (y1 > h) {
        if (y2 > h) return;

        // double newX = (h - y1) / m + x1;
        // if (newX < 0 || newX > w) return;
        // x1 = static_cast<int>(newX);
        // y1 = h;
        if (y2 > h) return;             // Both out on bottom
        if (m > m1 && m < m4) return;   // First out on bottom and no part of the line intersects screen
    }
    if (x1 < 0) {
        if (x2 < 0) return;             // Both out on left
        if (m < m3 && m < m4 || m > m3 && m > m4) return;   // First out on left and no part of line intersects screen
        // if (x2 < 0) return;
        //
        // double newY = m * (0 - x1) + y1;
        // if (newY < 0 || newY > h) return;
        // x1 = 0;
        // y1 = static_cast<int>(newY);
    }
    if (x1 > w) {
        if (x2 > w) return;

        // double newY = m * (w - x1) + y1;
        // if (newY < 0 || newY > h) return;
        // x1 = w;
        // y1 = static_cast<int>(newY);
        if (x2 > w) return;             // Both out on right
        if (m > m1 && m > m2 || m < m1 && m < m2) return;   // First out on right and no part of line intersects screen
    }

    if (x1 < 0) {           // First out on left -> offset y1 to be in bounds
        double deltaX = -x1;
        double deltaY = m * deltaX;
        x1 = 0;
        y1 += round(deltaY);
    } else if (x1 > w) {    // First out on right -> offset y1 to be in bounds
        double deltaX = x1 - w;
        double deltaY = -m * deltaX;
        x1 = w;
        y1 += round(deltaY);
    }
    if (y1 < 0) {           // First out on top -> offset x1 to be in bounds
        double deltaY = -y1;
        double deltaX = deltaY / m;
        y1 = 0;
        x1 += round(deltaX);
    } else if (y1 > h) {    // First out on bottom -> offset x1 to be in bounds
        double deltaY = y1 - h;
        double deltaX = deltaY / -m;
        y1 = h;
        x1 += round(deltaX);
    }
    if (x2 < 0) {
        double deltaX = -x2;
        double deltaY = m * deltaX;
        x2 = 0;
        y2 += round(deltaY);
    } else if (x2 > w) {
        double deltaX = x2 - w;
        double deltaY = -m * deltaX;
        x2 = w;
        y2 += round(deltaY);
    }
    if (y2 < 0) {
        double deltaY = -y2;
        double deltaX = deltaY / m;
        y2 = 0;
        x2 += round(deltaX);
    } else if (y2 > h) {
        double deltaY = y2 - h;
        double deltaX = deltaY / -m;
        y2 = h;
        x2 += round(deltaX);
    }

    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int newX = 0, newY = 0;

    // Might be able to get rid of newX and newY if you update x1 and y1 directly
    for (int i = 0; i < screenDiagInPixels; i++) {
        drawAtSafe(x1 + newX, y1 + newY, 0, 255, 0);

        if (x1 + newX == x2 && y1 + newY == y2) break;
        // if (x1 + newX != x1 && (x1 + newX == 0 || x1 + newX == w)) {
        //     std::cout << "breaking\n";
        //     break;
        // }
        // if (y1 + newY != y1 && (y1 + newY == 0 || y1 + newY == h)) {
        //     std::cout << "exiting\n";
        //     break;
        // }

        int checkX = newX + sx;
        int checkY = newY + sy;
        double horMove = abs(newY - m * checkX);
        double vertMove = abs(checkY - m * newX);

        if (dx == 0) {
            newY = checkY;
        } else {
            if (vertMove < horMove) {
                newY = checkY;
            } else {
                newX = checkX;
            }
        }
        // if (i == screenDiagInPixels - 1) std::cout << "WHAT\n";
    }
}

/**
 * Turns 3d world coordinates into 2d screen coordinates
 * @param toMap a vec3 with x, y, z values in world coordinates. Z is treated as distance from the camera
 * @return screen coordinates x, y after projecting from 3d to 2d
 */
std::pair<int, int> WindowBuffer::projectionMap(const vec3& toMap) const {
    // if (toMap.z <= 0) return {-1, -1}; // Culls points if they're behind the camera

    /* 3d Projection to 2d Plane */
    double distToObj = toMap.z <= 0 ? 0.01 : toMap.z;
    int projectedX = static_cast<int>((toMap.x / distToObj) * distToScreen + midScreenX);
    int projectedY = static_cast<int>((-toMap.y / distToObj) * distToScreen + midScreenY);

    return {projectedX, projectedY};
}

void WindowBuffer::drawTriangle(const vec3& a, const vec3& b, const vec3& c) {
    if (a.z <= 0 && b.z <= 0 && c.z <= 0) return;   // Cull points that are all behind the focal point

    auto [aProjX, aProjY] = projectionMap(a);
    auto [bProjX, bProjY] = projectionMap(b);
    auto [cProjX, cProjY] = projectionMap(c);
    drawLine(aProjX, aProjY, bProjX, bProjY);
    drawLine(bProjX, bProjY, cProjX, cProjY);
    drawLine(cProjX, cProjY, aProjX, aProjY);
}

void WindowBuffer::processThreads(
    std::vector<Triangle>::const_iterator begin,
    std::vector<Triangle>::const_iterator end,
    const std::vector<vec3>& vertices,
    const vec3& location,
    const Quaternion& rotation) {
    // Can be used to rotate around a specific point relative to the object
    vec3 rotationPoint{0, 0, 0};    // 0, 0, 0 spins around the objects centerpoint

    std::for_each(begin, end, [this, &vertices, location, rotation, rotationPoint](const Triangle& face) {
        drawTriangle(
            vec3::rotate(vertices[face[0]] + rotationPoint, rotation) + location - rotationPoint,
            vec3::rotate(vertices[face[1]] + rotationPoint, rotation) + location - rotationPoint,
            vec3::rotate(vertices[face[2]] + rotationPoint, rotation) + location - rotationPoint);
    });
}

void WindowBuffer::drawWireframe(const WireFrame& wireframe) {
    auto& vertices = wireframe.getVertices();
    auto& faces = wireframe.getFaces();
    const auto& location = wireframe.getLocation();
    const auto& rotation = wireframe.getRotation();
    unsigned int numThreads = std::thread::hardware_concurrency();
    unsigned int chunkSize = faces.size() / numThreads;

    std::vector<std::future<void>> futures;

    for (unsigned int i = 0; i < numThreads; i++) {
        auto begin = faces.begin() + i * chunkSize;
        auto end = (i == numThreads - 1) ? faces.end() : begin + chunkSize;

        futures.push_back(std::async(std::launch::async, [this, begin, end, &vertices, location, rotation] {
            this->processThreads(begin, end, vertices, location, rotation);
        }));
    }

    // Ensure all threads finish
    for (auto& fut : futures) fut.get();
}

float WindowBuffer::getLuma(const int& color) {
    return 0.299f * ((color >> 16) & 0xFF) + 0.587f * ((color >> 8) & 0xFF) + 0.114f * ((color >> 0) & 0xFF);
}

void WindowBuffer::FXAA() {
    unsigned char* output = new unsigned char[4 * w * h];
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            unsigned int middle, north, east, south, west;
            memcpy(&middle, &memory[4 * (x + y * w)], 4);
            memcpy(&north, &memory[4 * (x + (y - 1) * w)], 4);
            memcpy(&east, &memory[4 * ((x + 1) + y * w)], 4);
            memcpy(&south, &memory[4 * (x + (y + 1) * w)], 4);
            memcpy(&west, &memory[4 * ((x - 1) + y * w)], 4);

            float lumaM = getLuma(middle);
            float lumaN = getLuma(north);
            float lumaE = getLuma(east);
            float lumaS = getLuma(south);
            float lumaW = getLuma(west);

            float lumaMin = std::min({lumaM, lumaN, lumaE, lumaS, lumaW});
            float lumaMax = std::max({lumaM, lumaN, lumaE, lumaS, lumaW});

            if (lumaMax - lumaMin < 0.0312f) {  // FXAA threshold
                memcpy(&output[4 * (x + y * w)], &middle, 4);
            } else {
                float blurFactor = (lumaN + lumaE + lumaS + lumaW) * 0.25f;
                float blend = std::abs(blurFactor - lumaM) / (lumaMax - lumaMin);
                blend = std::clamp(blend, 0.0f, 1.0f);

                output[4 * (x + y * w) + 0] = (1.0f - blend) * ((middle >> 16) & 0xFF) + blend * (((north >> 16) & 0xFF) + ((east >> 16) & 0xFF) + ((south >> 16) & 0xFF) + ((west >> 16) & 0xFF)) * 0.25f;
                output[4 * (x + y * w) + 1] = (1.0f - blend) * ((middle >> 8) & 0xFF) + blend * (((north >> 8) & 0xFF) + ((east >> 8) & 0xFF) + ((south >> 8) & 0xFF) + ((west >> 8) & 0xFF)) * 0.25f;
                output[4 * (x + y * w) + 2] = (1.0f - blend) * ((middle >> 0) & 0xFF) + blend * (((north >> 0) & 0xFF) + ((east >> 0) & 0xFF) + ((south >> 0) & 0xFF) + ((west >> 0) & 0xFF)) * 0.25f;
                output[4 * (x + y * w) + 3] = 0;
                // unsigned int newColor = (1.0f - blend) * middle + blend * (north + east + south + west) * 0.25f;
                // output[4 * (x + y * w)] = newColor;
            }
        }
    }

    memcpy(memory, output, 4 * w * h);
    delete[] output;
}

void resetWindowBuffer(WindowBuffer* windowBuffer, BITMAPINFO* bitmapInfo, HWND hwnd) {
    RECT rect = {};
    GetClientRect(hwnd, &rect);
    windowBuffer->h = rect.bottom;
    windowBuffer->w = rect.right;
    windowBuffer->screenDiagInPixels = ceil(sqrt(
        windowBuffer->w * windowBuffer->w + windowBuffer->h * windowBuffer->h));
    windowBuffer->midScreenX = windowBuffer->w / 2.0;
    windowBuffer->midScreenY = windowBuffer->h / 2.0;
    windowBuffer->distToScreen = windowBuffer->midScreenX / tan(windowBuffer->FOV * M_PI / 360.0);

    if (windowBuffer->memory) {
        VirtualFree(windowBuffer->memory, 0, MEM_RELEASE);
    }

    windowBuffer->memory = static_cast<unsigned char*>(VirtualAlloc(nullptr, 4 * windowBuffer->w * windowBuffer->h,
                                                                     MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

    bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFO);
    bitmapInfo->bmiHeader.biWidth = windowBuffer->w;
    bitmapInfo->bmiHeader.biHeight = -windowBuffer->h;
    bitmapInfo->bmiHeader.biPlanes = 1;
    bitmapInfo->bmiHeader.biBitCount = 32;
    bitmapInfo->bmiHeader.biCompression = BI_RGB;
}