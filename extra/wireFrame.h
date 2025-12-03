//
// Created by Quinn on 10/28/2025.
//

#ifndef FINALPROJECT_WIREFRAME_H
#define FINALPROJECT_WIREFRAME_H
#include <iostream>
#include <array>
#include <fstream>

#define matrix3 std::array<std::array<double, 3>, 3>

using std::array;

/**
 * A wrapper struct for std::array<double, 3>
 * Designed to store coordinates
 */
struct coord {
    array<double, 3> coordinates{};

    /**
     * Default constructor initializes coordinates to {0, 0, 0}
     */
    coord() = default;

    /**
     * Constructor for coord that takes a double array with 3 values
     * @param init the initial elements for the double array
     */
    coord(const std::initializer_list<double> init) {
        if (init.size() != 3) {
            throw std::invalid_argument("Coord must have exactly 3 elements");
        }
        std::copy(init.begin(), init.end(), coordinates.begin());
    }

    /**
     * Converts a string to a coord
     * @param str a string formatted as "int,int,int" to be parsed
     * @return a coord parsed from the string as {int, int, int}
     */
    static coord stoc(std::string str) {
        double x = std::stoi(str.substr(0, str.find(',')));
        str = str.substr(str.find(',') + 1);
        double y = std::stoi(str.substr(0, str.find(',')));
        str = str.substr(str.find(',') + 1);
        double z = std::stoi(str.substr(0, str.find(',')));
        return {x, y, z};
    }

    /**
     * Adds two coords and returns a coord
     * @param obj the coord to add
     * @return the resulting coord
     */
    coord operator+(const coord& obj) const {
        coord temp = {};
        for (int i = 0; i < 3; i++) {
            temp[i] = coordinates[i] + obj[i];
        }
        return temp;
    }

    /**
     * Subtracts two coords and returns a ooord
     * @param obj the coord to subtract
     * @return the resulting coord
     */
    coord operator-(const coord& obj) const {
        coord temp = {};
        for (int i = 0; i < 3; i++) {
            temp[i] = coordinates[i] - obj[i];
        }
        return temp;
    }

    /**
     * Adds a coord and assigns it to the current coord
     * @param obj the coord to add
     * @return the resulting coord
     */
    coord& operator+=(const coord& obj) {
        for (int i = 0; i < 3; i++) {
            coordinates[i] += obj[i];
        }
        return *this;
    }

    /**
     * Subtracts a coord and assigns it to the current coord
     * @param obj the coord to subtract
     * @return the resulting coord
     */
    coord& operator-=(const coord& obj) {
        for (int i = 0; i < 3; i++) {
            coordinates[i] -= obj[i];
        }
        return *this;
    }

    /**
     * Formats and writes a coordinate to an ostream
     * @param os the stream to write to
     * @param obj the coord to write to the stream
     * @return the stream
     */
    friend std::ostream& operator<<(std::ostream& os, const coord& obj) {
        os << obj[0] << ',' << obj[1] << ',' << obj[2];
        return os;
    }

    /**
     * Checks equivalency of two coords
     * @param obj the coord to compare
     * @return true if they are equal
     */
    bool operator==(const coord& obj) const {
        return coordinates == obj.coordinates;
    }

    /**
     * Gets the element at a certain position
     * @param idx the index of the coord
     * @return the element at idx
     */
    double& operator[](const std::size_t idx) {
        return coordinates[idx];
    }

    /**
     * Gets the constant element at a certain position
     * @param idx the index of the coord
     * @return the constant element at idx
     */
    const double& operator[](const std::size_t idx) const {
        return coordinates[idx];
    }
};

/**
 * Rotation Flags used to check which axes to rotate over
 */
enum rotationFlags {
    rotateX = 1,
    rotateY = 2,
    rotateZ = 4
};

/**
 * A class to store and edit the WireFrame data
 */
class WireFrame {
    int rotateFlags = 0;
    int width = 0;
    int height = 0;
    int depth = 0;

    /**
     * @return the origin of the WireFrame
     */
    coord getOrigin();

    /**
     * @param first the first 3D matrix to multiply
     * @param second the second 3D matrix to multiply
     * @return the resulting 3D matrix
     */
    static matrix3 matrixMult(const matrix3& first, const matrix3& second);
public:
    coord midPoint;
    array<coord, 8> coordinates;

    /**
     * Default constructor initializes all variables to 0
     */
    WireFrame() = default;

    /**
     * Creates a WireFrame with coordinates according to init
     * Will be deprecated later
     * @param init the coordinates of every corner
     */
    WireFrame(std::initializer_list<coord> init);

    /**
     * Creates a WireFrame according to User specifications
     * @param topLeft the top left coordinate of the WireFrame
     * @param width the width of the WireFrame
     * @param height the height of the WireFrame
     * @param depth the depth of the WireFrame
     */
    WireFrame(const coord &topLeft, double width, double height, double depth);

    /* DEPRECATED
    friend std::ostream& operator << (std::ostream& os, const WireFrame& obj) {
        std::cout << obj.topLeft << "\n" << obj.botRight << "\n";
        return os;
    }*/

    /**
     * Checks if 2 WireFrames are equal
     * @param obj the WireFrame to compare
     * @return true if the WireFrames are equal
     */
    bool operator==(const WireFrame& obj) const;

    /**
     * @return the rotation flags of the current WireFrame
     */
    int getRotation();

    /**
     * Sets the rotation of the current WireFrame
     * @param axis new rotation flags
     */
    void setRotation(int axis);

    /**
     * Toggles chosen rotations
     * @param axis chosen axis based on RotationFlags enum
     */
    void toggleRotation(int axis);

    /**
     * Rotates the current WireFrame
     */
    void rotate();

    /**
     * Moves the current WireFrame
     * @param amount the distance to move in each direction
     */
    void updateLocation(const coord &amount);
};


#endif //FINALPROJECT_WIREFRAME_H