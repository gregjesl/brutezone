cmake_minimum_required (VERSION 2.6)

include_directories(../inc)

set(CMAKE_CONFIGURATION_TYPES "Debug")

file(GLOB tests
    "*.c"
)
foreach(test ${tests})
    string(REGEX REPLACE ".*/" "" test_name "${test}")
    string(REGEX REPLACE ".c$" "" test_name "${test_name}")
    add_executable ("${test_name}_test" ${test})
    target_link_libraries("${test_name}_test" ${PROJECT_NAME})
    add_test(NAME ${test_name} COMMAND "${test_name}_test")
endforeach()