cmake_minimum_required(VERSION 3.10)
project(Editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# -------------------------------
# Source groups
# -------------------------------

file(GLOB CORE_SOURCES
    "src/Editor/core/*.cpp"
    "src/Editor/core/*.h"
)

file(GLOB GRAPHICS_SOURCES
    "src/Editor/graphics/*.cpp"
    "src/Editor/graphics/*.h"
)

file(GLOB PANEL_SOURCES
    "src/Editor/panels/*.cpp"
    "src/Editor/panels/*.h"
)

file(GLOB IMGUI_SOURCES
    "src/Editor/imgui/*.cpp"
    "src/Editor/imgui/*.h"
)

file(GLOB_RECURSE EXTERNAL_SRC
    "src/Editor/external/tinyfiledialogs.c"
    "src/Editor/external/tinyfiledialogs.h"
    "src/Editor/external/ImGuizmo.cpp"
    "src/Editor/external/ImGuizmo.h"
    "src/Editor/external/imnodes.h"
    "src/Editor/external/imnodes.cpp"

)


set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Editor/external/imgui)
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
source_group("graphics" FILES ${GRAPHICS_SOURCES})
source_group("panels" FILES ${PANEL_SOURCES})
source_group("imgui" FILES ${IMGUI_SOURCES})
source_group("external" FILES ${EXTERNAL_SRC})

# -------------------------------
# Create executable
# -------------------------------

add_executable(Editor
    main.cpp
    ${CORE_SOURCES}
    ${GRAPHICS_SOURCES}
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
)

# -------------------------------
# Link Engine
# -------------------------------

target_link_libraries(Editor
    Lengine
)