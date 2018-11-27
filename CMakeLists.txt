cmake_minimum_required (VERSION 2.6)

project (brutezone)

include_directories ("${PROJECT_SOURCE_DIR}/inc")

set(CMAKE_CONFIGURATION_TYPES "Release")

add_library(${PROJECT_NAME}
    src/timezone.c
    src/timezone_impl.c
)

add_subdirectory(test)