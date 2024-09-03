项目管理架构如下
```
MyProject/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
├── lib/
│      └── module1.cpp
│      └──module2.cpp
├── include/
│       └── mylib.h
└── tests/
    ├── test_main.cpp
    └── CMakeLists.txt
```

运行项目如下：
在项目根目录下创建一个构建目录

`mkdir build && cd build`

运行cmake以配置项目

`cmake ..` 

使用生成的构建系统文件进行编译，假设生成了 Makefile：编译项目

`make` 

编译完成后，可以运行生成的可执行文件：

`./MyExecutable`