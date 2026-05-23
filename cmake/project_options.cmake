include_guard(GLOBAL)

option(VELO_ENABLE_TESTS "Build Velo test targets." ON)
option(VELO_ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors." ON)

function(velo_setup_options)
    if (TARGET velo_project_options)
        return()
    endif ()

    add_library(velo_project_options INTERFACE)
    add_library(Velo::ProjectOptions ALIAS velo_project_options)

    target_compile_features(velo_project_options INTERFACE cxx_std_23)
endfunction()