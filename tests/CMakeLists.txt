include(FetchContent)
FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(velo_smoke_tests
        unit/smote_test.cpp

        unit/source/source_file_test.cpp
        unit/diagnostic/diagnostic_engine_test.cpp

        unit/lexer/lexer_test.cpp
)

target_link_libraries(velo_smoke_tests PRIVATE
        GTest::gtest_main
        Velo::ProjectOptions
        Velo::ProjectWarnings

        Velo::Source
        Velo::Diagnostic
        Velo::Lexer
)

gtest_discover_tests(velo_smoke_tests)