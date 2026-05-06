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
        drawAtSafe(x1 + newX, y1 + newY, r, g, b);

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

void WindowBuffer::drawTriangle(const vec3& a, const vec3& b, const vec3& c) {
    if (a.z <= 0 && b.z <= 0 && c.z <= 0) return;   // Cull points that are all behind the focal point

    auto [aProjX, aProjY] = raycast.projectionMap(a);
    auto [bProjX, bProjY] = raycast.projectionMap(b);
    auto [cProjX, cProjY] = raycast.projectionMap(c);
    drawLine(aProjX, aProjY, bProjX, bProjY);
    drawLine(bProjX, bProjY, cProjX, cProjY);
    drawLine(cProjX, cProjY, aProjX, aProjY);
}

void WindowBuffer::drawSquare(std::pair<int, int> p1, std::pair<int, int> p2) {
    drawLine(p1.first, p1.second, p1.first, p2.second);
    drawLine(p1.first, p2.second, p2.first, p2.second);
    drawLine(p2.first, p2.second, p2.first, p1.second);
    drawLine(p2.first, p1.second, p1.first, p1.second);
}

void WindowBuffer::showHitBox(const WireFrame& wireFrame) {
    int red = r;
    int green = g;
    int blue = b;

    auto& location = wireFrame.getLocation();
    auto projectedLoc = raycast.projectionMap(location);
    drawAtSafe(projectedLoc.first, projectedLoc.second, 255, 0, 0);

    setColor(0, 255, 255);
    auto projectedMin = raycast.projectionMap(vec3{wireFrame.min.x, wireFrame.min.y, wireFrame.max.z} + location);
    auto projectedMax = raycast.projectionMap(wireFrame.max + location);
    drawSquare(projectedMin, projectedMax);
    projectedMin = raycast.projectionMap(wireFrame.min + location);
    projectedMax = raycast.projectionMap(vec3{wireFrame.max.x, wireFrame.max.y, wireFrame.min.z} + location);
    drawSquare(projectedMin, projectedMax);
    setColor(red, green, blue);
}

void WindowBuffer::drawWireframe(const WireFrame& wireFrame) {
    auto& vertices = wireFrame.getVertices();
    auto& faces = wireFrame.getFaces();
    const auto& location = wireFrame.getLocation();
    const auto& rotation = wireFrame.getRotation();
    unsigned int numThreads = std::thread::hardware_concurrency();
    unsigned int chunkSize = faces.size() / numThreads;

    if (showHitboxes) showHitBox(wireFrame);

    const auto& children = wireFrame.getChildren();
    for (const auto& child : children) {
        drawWireframe(*child);
    }

    std::vector<std::future<void>> futures;

    for (unsigned int i = 0; i < numThreads; i++) {
        auto begin = faces.begin() + i * chunkSize;
        auto end = (i == numThreads - 1) ? faces.end() : begin + chunkSize;

        futures.push_back(std::async(std::launch::async, [this, begin ,end, &vertices, location, rotation] {
            std::for_each(begin, end, [this, &vertices, location, rotation](const Triangle& face) {
                drawTriangle(
                    vec3::rotate(vertices[face[0]], rotation) + location,
                    vec3::rotate(vertices[face[1]], rotation) + location,
                    vec3::rotate(vertices[face[2]], rotation) + location);
            });
        }));
    }

    // Ensure all threads finish
    for (auto& fut : futures) fut.get();
}

float WindowBuffer::getLuma(unsigned int color) {
    return 0.299f * ((color >> 16) & 0xFF) + 0.587f * ((color >> 8) & 0xFF) + 0.114f * ((color >> 0) & 0xFF);
}

void WindowBuffer::FXAA() {
    auto* pixel = reinterpret_cast<unsigned int*>(memory);
    int totalPixels = w * h;

    #pragma omp parallel for    // Enable use of all available cores
    for (int i = 0; i < totalPixels; i++) {
        lumaBuffer[i] = getLuma(pixel[i]);
    }

    #pragma omp parallel for
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            int i = y * w + x;

            float lumaM = lumaBuffer[i];
            float lumaN = lumaBuffer[i - w];
            float lumaE = lumaBuffer[i + 1];
            float lumaS = lumaBuffer[i + w];
            float lumaW = lumaBuffer[i - 1];

            float lumaMin = std::min(lumaN, std::min(lumaN, std::min(lumaE, std::min(lumaS, lumaW))));
            float lumaMax = std::max(lumaN, std::max(lumaN, std::max(lumaE, std::max(lumaS, lumaW))));

            float range = lumaMax - lumaMin;

            if (range < 0.0312f) {  // FXAA threshold
                fxaaBuffer[i] = pixel[i];
                continue;
            }

            float blend = std::abs(((lumaN + lumaE + lumaS + lumaW) * 0.25f) - lumaM) / range;
            if (blend > 1.0f) blend = 1.0f;

            unsigned int cN = pixel[i - w], cE = pixel[i + 1], cS = pixel[i + w], cW = pixel[i - 1], cM = pixel[i];

            float avgR = ((cN >> 16 & 0xFF) + (cS >> 16 & 0xFF) + (cE >> 16 & 0xFF) + (cW >> 16 & 0xFF)) * 0.25f;
            float avgG = ((cN >> 8 & 0xFF)  + (cS >> 8 & 0xFF)  + (cE >> 8 & 0xFF)  + (cW >> 8 & 0xFF))  * 0.25f;
            float avgB = ((cN & 0xFF)       + (cS & 0xFF)       + (cE & 0xFF)       + (cW & 0xFF))       * 0.25f;

            auto red = static_cast<unsigned int>((1.0f - blend) * (cM >> 16 & 0xFF) + blend * avgR);
            auto green = static_cast<unsigned int>((1.0f - blend) * (cM >> 8 & 0xFF) + blend * avgG);
            auto blue = static_cast<unsigned int>((1.0f - blend) * (cM & 0xFF) + blend * avgB);

            fxaaBuffer[i] = (red << 16) | (green << 8) | blue;
        }
    }
    memcpy(memory, fxaaBuffer, 4 * w * h);
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

    windowBuffer->raycast.midScreenX = windowBuffer->w / 2.0;
    windowBuffer->raycast.midScreenY = windowBuffer->h / 2.0;
    windowBuffer->raycast.distToScreen = windowBuffer->midScreenX / tan(windowBuffer->FOV * M_PI / 360.0);

    if (windowBuffer->memory) {
        VirtualFree(windowBuffer->memory, 0, MEM_RELEASE);
    }

    windowBuffer->memory = static_cast<unsigned char*>(VirtualAlloc(nullptr, 4 * windowBuffer->w * windowBuffer->h,
                                                                     MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

    windowBuffer->lumaBuffer = static_cast<float*>(_aligned_malloc(
        windowBuffer->w * windowBuffer->h * sizeof(unsigned int), 32));
    windowBuffer->fxaaBuffer = static_cast<unsigned int*>(_aligned_malloc(
        windowBuffer->w * windowBuffer->h * sizeof(unsigned int), 32));

    bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFO);
    bitmapInfo->bmiHeader.biWidth = windowBuffer->w;
    bitmapInfo->bmiHeader.biHeight = -windowBuffer->h;
    bitmapInfo->bmiHeader.biPlanes = 1;
    bitmapInfo->bmiHeader.biBitCount = 32;
    bitmapInfo->bmiHeader.biCompression = BI_RGB;
}