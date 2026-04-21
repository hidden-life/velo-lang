include(FetchContent)
FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(velo_smoke_tests
        unit/smote_test.cpp
)

target_link_libraries(velo_smoke_tests PRIVATE
        GTest::gtest_main
        Velo::ProjectOptions
        Velo::ProjectWarnings
)

gtest_discover_tests(velo_smoke_tests)