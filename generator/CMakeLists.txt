cmake_minimum_required (VERSION 2.6)

add_custom_target(generator
    COMMAND nuget restore brutezone.sln
    COMMAND msbuild brutezone.sln -p:Configuration=Release -p:Platform=\"Any CPU\"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/generator"
)

if(WIN32)
    add_custom_target(database
        COMMAND brutezone.exe
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/generator/bin/Release"
    )
else()
    add_custom_target(database
        COMMAND mono brutezone.exe
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/generator/bin/Release"
    )
endif()