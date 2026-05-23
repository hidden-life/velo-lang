cmake_minimum_required(VERSION 3.25...4.2)
project(
        Velo
        VERSION 0.1.0
        DESCRIPTION "Velo programming language and runtime."
        LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# keep a single language standard across the whole project
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(project_options)
include(warnings)

velo_setup_options()
velo_setup_warnings()

add_subdirectory(src)
add_subdirectory(apps/velo)

if (VELO_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()