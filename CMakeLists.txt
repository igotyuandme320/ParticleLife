cmake_minimum_required(VERSION 3.20)
project(ParticleLife VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Prefer an explicit CMAKE_PREFIX_PATH, otherwise fall back to common Qt install locations.
if(NOT CMAKE_PREFIX_PATH)
    set(QT_PREFIX_CANDIDATES)

    foreach(QT_ENV_VAR QT_ROOT QTDIR)
        if(DEFINED ENV{${QT_ENV_VAR}} AND EXISTS "$ENV{${QT_ENV_VAR}}")
            list(APPEND QT_PREFIX_CANDIDATES "$ENV{${QT_ENV_VAR}}")
        endif()
    endforeach()

    if(WIN32)
        file(GLOB QT_WIN_CANDIDATES LIST_DIRECTORIES true
            "C:/Qt/*/msvc2022_64"
            "C:/Qt/*/msvc2019_64"
        )
        list(SORT QT_WIN_CANDIDATES COMPARE NATURAL ORDER DESCENDING)
        list(APPEND QT_PREFIX_CANDIDATES ${QT_WIN_CANDIDATES})
    else()
        list(APPEND QT_PREFIX_CANDIDATES
            "/opt/homebrew/opt/qt"
            "/opt/homebrew/opt/qtbase"
            "/usr/local/opt/qt"
            "/usr/local/opt/qtbase"
        )
    endif()

    foreach(QT_PREFIX IN LISTS QT_PREFIX_CANDIDATES)
        if(EXISTS "${QT_PREFIX}")
            list(PREPEND CMAKE_PREFIX_PATH "${QT_PREFIX}")
            message(STATUS "Using Qt from ${QT_PREFIX}")
            break()
        endif()
    endforeach()
endif()

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
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "ParticleLife"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
)

# Optimization/source encoding flags tuned per compiler.
if(MSVC)
    target_compile_options(ParticleLife PRIVATE /O2 /utf-8)
else()
    target_compile_options(ParticleLife PRIVATE -O2)
endif()

if(WIN32)
    get_filename_component(QT_ROOT_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_ROOT_DIR}/bin")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ParticleLife POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-translations --verbose 0 "$<TARGET_FILE:ParticleLife>"
            COMMENT "Deploying Qt runtime with windeployqt"
        )
    endif()
endif()
