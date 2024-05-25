cmake_minimum_required(VERSION 3.13)
project(first_square)

if (MSVC)
    # <ranges> support requires /std:c++latest on MSVC
    set(CMAKE_CXX_STANDARD 23)
else (MSVC)
    set(CMAKE_CXX_STANDARD 20)
endif (MSVC)

include(FetchContent)

option(autoVkToolkitStarter_CreateDependencySymlinks
        "Create symbolic links instead of copying dependencies of examples, i.e. DLLs (Windows only) & assets."
        OFF)

add_executable(${PROJECT_NAME}
    src/main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC
    include/)

# Dependencies
## Auto-Vk-Toolkit
include(cmake/Auto-Vk-Toolkit.cmake)
target_include_directories(${PROJECT_NAME} PUBLIC Auto_Vk_Toolkit)
target_link_libraries(${PROJECT_NAME} PUBLIC Auto_Vk_Toolkit)

# Assets
include(${Auto_Vk_Toolkit_SOURCE_DIR}/cmake/post_build_helper/add_post_build_commands.cmake)

get_target_property(autoVkToolkitStarter_BINARY_DIR ${PROJECT_NAME} BINARY_DIR)
add_post_build_commands(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/shaders
    ${autoVkToolkitStarter_BINARY_DIR}/shaders
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    "${PROJECT_SOURCE_DIR}/assets"
    ${autoVkToolkitStarter_CreateDependencySymlinks})
