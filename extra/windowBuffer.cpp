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

void WindowBuffer::drawSquare(array<int, 2> first, array<int, 2> second, array<int, 2> third, array<int, 2> fourth) {
    drawLine(first[0], first[1], second[0], second[1]);
    drawLine(second[0], second[1], third[0], third[1]);
    drawLine(third[0], third[1], fourth[0], fourth[1]);
    drawLine(fourth[0], fourth[1], first[0], first[1]);
}

array<int, 2> WindowBuffer::projectionMap(const coord& toMap) const {
    /* 3d Projection to 2d Plane */
    // Find the middle of the current window
    double midScreenX = (w / 2.0);
    double midScreenY = (h / 2.0);
    // Calculate the distance from the focal point to the screen
    double distToScreen = midScreenX / tan(FOV * M_PI / 360.0);

    // Calculate the x and y shift with the offset positions
    double xShift = abs(toMap[0] - midScreenX) * (toMap[2] - distToScreen);
    double yShift = abs(toMap[1] - midScreenY) * (toMap[2] - distToScreen);

    if (toMap[2] > 0) {    // Prevent division by 0
        xShift /= toMap[2];
        yShift /= toMap[2];
    }

    int projectedX = (toMap[0] < midScreenX) ? toMap[0] + xShift : toMap[0] - xShift;
    int projectedY = (toMap[1] < midScreenY) ? toMap[1] + yShift : toMap[1] - yShift;
    return {projectedX, projectedY};
}

void WindowBuffer::drawCube(const WireFrame &cube) {
    // Check for entire object being behind the focal point
    bool render = false;
    for (coord point : cube.coordinates) {
        if (point[2] > 0) {
            render = true;
            break;
        }
    }
    // Skips drawing if entire object is behind the focal point
    if (!render) return;

    array<array<int, 2>, 8> projected = {};
    for (int i = 0; i < 8; i++) {
        projected[i] = projectionMap(cube.coordinates[i]);
    }

    // Draw the top and bottom portions of the cube
    drawSquare(projected[0], projected[1], projected[2], projected[3]);
    drawSquare(projected[4], projected[5], projected[6], projected[7]);

    // Draw the lines connecting the top and bottom portions of the cube
    drawLine(projected[0][0], projected[0][1], projected[7][0], projected[7][1]);
    drawLine(projected[3][0], projected[3][1], projected[4][0], projected[4][1]);
    drawLine(projected[2][0], projected[2][1], projected[5][0], projected[5][1]);
    drawLine(projected[1][0], projected[1][1], projected[6][0], projected[6][1]);
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