include_guard(GLOBAL)

function(velo_setup_warnings)
    if (TARGET velo_project_warnings)
        return()
    endif ()

    add_library(velo_project_warnings INTERFACE)
    add_library(Velo::ProjectWarnings ALIAS velo_project_warnings)

    if (MSVC)
        set(_warning_flags /W4 /permissive-)
        if (VELO_ENABLE_WARNINGS_AS_ERRORS)
            list(APPEND _warning_flags /WX)
        endif ()
    else ()
        set(_warning_flags -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wformat=2)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            list(APPEND _warning_flags -Wduplicated-cond -Wlogical-op)
        endif ()

        if (VELO_ENABLE_WARNINGS_AS_ERRORS)
            list(APPEND _warning_flags -Werror)
        endif ()
    endif ()

    target_compile_options(velo_project_warnings INTERFACE ${_warning_flags})
endfunction()