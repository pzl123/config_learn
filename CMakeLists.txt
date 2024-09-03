cmake_minimum_required(VERSION 3.10) #cmake最低版本
project(cmake_test VERSION 1.0)


#设置C标准
set(CMAKE_C_STANDARD 11)


# 设置静态库的输出目录
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

# 添加 lib 目录下的源文件作为库
add_library(mylib STATIC
    lib/module1.c
    lib/module2.c
)

#包含头文件
include_directories(${PROJECT_SOURCE_DIR}/include)#${PROJECT_SOURCE_DIR} 是一个 CMake 变量，指向项目的根目录


# 设置可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 添加可执行文件
add_executable(MyProject src/main.c)

# 链接库
target_link_libraries(MyProject mylib)


