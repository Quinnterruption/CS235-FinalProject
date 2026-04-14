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

    /**
     * Adds a point and it's color to the memory buffer to be rendered
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
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                memory[4 * (x + y * w) + 0] = b;
                memory[4 * (x + y * w) + 1] = g;
                memory[4 * (x + y * w) + 2] = r;
                memory[4 * (x + y * w) + 3] = 0;
            }
        }
    }

    /**
     * Calculates the coordinates of a 3D point projected on a 2D plane
     * @param toMap the 3D coordinate to project
     * @return a 2D point with offset coordinates
     */
    // array<int, 2> projectionMap(const coord& toMap) const;

    /**
     * Draws a square between four points
     * @param first point
     * @param second point
     * @param third point
     * @param fourth point
     */
    void drawSquare(array<int, 2> first, array<int, 2> second, array<int, 2> third, array<int, 2> fourth);

    /**
     * Draws a line between two points
     * @param x1 coordinate
     * @param y1 coordinate
     * @param x2 coordinate
     * @param y2 coordinate
     */
    void drawLine(int x1, int y1, int x2, int y2);

    /**
     * Projection mapping calculation needs to be moved to a different function
     * Calculates the projection mapping and renders a given WireFrame as a cube
     * @param cube the WireFrame to render
     */
    void drawCube(const WireFrame& cube);

//    void drawLine(int x1, int y1, int x2, int y2) {
//        int dx = abs(x2 - x1);
//        int sx = x1 < x2 ? 1 : -1;
//        int dy = -abs(y2 - y1);
//        int sy = y1 < y2 ? 1 : -1;
//        int err = dx + dy;
//        int e2;
//
//        while (true) {
//            drawAtSafe(x1, y1, 0, 255, 0);
//
//            if (x1 == x2 && y1 == y2) break;
//            e2 = 2 * err;
//            if (e2 >= dy) {
//                err += dy;
//                x1 += sx;
//            }
//            if (e2 <= dx) {
//                err += dx;
//                y1 += sy;
//            }
//        }
//    }
};

/**
 * Clears and reallocates the windowBuffer memory
 * @param windowBuffer the current windowBuffer to reset
 * @param bitmapInfo the configuration for the WIN32 buffer
 * @param hwnd the current window
 */
void resetWindowBuffer(WindowBuffer* windowBuffer, BITMAPINFO* bitmapInfo, HWND hwnd);

#endif //FINALPROJECT_WINDOWBUFFER_H
