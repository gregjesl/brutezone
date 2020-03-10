cmake_minimum_required (VERSION 2.6)

project (brutezone)

include_directories ("${PROJECT_SOURCE_DIR}/inc")

add_library(${PROJECT_NAME}
    src/timezone.c
    src/timezone_impl.c
)

# Support testing
enable_testing()
add_subdirectory(benchmark)
add_subdirectory(test)
