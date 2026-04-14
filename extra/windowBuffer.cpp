//
// Created by Quinn on 11/1/2025.
//

#include <cmath>
#include "windowBuffer.h"

using std::array;

void WindowBuffer::drawLine(int x1, int y1, int x2, int y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    double m = dy / dx;
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int newX = 0, newY = 0;

    // Might be able to get rid of newX and newY if you update x1 and y1 directly
    while (true) {
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

// void WindowBuffer::drawLine(int x1, int y1, int x2, int y2) {
//     double dx = x2 - x1;
//     double dy = y2 - y1;
//     double m = dy / dx;
//     int sx = x1 < x2 ? 1 : -1;
//     int sy = y1 < y2 ? 1 : -1;
//     int newX = 0, newY = 0;
//
//     // Might be able to get rid of newX and newY if you update x1 and y1 directly
//     while (true) {
//         drawAtSafe(x1, y1, 0, 255, 0);
//
//         if (x1 == x2 && y1 == y2) break;
//
//         int checkX = x1 + sx;
//         int checkY = y1 + sy;
//         double horMove = abs(y1 - m * checkX);
//         double vertMove = abs(checkY - m * x1);
//
//         if (dx == 0) {
//             y1++;
//         } else {
//             if (vertMove < horMove) {
//                 y1++;
//             } else {
//                 x1++;
//             }
//         }
//     }
// }

std::pair<int, int> WindowBuffer::projectionMap(const vec3& toMap) const {
    if (toMap.z <= 0) return {-1, -1}; // Culls points if they're behind the camera
    // Needs to have a check where ever it's called to ensure the points aren't -1, -1

    /* 3d Projection to 2d Plane */
    // Find the middle of the current window
    double midScreenX = w / 2.0;
    double midScreenY = h / 2.0;
    // Calculate the distance from the focal point to the screen
    double distToScreen = midScreenX / tan(FOV * M_PI / 360.0);

    int projectedX = static_cast<int>((toMap.x / toMap.z) * distToScreen + midScreenX);
    int projectedY = static_cast<int>((-toMap.y / toMap.z) * distToScreen + midScreenY);

    return {projectedX, projectedY};
}

void WindowBuffer::drawTriangle(const vec3& a, const vec3& b, const vec3& c) {
    std::pair<int, int> p1 = projectionMap(a);
    std::pair<int, int> p2 = projectionMap(b);
    std::pair<int, int> p3 = projectionMap(c);
    drawLine(p1.first, p1.second, p2.first, p2.second);
    drawLine(p2.first, p2.second, p3.first, p3.second);
    drawLine(p3.first, p3.second, p1.first, p1.second);
}

void WindowBuffer::drawWireframe(const WireFrame& wireframe) {
    for (auto& face : wireframe.faces) {
        drawTriangle(wireframe.vertices[face[0]], wireframe.vertices[face[1]], wireframe.vertices[face[2]]);
    }
}

void resetWindowBuffer(WindowBuffer* windowBuffer, BITMAPINFO* bitmapInfo, HWND hwnd) {
    RECT rect = {};
    GetClientRect(hwnd, &rect);
    windowBuffer->h = rect.bottom;
    windowBuffer->w = rect.right;

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