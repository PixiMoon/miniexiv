add_executable(miniexiv_test test_all.cpp)

target_link_libraries(miniexiv_test
    PRIVATE
    miniexiv
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test.jpg
    ${CMAKE_CURRENT_BINARY_DIR}/test.jpg
    COPYONLY
)
add_test(
    NAME miniexiv_test
    COMMAND miniexiv_test
)

set_tests_properties(miniexiv_test PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)