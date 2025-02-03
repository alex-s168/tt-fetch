cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(tt-umd)

project(tt-fetch VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(tt-fetch
    main.cpp)

target_include_directories(tt-fetch PRIVATE "tt-umd/device/api")

target_link_libraries(tt-fetch PUBLIC
        fmt::fmt-header-only
        umd::device)

install(TARGETS tt-fetch
        RUNTIME DESTINATION bin)
