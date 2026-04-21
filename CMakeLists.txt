cmake_minimum_required(VERSION 3.25)
project(FinalProject)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXE_LINKER_FLAGS "--static")

add_executable(FinalProject main.cpp
        extra/wireFrame.h extra/wireFrame.cpp
        resources/resource.h resources/resource.rc
        extra/windowBuffer.h extra/windowBuffer.cpp
        extra/playback.h extra/playback.cpp
        extra/triangle.h
        extra/quaternion.h
)
