#对 skynet 的源码没有任何改动的windows版
[![Build status](https://ci.appveyor.com/api/projects/status/9j45lldyxmfdau3r?svg=true)](https://ci.appveyor.com/project/dpull/skynet-mingw)

1. skynet 以submoudle链接，方便升级，确保不改。
1. 仅扩展了600行代码，方便维护。

存在的问题，console服务不可用（无法对stdin进行select）， 会提示如下出错信息，暂时没想到好法子解决。

```bash
stack traceback:
        [C]: in function 'assert'
        ./lualib/socket.lua:361: in function 'socket.lock'
        ./service/console.lua:15: in upvalue 'func'
        ./lualib/skynet.lua:452: in upvalue 'f'
        ./lualib/skynet.lua:105: in function <./lualib/skynet.lua:104>
```

## 环境
1. 安装 [MinGW](http://sourceforge.net/projects/mingw/files/)
1. 安装gcc g++
1. 安装pthread (dev)
1. 运行 MinGW\msys\1.0\msys.bat
1. 运行prepare.sh
1. 运行make

## 测试

```bash
./skynet.exe examples/config    # Launch first skynet node  (Gate server) and a skynet-master (see config for standalone option)
./3rd/lua/lua examples/client.lua   # Launch a client, and try to input hello.
```

## 常见问题
1. 建议使用 MinGW\msys\1.0\msys.bat 进行编译
1. 错误: gcc: Command not found  解决: 修改msys\1.0\etc\fstab中的/mingw路径
1. 当提示缺少类似dlfcn.h文件时,建议看看头文件搜索路径是否有问题,举个例子perl(Strawberry Perl)中有gcc程序,同时它注册了系统环境变量

## 相关文档
[开发笔记](http://www.dpull.com/blog/skynet_mingw/) 


