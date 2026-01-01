cmake_minimum_required(VERSION 3.14)
project(glic VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Download stb_image if not present
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/stb_image.h")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
        "${CMAKE_SOURCE_DIR}/external/stb_image.h"
        SHOW_PROGRESS
    )
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/stb_image_write.h")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h"
        "${CMAKE_SOURCE_DIR}/external/stb_image_write.h"
        SHOW_PROGRESS
    )
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/glic.cpp
    src/planes.cpp
    src/colorspaces.cpp
    src/segment.cpp
    src/prediction.cpp
    src/quantization.cpp
    src/wavelet.cpp
    src/encoding.cpp
    src/bitio.cpp
    src/effects.cpp
)

# Header files
set(HEADERS
    src/glic.hpp
    src/planes.hpp
    src/colorspaces.hpp
    src/segment.hpp
    src/prediction.hpp
    src/quantization.hpp
    src/wavelet.hpp
    src/encoding.hpp
    src/bitio.hpp
    src/config.hpp
    src/effects.hpp
)

add_executable(glic ${SOURCES} ${HEADERS})

target_include_directories(glic PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external
)

# Compiler warnings
if(MSVC)
    target_compile_options(glic PRIVATE /W4)
else()
    target_compile_options(glic PRIVATE -Wall -Wextra -O2)
endif()

# Installation
install(TARGETS glic RUNTIME DESTINATION bin)
