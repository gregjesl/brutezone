cmake_minimum_required (VERSION 2.6)

include_directories(../inc)

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

# BRUTEZONE_MIN_TIME and BRUTEZONE_MAX_TIME should also be tested
add_library("${PROJECT_NAME}_slim"
        ../src/timezone.c
        ../src/timezone_impl.c
    )

target_compile_definitions("${PROJECT_NAME}_slim"
    PUBLIC BRUTEZONE_MIN_TIME=1546304400)
    # 2019-01-01T01:00:00Z

target_compile_definitions("${PROJECT_NAME}_slim"
    PUBLIC BRUTEZONE_MAX_TIME=1640998800)
    # 2022-01-01T01:00:00Z

add_executable(min_max_time_test timezone_gmt_time.c)
target_link_libraries(min_max_time_test "${PROJECT_NAME}_slim")
add_test(NAME "min-max-time" COMMAND min_max_time_test)