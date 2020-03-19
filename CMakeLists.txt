cmake_minimum_required (VERSION 2.6)

# For integration in a ESP project
# See https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#using-third-party-cmake-projects-with-components
if (ESP_PLATFORM)

    idf_component_register(
        SRCS "src/timezone.c" "src/timezone_impl.c"
        INCLUDE_DIRS "inc"
    )

else()

    project (brutezone)

    include_directories ("${PROJECT_SOURCE_DIR}/inc")

    add_library(${PROJECT_NAME}
        src/timezone.c
        src/timezone_impl.c
    )

    add_subdirectory(generator)

    # Support testing
    enable_testing()
    add_subdirectory(test)

    # Run benchmarks for Unix systems
    if(UNIX)
        add_subdirectory(benchmark)
    endif()

endif()

if(BRUTEZONE_MIN_TIME)
    target_compile_definitions(${PROJECT_NAME}
        PUBLIC BRUTEZONE_MIN_TIME=${BRUTEZONE_MIN_TIME})
    message(STATUS "Brutezone minimum time set to ${BRUTEZONE_MIN_TIME} seconds from 1970-01-01T01:00:00Z")
else()
    message(STATUS "Brutezone using default minimum time of 1970-01-01T01:00:00Z")
endif()
if(BRUTEZONE_MAX_TIME)
    target_compile_definitions(${PROJECT_NAME}
        PUBLIC BRUTEZONE_MAX_TIME=${BRUTEZONE_MAX_TIME})
    message(STATUS "Brutezone maximum time set to ${BRUTEZONE_MAX_TIME} seconds from 1970-01-01T01:00:00Z")
else()
    message(STATUS "Brutezone using default maximum time of 2038-01-19T01:00:00Z")
endif()