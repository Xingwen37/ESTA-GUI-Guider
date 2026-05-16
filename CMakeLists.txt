cmake_minimum_required(VERSION 3.10)
project(ESTA_Simulator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
elseif(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
endif()

# 只需要找核心的 SDL2 库即可
find_package(SDL2 REQUIRED)

# 包含头文件路径
include_directories(
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/core/ui
    ${CMAKE_SOURCE_DIR}/core/infra
    ${CMAKE_SOURCE_DIR}/core/profile
    ${CMAKE_SOURCE_DIR}/core/event
    ${CMAKE_SOURCE_DIR}/core/app
    ${CMAKE_SOURCE_DIR}/port
    ${SDL2_INCLUDE_DIRS}
)

# 收集所有源文件
file(GLOB_RECURSE SOURCES
    "core/*.c"
    "port/*.c"
    "simulator/*.c"
)

add_executable(${PROJECT_NAME} ${SOURCES})

# 链接 SDL2 和 math 库
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
if(UNIX)
    target_link_libraries(${PROJECT_NAME} m)
endif()