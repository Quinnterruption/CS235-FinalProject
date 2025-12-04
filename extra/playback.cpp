//
// File Name: playback.cpp
// Author: TechSolutions
//
//

#include "playback.h"
#include <fstream>
#include <windows.h>
#include "windowBuffer.h"
#include "wireFrame.h"
#include <algorithm>


uint64_t Playback::currentTimeMillis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void Playback::startRecord(int duration) {
    time_t timestamp = time(nullptr);   // Get the current time
    tm date = *localtime(&timestamp);   // Convert current time into a tm struct

    std::string fileName = std::format("{}-{}-{}_{}-{}-{}",     // MM-DD-YYYY_hh-mm-ss
        date.tm_mon, date.tm_mday, date.tm_year + 1900, date.tm_hour, date.tm_min, date.tm_sec);

    startRecord(fileName, duration);
}

void Playback::startRecord(const std::string& name, int duration) {
    outFile.open("../recordings/" + name + ".rndr");
    if (!outFile.is_open()) return;

    // Get the duration, start, and end times in milliseconds
    int durInMS = duration * 1000;
    startTime = currentTimeMillis();
    endTime = startTime + durInMS;

    outFile << durInMS << '\n'; // Write the recording length
}

void Playback::endRecord() {
    if (outFile.is_open()) outFile.close();
}

bool Playback::recording() {
    if (currentTimeMillis() >= endTime) {
        endRecord();
        return false;
    }
    return true;
}

void Playback::update(WireFrame wireFrame) {
    if (wireFrame != prevWireFrame) {
        outFile << currentTimeMillis() - startTime << ' '           // Write the time since the start
            << wireFrame.midPoint - prevWireFrame.midPoint << ' '   // Write the change in coordinates
            << wireFrame.getRotation() << '\n';                     // Write the new rotationFlags
        prevWireFrame = wireFrame;
    }
}

void Playback::replay(HWND hwnd, WindowBuffer& windowBuffer, std::string filePath) {
    using std::string, std::getline;
    // Check for a valid string and .rndr file extension
    if (filePath.empty()) return;
    if (filePath.substr(filePath.length() - 6, 5) != ".rndr") return;

    // Fixes the file path to use forward slashes and removes Windows quotes
    // Opens the file path
    std::ranges::replace(filePath, '\\', '/');
    std::erase(filePath, '"');

    std::ifstream inFile(filePath);
    string input;

    if (!inFile.is_open()) return;

    // Get the starting time from the file
    getline(inFile, input);
    const uint64_t startingTime = currentTimeMillis();
    const uint64_t endingTime = startingTime + std::stoi(input);

    // Create the WireFrame and
    WireFrame cube(coord{-50, -50, -50}, 100, 100, 100);
    while (currentTimeMillis() < endingTime) {
        getline(inFile, input);
        if (input.empty()) return;
        while (true) {  // Continues rendering the current state until the next command from the file
            if (currentTimeMillis() - startingTime >= std::stoi(input.substr(0, input.find(' ')))) {
                int rotateFlags = input.back() - '0';   // Get rotateFlags

                // Get the change in coordinate
                int firstSpace = input.find(' ') + 1;
                int secondSpace = input.find(' ', firstSpace);
                coord change = coord::stoc(input.substr(firstSpace, secondSpace - firstSpace));

                // Update the location and rotation of the cube
                cube.updateLocation(change);
                cube.setRotation(rotateFlags);
                break;
            }
            windowBuffer.clear(20, 20, 20);

            windowBuffer.drawCube(cube);
            cube.rotate();
            SendMessage(hwnd, WM_PAINT, 0, 0);
        }
    }
}
