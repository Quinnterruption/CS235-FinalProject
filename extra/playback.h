//
// File Name: playback.h
// Author: TechSolutions
//
//

#ifndef FINALPROJECT_PLAYBACK_H
#define FINALPROJECT_PLAYBACK_H
#include <chrono>
#include <fstream>
#include <windef.h>
#include "wireFrame.h"
#include "windowBuffer.h"

/**
 * A class to record updates within the environment and replay them
 */
class Playback {
    uint64_t startTime{};
    uint64_t endTime{};
    std::ofstream outFile;
    WireFrame prevWireFrame;
public:
    /**
     * Default constructor initializes all variables to 0
     */
    Playback() = default;

    /**
     * @return the current time in milliseconds since the Linux Epoch
     */
    static uint64_t currentTimeMillis();

    /**
     * Creates a file with the name "MM-DD-YYYY_HH-MM-SS"
     * Sets startTime and endTime to their respective values
     * @param duration the time to record for
     */
    void startRecord(int duration);

    /**
     * Creates a file with the chosen name
     * Sets startTime and endTime to their respective values
     * @param name the name of the file
     * @param duration the time to record for
     */
    void startRecord(const std::string &name, int duration);

    /**
     * Closes outFile if it is open
     */
    void endRecord();

    /**
     * @return true if outFile is open
     */
    bool recording();

    /**
     * Writes any changes in the given WireFrame to outFile
     * @param wireFrame the WireFrame to write to a file
     */
    void update(WireFrame wireFrame);

    /**
     * Replays all movements from a ".rndr" file
     * @param hwnd the current window
     * @param windowBuffer the current window's WindowBuffer
     * @param filePath the path of the file to read
     */
    static void replay(HWND__* hwnd, WindowBuffer& windowBuffer, std::string filePath);
};


#endif //FINALPROJECT_PLAYBACK_H