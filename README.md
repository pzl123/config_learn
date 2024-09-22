# 配置管理
将系统配置做成json文件，通过json文件进行配置管理，方便配置的修改和扩展。

项目管理架构如下
```
MyProject/
├── a.out
├── bin
│   ├── MyProject
│   └── zlog.conf
├── build
├── CMakeLists.txt
├── config-file-dir
│   ├── config
│   │   ├── cde.json
│   │   └── config.json
│   └── default
│       ├── default_cde.json
│       └── default_config.json
├── include
│   ├── CMakeLists.txt
│   ├── config-manage
│   │   ├── include
│   │   │   └── config_manage.h
│   │   └── src
│   │       └── config_manage.c
│   └── observer
│       ├── include
│       │   └── observer.h
│       └── src
│           └── observer.c
├── lib
│   ├── CMakeLists.txt
│   ├── libcJSONlib.so
│   ├── libcJSONxlib.so
│   ├── libconfig_managelib.so
│   ├── libobserverlib.so
│   ├── libzlog.so -> libzlog.so.1
│   ├── libzlog.so.1 -> libzlog.so.1.2
│   └── libzlog.so.1.2
├── src
│   ├── CMakeLists.txt
│   ├── main.c
│   └── zlog.conf
├── test
├── test.c
├── third-party
│   ├── cJSON
│   │   ├── include
│   │   │   └── cJSON.h
│   │   └── src
│   │       └── cJSON.c
│   ├── cJSONx
│   │   ├── include
│   │   │   └── cJSONx.h
│   │   └── src
│   │       └── cJSONx.c
│   ├── CMakeLists.txt
│   ├── uthash
│   │   └── include
│   │       ├── utarray.h
│   │       ├── uthash.h
│   │       ├── utlist.h
│   │       ├── utringbuffer.h
│   │       ├── utstack.h
│   │       └── utstring.h
│   └── zlog
│       └── include
│           └── zlog.h
└── web
│   └── abc.json    
├── README.md         
```

运行项目如下：
在项目根目录下创建一个构建目录

`mkdir build && cd build`

运行cmake以配置项目

`cmake ..` 

使用生成的构建系统文件进行编译，假设生成了 Makefile：编译项目

`make` 

编译完成后，在/bin目录下可以运行生成的可执行文件：

`./MyExecutable`
