//
// Created by TechSolutions on 5/1/2026.
//

#ifndef FINALPROJECT_RAYCAST_H
#define FINALPROJECT_RAYCAST_H

#include "vec3.h"


namespace rndr {
    struct Raycast {
        int midScreenX, midScreenY, distToScreen;

        /**
         * Turns 3d world coordinates into 2d screen coordinates
         * @param toMap a vec3 with x, y, z values in world coordinates. Z is treated as distance from the camera
         * @return screen coordinates x, y after projecting from 3d to 2d
         */
        std::pair<int, int> projectionMap(const vec3& toMap) const {
            double distToObj = toMap.z <= 0 ? 0.01 : toMap.z;
            int projectedX = static_cast<int>((toMap.x / distToObj) * distToScreen + midScreenX);
            int projectedY = static_cast<int>((-toMap.y / distToObj) * distToScreen + midScreenY);

            return {projectedX, projectedY};
        }

        vec3 project(const std::pair<int, int>& toMap, double zCoord) const {
            vec3 coords;

            double distToObj = zCoord <= 0 ? 0.01 : zCoord;
            coords.x = (toMap.first - midScreenX) / distToScreen * distToObj;
            coords.y = -((toMap.first - midScreenY) / distToScreen * distToObj);
            coords.z = zCoord;

            return coords;
        }
    };
}



#endif //FINALPROJECT_RAYCAST_H