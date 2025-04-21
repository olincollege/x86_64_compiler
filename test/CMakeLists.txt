# Some compilers warn that the return value of cr_assert gets converted from an
# int to a unsigned char:1. To avoid this error in those compilers, turn off
# conversion warnings for test code.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-conversion")

list(APPEND CMAKE_CTEST_ARGUMENTS --verbose)

# Test for the lexer
add_executable(test_lexer
    test_lexer.c
)
target_link_libraries(test_lexer
    PRIVATE lexer
    PUBLIC  ${CRITERION}
)
add_test(
    NAME test_lexer
    COMMAND test_lexer ${CRITERION_FLAGS}
)

# Test for the parser
add_executable(test_parser
    test_parser.c
)
target_link_libraries(test_parser
    PRIVATE parser
            lexer 
    PUBLIC  ${CRITERION}
)
add_test(
    NAME test_parser
    COMMAND test_parser ${CRITERION_FLAGS}
)

# Test for the codegen
add_executable(test_codegen
    test_codegen.c
)
target_link_libraries(test_codegen
    PRIVATE codegen parser lexer
    PUBLIC  ${CRITERION}
)
add_test(
    NAME test_codegen
    COMMAND test_codegen ${CRITERION_FLAGS}
)

# # Test for the compiler
# add_executable(test_compiler
#     test_compiler.c
# )

# target_link_libraries(test_compiler
#     PUBLIC ${CRITERION}
# )

# add_test(
#     NAME test_compiler
#     COMMAND test_compiler ${CRITERION_FLAGS}
# )
