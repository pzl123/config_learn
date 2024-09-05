这是一个学习cmake案例

项目管理架构如下
```
MyProject/
├── CMakeLists.txt
├── 3party/
│      └── cJSON.c
│      └── cJSONx.c
├── config/
│         └── config.json
│         └── default_config.json
├── src/
│      ├── main.c
|      ├── CMakeLists.txt
|      ├── config_fun.c
|      ├── hashfunc.c
|
├── lib/
|      ├── CMakeLists.txt
|      ├── lib/
│             └── cJSON.c
│             └── cJSONx.c
│      
├── include/
│          └── get_config.h
|          ├── cJSON/
│          |        └── cJSON.h
|          |        └── cJSONx.h
|          ├── uthash/
│                   └── uthash.h
|                   
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
