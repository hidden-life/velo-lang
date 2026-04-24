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

add_library(velo_lexer
        lexer/token_kind.cpp
        lexer/token.cpp
        lexer/lexer.cpp
)

add_library(Velo::Lexer ALIAS velo_lexer)

target_include_directories(velo_lexer
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(velo_lexer
        PUBLIC
        Velo::ProjectOptions
        Velo::ProjectWarnings

        Velo::Source
        Velo::Diagnostic
)

add_library(velo_ast
        ast/ast_printer.cpp
)
add_library(Velo::AST ALIAS velo_ast)

target_include_directories(velo_ast
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(velo_ast
        PUBLIC
        Velo::ProjectOptions
        Velo::ProjectWarnings
        Velo::Source
)

add_library(velo_parser
        parser/parser.cpp
)
add_library(Velo::Parser ALIAS velo_parser)

target_include_directories(velo_parser
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(velo_parser
        PUBLIC
        Velo::ProjectOptions
        Velo::ProjectWarnings

        Velo::AST
        Velo::Lexer
        Velo::Diagnostic
)

add_library(velo_semantic
        semantic/semantic_analyzer.cpp
)

add_library(Velo::Semantic ALIAS velo_semantic)

target_include_directories(velo_semantic
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(velo_semantic
        PUBLIC
        Velo::ProjectOptions
        Velo::ProjectWarnings

        Velo::AST
        Velo::Diagnostic
)

add_library(velo_driver
        driver/driver.cpp
)
add_library(Velo::Driver ALIAS velo_driver)

target_include_directories(velo_driver
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(velo_driver
        PUBLIC
        Velo::ProjectOptions
        Velo::ProjectWarnings

        Velo::Source
        Velo::Diagnostic
        Velo::Lexer
        Velo::AST
        Velo::Parser
        Velo::Semantic
)