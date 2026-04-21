add_executable(velo main.cpp)

target_compile_definitions(velo PRIVATE VELO_VERSION="${PROJECT_VERSION}")

target_link_libraries(velo PRIVATE
        Velo::ProjectOptions
        Velo::ProjectWarnings
)