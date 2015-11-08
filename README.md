# 环境
1. 安装MinGW
2. 安装gcc g++
3. 安装pthread
4. 运行prepare.sh
5. 运行make

# 测试
./skynet examples/config    # Launch first skynet node  (Gate server) and a skynet-master (see config for standalone option)
./3rd/lua/lua examples/client.lua   # Launch a client, and try to input hello.

