//
// Created by Quinn on 11/1/2025.
//

#include <cmath>
#include "windowBuffer.h"

#include <thread>
#include <future>
#include <algorithm>

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
        if (m < m2 && m > m3) return;
    }
    if (y1 > h) {
        if (y2 > h) return;
        if (m > m1 && m < m4) return;
    }
    if (x1 < 0) {
        if (x2 < 0) return;
        if (m < m3 && m < m4 || m > m3 && m > m4) return;
    }
    if (x1 > w) {
        if (x2 > w) return;
        if (m > m1 && m > m2 || m < m1 && m < m2) return;
    }

    if (m != 0) {
        if (x1 < 0) {
            double deltaX = -x1;
            double deltaY = m * deltaX;
            x1 = 0;
            y1 += std::round(deltaY);
        } else if (x1 > w) {
            double deltaX = x1 - w;
            double deltaY = -m * deltaX;
            x1 = w;
            y1 += std::round(deltaY);
        }
        if (y1 < 0) {
            double deltaY = -y1;
            double deltaX = deltaY / m;
            y1 = 0;
            x1 += std::round(deltaX);
        } else if (y1 > h) {
            double deltaY = y1 - h;
            double deltaX = deltaY / -m;
            y1 = h;
            x1 += std::round(deltaX);
        }
        if (x2 < 0) {
            double deltaX = -x2;
            double deltaY = m * deltaX;
            x2 = 0;
            y2 += std::round(deltaY);
        } else if (x2 > w) {
            double deltaX = x2 - w;
            double deltaY = -m * deltaX;
            x2 = w;
            y2 += std::round(deltaY);
        }
        if (y2 < 0) {
            double deltaY = -y2;
            double deltaX = deltaY / m;
            y2 = 0;
            x2 += std::round(deltaX);
        } else if (y2 > h) {
            double deltaY = y2 - h;
            double deltaX = deltaY / -m;
            y2 = h;
            x2 += std::round(deltaX);
        }
    }

    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int newX = 0, newY = 0;

    // Might be able to get rid of newX and newY if you update x1 and y1 directly
    for (int i = 0; i < screenDiagInPixels; i++) {
        drawAtSafe(x1 + newX, y1 + newY, 0, 255, 0);

        if (x1 + newX == x2 && y1 + newY == y2) break;

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
    // Find the middle of the current window
    double midScreenX = w / 2.0;
    double midScreenY = h / 2.0;
    // Calculate the distance from the focal point to the screen
    double distToScreen = midScreenX / tan(FOV * M_PI / 360.0);

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
    std::for_each(begin, end, [this, &vertices, location, rotation](const Triangle& face) {
        drawTriangle(
            vec3::rotate(vertices[face[0]], rotation) + location,
            vec3::rotate(vertices[face[1]], rotation) + location,
            vec3::rotate(vertices[face[2]], rotation) + location);
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

        // futures.push_back(std::async(std::launch::async, &WindowBuffer::drawTriangle, this, begin, end, vertices));
        futures.push_back(std::async(std::launch::async, [this, begin, end, &vertices, location, rotation] {
            this->processThreads(begin, end, vertices, location, rotation);
        }));
    }

    // Ensure all threads finish
    for (auto& fut : futures) fut.get();
}

void resetWindowBuffer(WindowBuffer* windowBuffer, BITMAPINFO* bitmapInfo, HWND hwnd) {
    RECT rect = {};
    GetClientRect(hwnd, &rect);
    windowBuffer->h = rect.bottom;
    windowBuffer->w = rect.right;
    windowBuffer->screenDiagInPixels = ceil(sqrt(
        windowBuffer->w * windowBuffer->w + windowBuffer->h * windowBuffer->h));

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