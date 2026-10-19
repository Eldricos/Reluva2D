cmake_minimum_required(VERSION 3.24)
project(reluva VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RELUVA_USE_SYSTEM_SDL "Use an installed SDL3 instead of building it" OFF)
option(RELUVA_BUILD_TESTS "Build unit tests" ON)

include(FetchContent)

# --- SDL3 -------------------------------------------------------------------
if(RELUVA_USE_SYSTEM_SDL)
    find_package(SDL3 REQUIRED CONFIG)
else()
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.14
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(SDL3)
endif()

# --- Dear ImGui (no CMake support upstream, so build it here) -----------------
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9b
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC SDL3::SDL3)

# --- Warnings -----------------------------------------------------------------
add_library(reluva_warnings INTERFACE)
if(MSVC)
    target_compile_options(reluva_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(reluva_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# --- common: platform-independent core, no SDL allowed here -------------------
add_library(reluva_common STATIC
    src/common/config.cpp
    src/common/fs.cpp
    src/common/md5.cpp
    src/common/detection.cpp
    src/common/text.cpp)
target_include_directories(reluva_common PUBLIC src)
target_link_libraries(reluva_common PRIVATE reluva_warnings)

# --- engines: talk only to common/ --------------------------------------------
add_library(reluva_engines STATIC
    src/engines/registry.cpp
    src/engines/testbed/testbed.cpp
    src/engines/newworlds/newworlds.cpp
    src/engines/newworlds/cat_archive.cpp
    src/engines/newworlds/string_table.cpp
    src/engines/newworlds/texture.cpp
    src/engines/newworlds/geo.cpp
    src/engines/newworlds/gmf.cpp
    src/engines/newworlds/model_textures.cpp
    src/engines/newworlds/map_viewer.cpp
    src/engines/newworlds/model_viewer.cpp
    src/engines/newworlds/texture_viewer.cpp
    src/engines/stafu/stafu.cpp
    src/engines/stafu/source_inventory.cpp
    src/engines/stafu/source_detection.cpp
    src/engines/stafu/zip_archive.cpp
    src/engines/stafu/resource_manager.cpp
    src/engines/stafu/computer_database_index.cpp)
target_link_libraries(reluva_engines PUBLIC reluva_common PRIVATE reluva_warnings)

# --- executable: SDL3 backend + ImGui launcher --------------------------------
add_executable(reluva
    src/main.cpp
    src/backends/sdl3/sdl_system.cpp
    src/launcher/launcher.cpp
    src/tools/stafu_install_extract.cpp
    src/tools/stafu_install_validate.cpp
    src/tools/stafu_install_stage.cpp
    src/tools/stafu_install_from_cd.cpp)
target_link_libraries(reluva PRIVATE reluva_engines imgui SDL3::SDL3 reluva_warnings)

# --- developer tools -----------------------------------------------------------
add_executable(reluva-tool
    src/tools/reluva_tool.cpp
    src/tools/stafu_archive_probe.cpp
    src/tools/stafu_archive_directory.cpp
    src/tools/stafu_archive_reader.cpp
    src/tools/stafu_resource_tool.cpp
    src/tools/stafu_resource_inspect.cpp
    src/tools/stafu_offset_inspect.cpp
    src/tools/stafu_range_inspect.cpp
    src/tools/stafu_reference_inspect.cpp
    src/tools/stafu_index_inspect.cpp
    src/tools/stafu_computer_index.cpp
    src/tools/stafu_computer_record_inspect.cpp
    src/tools/stafu_install_analyze.cpp
    src/tools/stafu_install_manifest.cpp
    src/tools/stafu_install_extract.cpp
    src/tools/stafu_install_validate.cpp
    src/tools/stafu_install_stage.cpp
    src/tools/stafu_install_from_cd.cpp)
# ZIP method 8 requires raw DEFLATE support. Prefer an installed zlib;
# otherwise fetch a pinned upstream release for reproducible Windows builds.
find_package(ZLIB QUIET)
if(NOT ZLIB_FOUND)
    set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(stafu_zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.3.1
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(stafu_zlib)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()
target_link_libraries(reluva_engines PUBLIC ZLIB::ZLIB)
target_link_libraries(reluva-tool PRIVATE reluva_engines reluva_warnings)

add_executable(reluva-stafu-inventory src/tools/stafu_inventory_tool.cpp)
target_link_libraries(reluva-stafu-inventory PRIVATE reluva_engines reluva_warnings)

add_executable(reluva-stafu-detect src/tools/stafu_detect_tool.cpp)
target_link_libraries(reluva-stafu-detect PRIVATE reluva_engines reluva_warnings)

# --- tests ----------------------------------------------------------------------
if(RELUVA_BUILD_TESTS)
    enable_testing()
    add_executable(reluva_tests tests/test_common.cpp)
    target_link_libraries(reluva_tests PRIVATE reluva_common reluva_warnings)
    add_test(NAME common COMMAND reluva_tests)

    add_executable(reluva_newworlds_tests tests/test_newworlds_cat.cpp)
    target_link_libraries(reluva_newworlds_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME newworlds_cat COMMAND reluva_newworlds_tests)

    add_executable(reluva_stafu_inventory_tests tests/test_stafu_inventory.cpp)
    target_link_libraries(reluva_stafu_inventory_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_inventory COMMAND reluva_stafu_inventory_tests)

    add_executable(reluva_stafu_detection_tests tests/test_stafu_detection.cpp)
    target_link_libraries(reluva_stafu_detection_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_detection COMMAND reluva_stafu_detection_tests)

    add_executable(reluva_stafu_zip_tests tests/test_stafu_zip.cpp)
    target_link_libraries(reluva_stafu_zip_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_zip COMMAND reluva_stafu_zip_tests)

    add_executable(reluva_stafu_resource_tests tests/test_stafu_resources.cpp)
    target_link_libraries(reluva_stafu_resource_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_resources COMMAND reluva_stafu_resource_tests)

    add_executable(reluva_stafu_computer_index_tests tests/test_stafu_computer_index.cpp)
    target_link_libraries(reluva_stafu_computer_index_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_computer_index COMMAND reluva_stafu_computer_index_tests)

    add_executable(reluva_stafu_meta_tests tests/test_stafu_meta.cpp)
    target_link_libraries(reluva_stafu_meta_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_meta COMMAND reluva_stafu_meta_tests)
endif()
