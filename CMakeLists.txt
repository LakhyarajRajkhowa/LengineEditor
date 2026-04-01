cmake_minimum_required(VERSION 3.10)
project(Editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------
# Source groups
# -------------------------------

file(GLOB CORE_SOURCES
    "src/core/*.cpp"
    "src/core/*.h"
)

file(GLOB PANEL_SOURCES
    "src/panels/*.cpp"
    "src/panels/*.h"
)

file(GLOB IMGUI_SOURCES
    "src/imgui/*.cpp"
    "src/imgui/*.h"
)

file(GLOB_RECURSE EXTERNAL_SRC
    "src/external/tinyfiledialogs.c"
    "src/external/tinyfiledialogs.h"
    "src/external/ImGuizmo.cpp"
    "src/external/ImGuizmo.h"
)


set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui)
set(EXTERNAL_IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
# -------------------------------
# Visual Studio grouping
# -------------------------------

source_group("core" FILES ${CORE_SOURCES})
source_group("panels" FILES ${PANEL_SOURCES})
source_group("imgui" FILES ${IMGUI_SOURCES})
source_group("external" FILES ${EXTERNAL_SRC})

# -------------------------------
# Create executable
# -------------------------------

add_executable(Editor
    main.cpp
    ${CORE_SOURCES}
    ${PANEL_SOURCES}
    ${IMGUI_SOURCES}
    ${EXTERNAL_IMGUI_SOURCES}
    ${EXTERNAL_SRC}
)

# -------------------------------
# Include directories
# -------------------------------

target_include_directories(Editor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/panels
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/external
)

# -------------------------------
# Link Engine
# -------------------------------

target_link_libraries(Editor
    Lengine
)