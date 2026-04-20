cmake_minimum_required(VERSION 3.10)
project(OSC_Simulator C)

# 强制使用 C11 标准，保持代码规范
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 开启常见警告，保证代码质量
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# 寻找系统中的 SDL2 和 SDL2_ttf 库
find_package(SDL2 REQUIRED)

# 包含头文件路径
include_directories(
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/port
    ${SDL2_INCLUDE_DIRS}
)

# 收集所有源文件
file(GLOB SOURCES 
    "core/*.c"
    "port/*.c"
    "simulator/*.c"
)

# 生成可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接对应的动态库 (-lSDL2 -lSDL2_ttf)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} SDL2_ttf)