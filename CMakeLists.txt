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
    src/engines/stafu/computer_database_index.cpp
    src/engines/stafu/computer_string_index.cpp
    src/engines/stafu/computer_hierarchy_candidate.cpp
    src/engines/stafu/computer_navigation_index.cpp)
target_link_libraries(reluva_engines PUBLIC reluva_common PRIVATE reluva_warnings)

# --- executable: SDL3 backend + ImGui launcher --------------------------------
add_executable(reluva
    src/main.cpp
    src/backends/sdl3/sdl_system.cpp
    src/launcher/launcher.cpp
    src/launcher/stafu_runtime_ui.cpp
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
    src/tools/stafu_computer_string_inspect.cpp
    src/tools/stafu_computer_header_compare.cpp
    src/tools/stafu_computer_article_scan.cpp
    src/tools/stafu_computer_reference_layout.cpp
    src/tools/stafu_computer_byte_patterns.cpp
    src/tools/stafu_computer_region_compare.cpp
    src/tools/stafu_computer_run_boundaries.cpp
    src/tools/stafu_computer_run_references.cpp
    src/tools/stafu_computer_primary_correlation.cpp
    src/tools/stafu_computer_source_displacement.cpp
    src/tools/stafu_computer_layout_validate.cpp
    src/tools/stafu_computer_group_surroundings.cpp
    src/tools/stafu_computer_field_layout.cpp
    src/tools/stafu_computer_triple_targets.cpp
    src/tools/stafu_computer_string_lengths.cpp
    src/tools/stafu_computer_gap_lengths.cpp
    src/tools/stafu_computer_global_layout.cpp
    src/tools/stafu_computer_source_ordering.cpp
    src/tools/stafu_computer_offset_base.cpp
    src/tools/stafu_computer_strings.cpp
    src/tools/stafu_computer_rejected_fields.cpp
    src/tools/stafu_computer_text_headers.cpp
    src/tools/stafu_computer_hierarchy.cpp
    src/tools/stafu_computer_boundaries.cpp
    src/tools/stafu_computer_structure.cpp
    src/tools/stafu_computer_navigation.cpp
    src/tools/stafu_computer_content_audit.cpp
    src/tools/stafu_spr_structure.cpp
    src/tools/stafu_bridge_inventory.cpp
    src/tools/stafu_computer_browse.cpp
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

add_executable(reluva-stafu-spr-browser src/tools/stafu_spr_browser.cpp)
add_executable(reluva-stafu-spr-suffix src/tools/stafu_spr_suffix.cpp)
add_executable(reluva-stafu-spr-relations src/tools/stafu_spr_relations.cpp)
add_executable(reluva-stafu-spr-triples src/tools/stafu_spr_triples.cpp)
add_executable(reluva-stafu-spr-sequence src/tools/stafu_spr_sequence.cpp)
add_executable(reluva-stafu-spr-segments src/tools/stafu_spr_segments.cpp)
add_executable(reluva-stafu-spr-structure-pairs src/tools/stafu_spr_structure_pairs.cpp)
add_executable(reluva-stafu-spr-changes src/tools/stafu_spr_changes.cpp)
add_executable(reluva-stafu-spr-headers src/tools/stafu_spr_headers.cpp)
add_executable(reluva-stafu-spr-short-bodies src/tools/stafu_spr_short_bodies.cpp)
add_executable(reluva-stafu-spr-offsets src/tools/stafu_spr_offsets.cpp)
add_executable(reluva-stafu-spr-translations src/tools/stafu_spr_translations.cpp)
add_executable(reluva-stafu-spr-event-structure src/tools/stafu_spr_event_structure.cpp)
add_executable(reluva-stafu-spr-event-bytes src/tools/stafu_spr_event_bytes.cpp)
add_executable(reluva-stafu-spr-dnar src/tools/stafu_spr_dnar.cpp)
add_executable(reluva-stafu-spr-dnar-survey src/tools/stafu_spr_dnar_survey.cpp)
add_executable(reluva-stafu-spr-dnar-relations src/tools/stafu_spr_dnar_relations.cpp)
add_executable(reluva-stafu-spr-dnar-normalized src/tools/stafu_spr_dnar_normalized.cpp)
add_executable(reluva-stafu-spr-dnar-packed src/tools/stafu_spr_dnar_packed.cpp)
add_executable(reluva-stafu-bridge-img src/tools/stafu_bridge_img.cpp)
add_executable(reluva-stafu-bridge-gif src/tools/stafu_bridge_gif.cpp)
add_executable(reluva-stafu-bridge-lzw src/tools/stafu_bridge_lzw.cpp)
add_executable(reluva-stafu-bridge-rgba src/tools/stafu_bridge_rgba.cpp)
add_executable(reluva-stafu-rm-survey src/tools/stafu_rm_survey.cpp)
add_executable(reluva-stafu-rm-layout src/tools/stafu_rm_layout.cpp)
add_executable(reluva-stafu-palette-survey src/tools/stafu_palette_survey.cpp)
target_link_libraries(reluva-stafu-palette-survey PRIVATE reluva_engines reluva_warnings)
add_executable(reluva-stafu-palette-scan src/tools/stafu_palette_scan.cpp)
target_link_libraries(reluva-stafu-palette-scan PRIVATE reluva_engines reluva_warnings)
add_executable(reluva-stafu-rm-raster src/tools/stafu_rm_raster.cpp)
target_link_libraries(reluva-stafu-rm-raster PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-rm-layout PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-rm-survey PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-bridge-rgba PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-bridge-lzw PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-bridge-gif PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-bridge-img PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-dnar-packed PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-dnar-normalized PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-dnar-relations PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-dnar-survey PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-dnar PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-event-bytes PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-event-structure PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-translations PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-offsets PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-short-bodies PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-headers PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-changes PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-structure-pairs PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-segments PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-sequence PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-triples PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-relations PRIVATE reluva_engines reluva_warnings)
add_executable(reluva-stafu-spr-run-experiment src/tools/stafu_spr_run_experiment.cpp)
target_link_libraries(reluva-stafu-spr-run-experiment PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-suffix PRIVATE reluva_engines reluva_warnings)
target_link_libraries(reluva-stafu-spr-browser PRIVATE reluva_engines reluva_warnings)

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

    add_executable(reluva_stafu_computer_strings_tests tests/test_stafu_computer_strings.cpp)
    target_link_libraries(reluva_stafu_computer_strings_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_computer_strings COMMAND reluva_stafu_computer_strings_tests)

    add_executable(reluva_stafu_computer_hierarchy_tests tests/test_stafu_computer_hierarchy.cpp)
    target_link_libraries(reluva_stafu_computer_hierarchy_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_computer_hierarchy COMMAND reluva_stafu_computer_hierarchy_tests)

    add_executable(reluva_stafu_computer_navigation_tests tests/test_stafu_computer_navigation.cpp)
    target_link_libraries(reluva_stafu_computer_navigation_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_computer_navigation COMMAND reluva_stafu_computer_navigation_tests)

    add_executable(reluva_stafu_spr_candidate_tests tests/test_stafu_spr_candidate.cpp)
    target_link_libraries(reluva_stafu_spr_candidate_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_candidate COMMAND reluva_stafu_spr_candidate_tests)

    add_executable(reluva_stafu_spr_container_tests tests/test_stafu_spr_container.cpp)
    target_link_libraries(reluva_stafu_spr_container_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_container COMMAND reluva_stafu_spr_container_tests)

    add_executable(reluva_stafu_spr_sequence_index_tests tests/test_stafu_spr_sequence_index.cpp)
    target_link_libraries(reluva_stafu_spr_sequence_index_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_sequence_index COMMAND reluva_stafu_spr_sequence_index_tests)

    add_executable(reluva_stafu_rm_palette_inventory_tests tests/test_stafu_rm_palette_inventory.cpp)
    target_link_libraries(reluva_stafu_rm_palette_inventory_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_rm_palette_inventory COMMAND reluva_stafu_rm_palette_inventory_tests)

    add_executable(reluva_stafu_rm_palette_scan_tests tests/test_stafu_rm_palette_scan.cpp)
    target_link_libraries(reluva_stafu_rm_palette_scan_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_rm_palette_scan COMMAND reluva_stafu_rm_palette_scan_tests)

    add_executable(reluva_stafu_rm_candidate_palette_tests tests/test_stafu_rm_candidate_palette.cpp)
    target_link_libraries(reluva_stafu_rm_candidate_palette_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_rm_candidate_palette COMMAND reluva_stafu_rm_candidate_palette_tests)

    add_executable(reluva_stafu_rm_section_align_tests tests/test_stafu_rm_section_align.cpp)
    target_link_libraries(reluva_stafu_rm_section_align_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_rm_section_align COMMAND reluva_stafu_rm_section_align_tests)

    add_executable(reluva_stafu_rm_horizontal_shift_tests tests/test_stafu_rm_horizontal_shift.cpp)
    target_link_libraries(reluva_stafu_rm_horizontal_shift_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_rm_horizontal_shift COMMAND reluva_stafu_rm_horizontal_shift_tests)

    add_executable(reluva_stafu_rm_grayscale_tests tests/test_stafu_rm_grayscale.cpp)
    target_link_libraries(reluva_stafu_rm_grayscale_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_rm_grayscale COMMAND reluva_stafu_rm_grayscale_tests)

    add_executable(reluva_stafu_rm_raster_tests tests/test_stafu_rm_raster.cpp)
    target_link_libraries(reluva_stafu_rm_raster_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_rm_raster COMMAND reluva_stafu_rm_raster_tests)

    add_executable(reluva_stafu_rm_layout_tests tests/test_stafu_rm_layout.cpp)
    target_link_libraries(reluva_stafu_rm_layout_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_rm_layout COMMAND reluva_stafu_rm_layout_tests)

    add_executable(reluva_stafu_rm_survey_tests tests/test_stafu_rm_survey.cpp)
    target_link_libraries(reluva_stafu_rm_survey_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_rm_survey COMMAND reluva_stafu_rm_survey_tests)

    add_executable(reluva_stafu_spr_gif_rgba_tests tests/test_stafu_spr_gif_rgba.cpp)
    target_link_libraries(reluva_stafu_spr_gif_rgba_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_gif_rgba COMMAND reluva_stafu_spr_gif_rgba_tests)

    add_executable(reluva_stafu_spr_gif_lzw_tests tests/test_stafu_spr_gif_lzw.cpp)
    target_link_libraries(reluva_stafu_spr_gif_lzw_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_gif_lzw COMMAND reluva_stafu_spr_gif_lzw_tests)

    add_executable(reluva_stafu_spr_gif_structure_tests tests/test_stafu_spr_gif_structure.cpp)
    target_link_libraries(reluva_stafu_spr_gif_structure_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_gif_structure COMMAND reluva_stafu_spr_gif_structure_tests)

    add_executable(reluva_stafu_spr_img_probe_tests tests/test_stafu_spr_img_probe.cpp)
    target_link_libraries(reluva_stafu_spr_img_probe_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_img_probe COMMAND reluva_stafu_spr_img_probe_tests)

    add_executable(reluva_stafu_spr_dnar_packed_tests tests/test_stafu_spr_dnar_packed.cpp)
    target_link_libraries(reluva_stafu_spr_dnar_packed_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_dnar_packed COMMAND reluva_stafu_spr_dnar_packed_tests)

    add_executable(reluva_stafu_spr_pair_relation_tests tests/test_stafu_spr_pair_relation.cpp)
    target_link_libraries(reluva_stafu_spr_pair_relation_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_pair_relation COMMAND reluva_stafu_spr_pair_relation_tests)

    add_executable(reluva_stafu_spr_run_experiment_tests tests/test_stafu_spr_run_experiment.cpp)
    target_link_libraries(reluva_stafu_spr_run_experiment_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_run_experiment COMMAND reluva_stafu_spr_run_experiment_tests)

    add_executable(reluva_stafu_spr_suffix_control_tests tests/test_stafu_spr_suffix_control.cpp)
    target_link_libraries(reluva_stafu_spr_suffix_control_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_suffix_control COMMAND reluva_stafu_spr_suffix_control_tests)

    add_executable(reluva_stafu_spr_terminal_bits_tests tests/test_stafu_spr_terminal_bits.cpp)
    target_link_libraries(reluva_stafu_spr_terminal_bits_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_terminal_bits COMMAND reluva_stafu_spr_terminal_bits_tests)

    add_executable(reluva_stafu_spr_boundary_report_tests tests/test_stafu_spr_boundary_report.cpp)
    target_link_libraries(reluva_stafu_spr_boundary_report_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_boundary_report COMMAND reluva_stafu_spr_boundary_report_tests)

    add_executable(reluva_stafu_spr_pattern_survey_tests tests/test_stafu_spr_pattern_survey.cpp)
    target_link_libraries(reluva_stafu_spr_pattern_survey_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_pattern_survey COMMAND reluva_stafu_spr_pattern_survey_tests)

    add_executable(reluva_stafu_spr_bit_reader_tests tests/test_stafu_spr_bit_reader.cpp)
    target_link_libraries(reluva_stafu_spr_bit_reader_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_bit_reader COMMAND reluva_stafu_spr_bit_reader_tests)

    add_executable(reluva_stafu_spr_suffix_compare_tests tests/test_stafu_spr_suffix_compare.cpp)
    target_link_libraries(reluva_stafu_spr_suffix_compare_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_suffix_compare COMMAND reluva_stafu_spr_suffix_compare_tests)

    add_executable(reluva_stafu_spr_match_boundary_tests tests/test_stafu_spr_match_boundary.cpp)
    target_link_libraries(reluva_stafu_spr_match_boundary_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_match_boundary COMMAND reluva_stafu_spr_match_boundary_tests)

    add_executable(reluva_stafu_spr_shift_verify_tests tests/test_stafu_spr_shift_verify.cpp)
    target_link_libraries(reluva_stafu_spr_shift_verify_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_shift_verify COMMAND reluva_stafu_spr_shift_verify_tests)

    add_executable(reluva_stafu_spr_shift_survey_tests tests/test_stafu_spr_shift_survey.cpp)
    target_link_libraries(reluva_stafu_spr_shift_survey_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_shift_survey COMMAND reluva_stafu_spr_shift_survey_tests)

    add_executable(reluva_stafu_spr_cross_bit_compare_tests tests/test_stafu_spr_cross_bit_compare.cpp)
    target_link_libraries(reluva_stafu_spr_cross_bit_compare_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_cross_bit_compare COMMAND reluva_stafu_spr_cross_bit_compare_tests)

    add_executable(reluva_stafu_spr_bit_survey_tests tests/test_stafu_spr_bit_survey.cpp)
    target_link_libraries(reluva_stafu_spr_bit_survey_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_bit_survey COMMAND reluva_stafu_spr_bit_survey_tests)

    add_executable(reluva_stafu_spr_image_payload_tests tests/test_stafu_spr_image_payload.cpp)
    target_link_libraries(reluva_stafu_spr_image_payload_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_image_payload COMMAND reluva_stafu_spr_image_payload_tests)

    add_executable(reluva_stafu_spr_playback_cursor_tests tests/test_stafu_spr_playback_cursor.cpp)
    target_link_libraries(reluva_stafu_spr_playback_cursor_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_spr_playback_cursor COMMAND reluva_stafu_spr_playback_cursor_tests)

    add_executable(reluva_stafu_screen_manager_tests tests/test_stafu_screen_manager.cpp)
    target_link_libraries(reluva_stafu_screen_manager_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_screen_manager COMMAND reluva_stafu_screen_manager_tests)

    add_executable(reluva_stafu_computer_browser_tests tests/test_stafu_computer_browser.cpp)
    target_link_libraries(reluva_stafu_computer_browser_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_computer_browser COMMAND reluva_stafu_computer_browser_tests)

    add_executable(reluva_stafu_meta_tests tests/test_stafu_meta.cpp)
    target_link_libraries(reluva_stafu_meta_tests PRIVATE reluva_engines reluva_warnings)
    add_test(NAME stafu_meta COMMAND reluva_stafu_meta_tests)
endif()
