# 创建库目标
add_library(MyLib STATIC
    lib/module1.cpp
    lib/module2.cpp
)
# 指定库的头文件
target_include_directories(MyLib PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 创建可执行文件目标
add_executable(test_camke main.cpp)

# 链接库到可执行文件
target_link_libraries(test_camke PRIVATE MyLib)