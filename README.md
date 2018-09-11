# 关于skynet-mingw [![Build status](https://ci.appveyor.com/api/projects/status/9j45lldyxmfdau3r?svg=true)](https://ci.appveyor.com/project/xiyoo0812/skynet-mingw)

[skynet-mingw](https://github.com/xiyoo0812/skynet-mingw) 是[skynet](https://github.com/cloudwu/skynet)的windows平台的实现。本项目从https://github.com/dpull/skynet-mingw fork而来，对此版本进行了优化和更新。其主要特点是：

1. skynet 以submodule链接，方便升级，**确保不改**。
2. 仅扩展了700行代码，方便维护。
3. 自动更新skynet，自动构建，自动化测试，确保质量。
4. 编译系统使用msys2。

## 编译
1. 安装 msys2(https://github.com/msys2/msys2)，地址（https://github.com/msys2/msys2/wiki/MSYS2-installation）。
2. 安装 gcc make git
```bash
pacman -S gcc
pacman -S make
pacman -S git
```
3. 克隆 git clone https://github.com/xiyoo0812/skynet-mingw
4. 更新 git submodule update --init --recursive
5. 准备 ./prepare.sh
6. 修改 skynet会加载动态库，因为编译lua的时候需要修改mingw的编译配置，加上-DLUA_USE_DLOPEN选项。
7. 编译 make

## 常见问题
1. 加载各种so报错，通常是由于lua库没有使用动态库选项，解决见上面第6步。

## 测试

```bash
./skynet.exe examples/config            # Launch first skynet node  (Gate server) and a skynet-master (see config for standalone option)
./3rd/lua/lua examples/client.lua       # Launch a client, and try to input hello.
```

## 已知问题

1. console服务不可用(无法对stdin进行select)， 会提示如下出错信息，暂时没有解决方案。（同dpull版本）

```bash
stack traceback:
        [C]: in function 'assert'
        ./lualib/socket.lua:361: in function 'socket.lock'
        ./service/console.lua:15: in upvalue 'func'
        ./lualib/skynet.lua:452: in upvalue 'f'
        ./lualib/skynet.lua:105: in function <./lualib/skynet.lua:104>
```

2. 使用 skynet.abort 无法退出，看堆栈卡在了系统中，暂时没有解决方案。（替代方案 os.exit(true) ）（同dpull版本）

```bash
#0  0x77bd718c in ntdll!ZwWaitForMultipleObjects () from C:\WINDOWS\SYSTEM32\ntdll.dll
#1  0x74c0a4fa in WaitForMultipleObjectsEx () from C:\WINDOWS\SYSTEM32\KernelBase.dll
#2  0x74c0a3d8 in WaitForMultipleObjects () from C:\WINDOWS\SYSTEM32\KernelBase.dll
#3  0x6085be1c in pause () from D:\MinGW\msys\1.0\bin\msys-1.0.dll
#4  0x6085ccf1 in msys-1.0!cwait () from D:\MinGW\msys\1.0\bin\msys-1.0.dll
#5  0x6080dff4 in msys-1.0!cygwin_stackdump () from D:\MinGW\msys\1.0\bin\msys-1.0.dll
#6  0x00413fe5 in ?? ()
#7  0x00413e8f in ?? ()
#8  0x00412a1b in ?? ()
#9  0x0040f77b in ?? ()
#10 0x0040f151 in ?? ()
#11 0x00403869 in __mingw_opendir ()
#12 0x0000000a in ?? ()
#13 0x0069fe30 in ?? ()
#14 0x00000000 in ?? ()
```
3. 启动的时候 skynet.readline 报错，有时间再看。
```bash
[:0100000b] lua call [0 to :100000b : 0 msgsz = 56] error 
: ./lualib/skynet.lua:643: ./lualib/skynet.lua:197: ./lualib/skynet/socket.lua:311: assertion failed!
stack traceback:
        [C]: in function 'assert'
        ./lualib/skynet/socket.lua:311: in function 'skynet.socket.readline'
        ./service/console.lua:16: in upvalue 'f'
        ./lualib/skynet.lua:127: in function <./lualib/skynet.lua:126>
stack traceback:
        [C]: in function 'assert'
        ./lualib/skynet.lua:643: in function 'skynet.dispatch_message'
```
