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

    add_custom_target(generator
        COMMAND nuget restore brutezone.sln
        COMMAND msbuild brutezone.sln -p:Configuration=Release -p:Platform=\"Any CPU\"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/generator"
    )

    if(WIN32)
        set(GENERATOR_COMMAND brutezone.exe)
    else()
        set(GENERATOR_COMMAND mono brutezone.exe)
    endif()

    add_custom_target(database
        COMMAND ${GENERATOR_COMMAND}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/generator/bin/Release"
    )

    add_custom_target(future
        COMMAND ${GENERATOR_COMMAND} ARGS --future
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/generator/bin/Release"
    )

    add_dependencies(database generator)
    add_dependencies(future generator)

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
