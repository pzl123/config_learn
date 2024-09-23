# src/CMakeLists.txt

# Add executable
add_executable(MyProject
    main.c
)

# Link with third-party and custom libraries
# target_link_libraries(MyProject
#     pthread
# )

target_link_libraries(MyProject
    PRIVATE
    cJSONlib
    cJSONxlib
    config_managelib
    observerlib
    pthread
    # zlog
    ${PROJECT_SOURCE_DIR}/lib/libzlog.so
    ${PROJECT_SOURCE_DIR}/lib/libfcgi.a
)


# 添加 pcu.fcgi 可执行文件
add_executable(pcu.fcgi
    fcgi/fcgi_main.c
)

# 链接 pcu.fcgi 所需的库
target_link_libraries(pcu.fcgi
    PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/libfcgi.a
)