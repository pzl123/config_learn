# 这是一个学习cmake案例

项目管理架构如下
```
MyProject/
├── CMakeLists.txt
├── third_party/
|   ├── cJSON/  # 单个库的目录
│   |   ├── src/       # 第三方库的源代码
│   |   |   └── cJSON.c
│   │   ├── include/   # 第三方库的头文件
│   |   |   └── cJSON.h
|   |
|   ├── cJSONx/  
│   |   ├── src/       
│   |   |   └── cJSON.c
│   │   ├── include/ 
│   |   |   └── cJSONx.h
|   | 
|   ├── uthash/  
│   |   ├── src/       
│   │   ├── include/ 
│   |   |   └── uthash.h
|
├── config/
│         └── config.json
│         └── default_config.json
├── src/
│      ├── main.c
|      ├── CMakeLists.txt
|
|      
|
├── lib/
|      ├── CMakeLists.txt
|
│      
├── include/
|      ├── config/
│      |   ├── src/       
│      |   |   ├── config_manage.c
│      │   ├── include/ 
│      |   |   └── config_manage.h
|      |
|      ├── observer/
│      |   ├── src/       
│      |   |   ├── observer.c
│      │   ├── include/ 
│      |   |   └── observer.h
|      |
|      ├── uthash/       
│      │   ├── include/ 
│              └── observer.h
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
