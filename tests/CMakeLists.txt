include(FetchContent)
FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(velo_smoke_tests
        unit/smoke_test.cpp

        unit/source/source_file_test.cpp
        unit/diagnostic/diagnostic_engine_test.cpp
        unit/lexer/lexer_test.cpp
        unit/parser/parser_test.cpp
        unit/driver/driver_test.cpp
        unit/ast/ast_printer_test.cpp
)

target_link_libraries(velo_smoke_tests PRIVATE
        GTest::gtest_main
        Velo::ProjectOptions
        Velo::ProjectWarnings

        Velo::Source
        Velo::Diagnostic
        Velo::Lexer
        Velo::AST
        Velo::Parser
        Velo::Driver
)

gtest_discover_tests(velo_smoke_tests)