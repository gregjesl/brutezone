cmake_minimum_required (VERSION 2.6)

include_directories(../inc)

add_executable(find_timezone find_timezone.c)
target_link_libraries(find_timezone brutezone)

add_executable(timezone_gmt_time timezone_gmt_time.c)
target_link_libraries(timezone_gmt_time brutezone)

add_executable(timezone_local_time timezone_local_time.c)
target_link_libraries(timezone_local_time brutezone)
