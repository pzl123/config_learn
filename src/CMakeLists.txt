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
)