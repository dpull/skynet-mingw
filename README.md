# 关于skynet-mingw [![Build status](https://ci.appveyor.com/api/projects/status/9j45lldyxmfdau3r?svg=true)](https://ci.appveyor.com/project/dpull/skynet-mingw)

[skynet-mingw](https://github.com/dpull/skynet-mingw) 是[skynet](https://github.com/cloudwu/skynet)的windows平台的实现。其主要特点是：

1. skynet 以submodule链接，方便升级，**确保不改**。
1. 仅扩展了700行代码，方便维护。
1. 自动更新skynet，自动构建，自动化测试，确保质量。

## 编译
不想自行编译的朋友可访问 [自动构建平台获取最新的构建版本](https://ci.appveyor.com/project/dpull/skynet-mingw/build/artifacts)。

1. 安装 [MinGW](http://sourceforge.net/projects/mingw/files/)
1. 安装 `gcc g++`
1. 安装 `pthread (dev)`
1. 运行 `MinGW\msys\1.0\msys.bat`
1. 运行 `prepare.sh`
1. 运行 `make`

### 常见问题
1. 建议使用 `MinGW\msys\1.0\msys.bat` 进行编译
1. 错误: `gcc: Command not found`, 解决: 修改 `msys\1.0\etc\fstab` 中的 `/mingw` 路径
1. 当提示缺少类似`dlfcn.h`文件时，建议看看头文件搜索路径是否有问题，举个例子`perl(Strawberry Perl)`中有`gcc`程序，同时它注册了系统环境变量

## 测试

```bash
./skynet.exe examples/config    # Launch first skynet node  (Gate server) and a skynet-master (see config for standalone option)
./3rd/lua/lua examples/client.lua   # Launch a client, and try to input hello.
```

存在的问题，console服务不可用(无法对stdin进行select)， 会提示如下出错信息，暂时没有解决方案。

```bash
stack traceback:
        [C]: in function 'assert'
        ./lualib/socket.lua:361: in function 'socket.lock'
        ./service/console.lua:15: in upvalue 'func'
        ./lualib/skynet.lua:452: in upvalue 'f'
        ./lualib/skynet.lua:105: in function <./lualib/skynet.lua:104>
```

## 相关文档
[开发笔记](http://www.dpull.com/blog/2015-11-08-skynet_mingw) 
