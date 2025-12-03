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


uint64_t Playback::currentTimeMillis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void Playback::startRecord(int duration) {
    time_t timestamp = time(nullptr);
    tm date = *localtime(&timestamp);

    std::string fileName = std::format("{}-{}-{}_{}-{}-{}",
        date.tm_mon, date.tm_mday, date.tm_year + 1900, date.tm_hour, date.tm_min, date.tm_sec);

    startRecord(fileName, duration);
}

void Playback::startRecord(const std::string& name, int duration) {
    int durInMS = duration * 1000;
    outFile.open("../recordings/" + name + ".rndr");
    if (!outFile.is_open()) return;
    
    startTime = currentTimeMillis();
    endTime = startTime + durInMS;
    outFile << durInMS << '\n';
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
        outFile << currentTimeMillis() - startTime << ' ' << wireFrame.midPoint - prevWireFrame.midPoint << ' ' << wireFrame.getRotation() << '\n';
    }
    prevWireFrame = wireFrame;
}

void Playback::replay(HWND hwnd, WindowBuffer& windowBuffer, const std::string& filePath) {
    using std::string, std::getline;
    if (filePath.empty()) return;
    if (filePath.substr(filePath.length() - 6, 5) != ".rndr") return;

    std::ifstream inFile;
    string input;

    int startOfFileName = filePath.find_last_of('\\') + 1;
    string name = filePath.substr(startOfFileName, filePath.length() - startOfFileName - 1);
    inFile.open("../recordings/" + name);

    if (!inFile.is_open()) return;
    getline(inFile, input);

    const uint64_t startingTime = currentTimeMillis();
    const uint64_t endingTime = startingTime + std::stoi(input);

    WireFrame cube;
    bool firstTime = true;
    while (currentTimeMillis() < endingTime) {
        getline(inFile, input);
        if (input.empty()) return;
        while (true) {
            if (currentTimeMillis() - startingTime >= std::stoi(input.substr(0, input.find(' ')))) {
                if (firstTime) {
                    cube = WireFrame(coord{-50, -50, -50}, 100, 100, 100);
                    firstTime = false;
                }

                int rotateFlags = input.back() - '0';

                int firstSpace = input.find(' ') + 1;
                int secondSpace = input.find(' ', firstSpace);
                coord change = coord::stoc(input.substr(firstSpace, secondSpace - firstSpace));
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
