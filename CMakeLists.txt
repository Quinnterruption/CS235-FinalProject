cmake_minimum_required(VERSION 3.25)
project(FinalProject)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXE_LINKER_FLAGS "--static")


find_package(OpenMP REQUIRED)

if (OPENMP_FOUND)
    add_executable(FinalProject main.cpp
            extra/wireFrame.h extra/wireFrame.cpp
            resources/resource.h resources/resource.rc
            extra/windowBuffer.h extra/windowBuffer.cpp
            extra/playback.h extra/playback.cpp
            extra/includes/triangle.h
            extra/includes/quaternion.h
            extra/includes/vec3.h
            extra/includes/raycast.h
            extra/AABB.cpp
            extra/AABB.h
    )
    target_link_libraries(FinalProject PUBLIC OpenMP::OpenMP_CXX)
endif()