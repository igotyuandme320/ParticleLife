cmake_minimum_required(VERSION 3.20)
project(ParticleLife VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 installed via Homebrew on Apple Silicon
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt" ${CMAKE_PREFIX_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ParticleWidget.cpp
    src/ControlPanel.cpp
    src/Particle.cpp
)

set(HEADERS
    src/MainWindow.h
    src/ParticleWidget.h
    src/ControlPanel.h
    src/Particle.h
)

qt_add_executable(ParticleLife
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(ParticleLife PRIVATE
    Qt6::Core
    Qt6::Widgets
)

set_target_properties(ParticleLife PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "ParticleLife"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
)

# Optimization flags for better performance
target_compile_options(ParticleLife PRIVATE -O2)
