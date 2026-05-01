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
    };
}



#endif //FINALPROJECT_RAYCAST_H