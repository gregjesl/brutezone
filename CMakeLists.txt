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

    # Support testing
    enable_testing()
    add_subdirectory(test)

    # Run benchmarks for Unix systems
    if(UNIX)
        add_subdirectory(benchmark)
    endif()

endif()
