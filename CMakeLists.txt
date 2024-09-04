cmake_minimum_required(VERSION 3.10) # cmake最低版本
project(cmake_test VERSION 1.0)

# 设置C标准
set(CMAKE_C_STANDARD 11)

# 设置库和可执行文件的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)


# 包含头文件
include_directories(${PROJECT_SOURCE_DIR}/include) # ${PROJECT_SOURCE_DIR} 是一个 CMake 变量，指向项目的根目录



# 添加子目录
add_subdirectory(src)
add_subdirectory(lib)

