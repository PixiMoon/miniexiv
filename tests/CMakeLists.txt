add_executable(miniexiv_test
    test_all.cpp
)

target_link_libraries(miniexiv_test
    PRIVATE
        miniexiv
)

# Копируем тестовое изображение рядом с исполняемым файлом.
add_custom_command(
    TARGET miniexiv_test
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/test.jpg"
        "$<TARGET_FILE_DIR:miniexiv_test>/test.jpg"
    COMMENT "Copying test.jpg to the test directory"
    VERBATIM
)

# На Windows копируем miniexiv.dll рядом с тестом.
if(WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(
        TARGET miniexiv_test
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:miniexiv>"
            "$<TARGET_FILE_DIR:miniexiv_test>"
        COMMENT "Copying miniexiv DLL to the test directory"
        VERBATIM
    )
endif()

add_test(
    NAME miniexiv_test
    COMMAND "$<TARGET_FILE:miniexiv_test>"
)

set_tests_properties(
    miniexiv_test
    PROPERTIES
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:miniexiv_test>"
)