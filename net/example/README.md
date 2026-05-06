# Chat Room Example

这是一个使用 `AsyncUDPSocket` 实现的简单 UDP 聊天室示例。

## 文件说明

- `chat_server.cc` - 聊天服务器，监听指定端口，广播消息给所有连接的客户端
- `chat_client.cc` - 聊天客户端，连接服务器并收发消息

## 编译

```bash
# 在项目根目录执行
ninja -C out/Default base/net:chat_server base/net:chat_client
```

## 运行

### 启动服务器

```bash
./out/Default/chat_server 8888
```

### 启动客户端

```bash
# 连接本地服务器
./out/Default/chat_client 127.0.0.1 8888

# 或者连接远程服务器
./out/Default/chat_client <server_ip> <port>
```

## 使用方法

1. 首先启动服务器
2. 启动一个或多个客户端
3. 在客户端输入消息并按回车发送
4. 所有其他客户端会收到你的消息

### 支持的命令

- `/list` - 显示当前在线用户列表
- `/quit` - 退出聊天室

## 示例会话

### 服务器端
```
Chat server started on port 8888
Waiting for clients...

[SERVER] New client connected: 127.0.0.1:54321
[SERVER] New client connected: 127.0.0.1:54322
[127.0.0.1:54321]: Hello everyone!
[127.0.0.1:54322]: Hi there!
```

### 客户端 1
```
Connected to chat server at 127.0.0.1:8888
Commands: /quit to exit, /list to see online users
Type your message and press Enter to send.

Welcome to the chat room! You are: 127.0.0.1:54321
[127.0.0.1:54322] joined the chat
> Hello everyone!
[127.0.0.1:54322]: Hi there!
> /quit
Goodbye!
```

### 客户端 2
```
Connected to chat server at 127.0.0.1:8888
Commands: /quit to exit, /list to see online users
Type your message and press Enter to send.

Welcome to the chat room! You are: 127.0.0.1:54322
> [127.0.0.1:54321]: Hello everyone!
> Hi there!
[127.0.0.1:54321] left the chat
```

## 代码说明

### 关键概念

1. **PhysicalSocketServer** - 事件循环核心，基于 epoll 实现
2. **AsyncUDPSocket** - 异步 UDP Socket，通过信号通知数据到达
3. **sigslot** - 信号槽机制，用于解耦事件通知

### 服务器工作流程

```cpp
// 1. 创建 SocketServer
PhysicalSocketServer socket_server;

// 2. 创建 UDP Socket 并绑定端口
Socket* socket = socket_server.CreateSocket(AF_INET, SOCK_DGRAM);
SocketAddress bind_addr("0.0.0.0", port);
AsyncUDPSocket* udp_socket = AsyncUDPSocket::Create(socket, bind_addr);

// 3. 连接信号处理函数
udp_socket->SignalReadPacket.connect(this, &ChatServer::OnPacketReceived);

// 4. 运行事件循环
while (running) {
    socket_server.Wait(100);  // 等待事件，超时 100ms
}
```

### 客户端工作流程

```cpp
// 1. 创建 SocketServer
PhysicalSocketServer socket_server;

// 2. 创建 UDP Socket 并绑定到任意端口
Socket* socket = socket_server.CreateSocket(AF_INET, SOCK_DGRAM);
SocketAddress bind_addr("0.0.0.0", 0);
AsyncUDPSocket* udp_socket = AsyncUDPSocket::Create(socket, bind_addr);

// 3. 连接信号处理函数
udp_socket->SignalReadPacket.connect(this, &ChatClient::OnPacketReceived);

// 4. 发送消息到服务器
SocketAddress server_addr(server_ip, server_port);
udp_socket->SendTo(message.c_str(), message.size(), server_addr);

// 5. 运行事件循环
while (running) {
    socket_server.Wait(100);
}
```
