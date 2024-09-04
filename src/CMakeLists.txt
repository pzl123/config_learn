# 创建库目标
add_library(MyLib STATIC
    lib/config_fun.c
    lib/cJSON.c
)
# 指定库的头文件
target_include_directories(MyLib PUBLIC ${CMAKE_SOURCE_DIR}/include)

