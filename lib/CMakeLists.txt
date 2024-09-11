# 添加 lib 目录下的源文件作为库（静态库）
#add_library(configlib STATIC
#    lib/cJSON/cJSON.c
#    lib/config_fun.c
#    lib/cJSON/cJSONx.c
#)


# 添加 lib 目录下的源文件作为库（动态库）
add_library(cJSOnlib SHARED
    ${PROJECT_SOURCE_DIR}/3party/cJSON/cJSON.c
    ${PROJECT_SOURCE_DIR}/3party/cJSON/cJSONx.c    
)

add_library(configlib SHARED
    ${PROJECT_SOURCE_DIR}/src/config_fun.c
)

# 包含头文件目录
target_include_directories(cJSOnlib PRIVATE ${PROJECT_SOURCE_DIR}/include/cJSON)
target_include_directories(configlib PRIVATE ${PROJECT_SOURCE_DIR}/include)
