#对 skynet 的源码没有任何改动的windows版

## 环境
1. 安装MinGW
1. 安装gcc g++
1. 安装pthread (dev)
1. 运行prepare.sh
1. 运行make

## 测试

```C
./skynet.exe examples/config    # Launch first skynet node  (Gate server) and a skynet-master (see config for standalone option)
./3rd/lua/lua examples/client.lua   # Launch a client, and try to input hello.
```

## 常见问题
1. 建议使用 MinGW\msys\1.0\msys.bat 进行编译
1. 错误: gcc: Command not found  解决: 修改msys\1.0\etc\fstab中的/mingw路径
1. 当提示缺少类似dlfcn.h文件时,建议看看头文件搜索路径是否有问题,举个例子perl(Strawberry Perl)中有gcc程序,同时它注册了系统环境变量

## 相关文档
[开发笔记](http://www.dpull.com/blog/skynet_mingw/) 


