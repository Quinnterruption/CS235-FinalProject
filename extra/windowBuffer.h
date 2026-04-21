//
// Created by Quinn on 11/1/2025.
//

#ifndef FINALPROJECT_WINDOWBUFFER_H
#define FINALPROJECT_WINDOWBUFFER_H
#include <Windows.h>
#include "wireFrame.h"

/**
 * A struct to maintain a WindowBuffer and render certain objects
 */
struct WindowBuffer {
    unsigned char* memory = nullptr;
    int FOV = 90;
    int w = 0;
    int h = 0;
    int screenDiagInPixels = 0;

    /**
     * Adds a point and its color to the memory buffer to be rendered
     * @param x coordinate
     * @param y coordinate
     * @param r red value 0-255
     * @param g green value 0-255
     * @param b blue value 0-255
     */
    void drawAtSafe(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
        if (x >= w || y >= h || x < 0 || y < 0) {
            return;
        }

        memory[4 * (x + y * w) + 0] = b;
        memory[4 * (x + y * w) + 1] = g;
        memory[4 * (x + y * w) + 2] = r;
        memory[4 * (x + y * w) + 3] = 0;
    }

    /**
     * Assigns the entire memory buffer to the chosen color
     * @param r red value 0-255
     * @param g green value 0-255
     * @param b blue value 0-255
     */
    void clear(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0) {
        unsigned int color = (r << 16) | (g << 8) | b;

        unsigned int* pixel = reinterpret_cast<unsigned int*>(memory);
        int totalPixels = w * h;

        for (int i = 0; i < totalPixels; i++) {
            *pixel++ = color;
        }
    }

    void clearToBlack() {
        memset(memory, 0, w * h * 4);
    }

    /**
     * Calculates the coordinates of a 3D point projected on a 2D plane
     * @param toMap the 3D coordinate to project
     * @return a 2D point with offset coordinates
     */
    std::pair<int, int> projectionMap(const vec3& toMap) const;

    /**
     * Draws a line between two points
     * @param x1 coordinate
     * @param y1 coordinate
     * @param x2 coordinate
     * @param y2 coordinate
     */
    void drawLine(int x1, int y1, int x2, int y2);

    void drawTriangle(const vec3& a, const vec3& b, const vec3& c);

    void processThreads(
        std::vector<Triangle>::const_iterator begin,
        std::vector<Triangle>::const_iterator end,
        const std::vector<vec3>& vertices,
        const vec3& location,
        const Quaternion& rotation);

    void drawWireframe(const WireFrame& wireframe);
};

/**
 * Clears and reallocates the windowBuffer memory
 * @param windowBuffer the current windowBuffer to reset
 * @param bitmapInfo the configuration for the WIN32 buffer
 * @param hwnd the current window
 */
void resetWindowBuffer(WindowBuffer* windowBuffer, BITMAPINFO* bitmapInfo, HWND hwnd);

#endif //FINALPROJECT_WINDOWBUFFER_H
