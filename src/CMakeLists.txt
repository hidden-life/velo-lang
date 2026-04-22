add_library(velo_source
        source/source_file.cpp
        source/source_manager.cpp
        source/source_location.cpp
        source/source_range.cpp
)

add_library(Velo::Source ALIAS velo_source)

target_include_directories(velo_source
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(velo_source
        PUBLIC
        Velo::ProjectOptions
        Velo::ProjectWarnings
)

add_library(velo_diagnostic
        diagnostic/diagnostic.cpp
        diagnostic/diagnostic_engine.cpp
)

add_library(Velo::Diagnostic ALIAS velo_diagnostic)

target_include_directories(velo_diagnostic
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(velo_diagnostic
        PUBLIC
        Velo::ProjectWarnings
        Velo::ProjectOptions

        Velo::Source
)