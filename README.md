# Bemfa-WOL-Client

一个基于 C++20 编写的轻量级巴法云 (Bemfa) 远程唤醒 (WOL) 客户端。

## 📖 项目简介

本项目通过 TCP 长连接接入 **巴法云 (bemfa.com)** 物联网平台。当你在控制台发送指令后，程序会捕获指令并通过 `ether-wake` 在本地局域网发送魔术包（Magic Packet），从而实现远程唤醒目标电脑。

### 核心特性

* **轻量高效**：采用 C++ 编写，极低内存占用，适合嵌入式设备。
* **稳定可靠**：内置心跳包维持长连接，支持连接断开自动重连。
* **现代 C++**：利用 `std::jthread` 和 `std::stop_token` 优雅管理多线程。

---

## 🛠️ 准备工作

1. **巴法云账号**：前往 [巴法云官网](https://bemfa.com/) 获取你的 `UID`。
2. **创建主题**：在控制台创建一个 **TCP设备** 主题（例如 `wakeonlan006`）。
3. **目标设备**：确保目标电脑已开启 BIOS 中的 "Wake on LAN" 功能，并获取其 **MAC 地址**。
4. **环境依赖**：
* 本地已安装 `ether-wake` 工具（通常位于 `/usr/sbin/ether-wake`）。


---

## 🚀 快速开始

### 1. 编译项目

使用 CLion 导入项目，并配置好目标平台编译的工具链

### 2. 配置环境变量

程序运行依赖以下环境变量：

| 变量名 | 说明 | 示例                                 |
| --- | --- |------------------------------------|
| `UID` | 巴法云私钥 | `88888888af0000000000000000000000` |
| `TOPIC` | 订阅的主题名称 | ` wakeonlan006`                    |
| `MAC` | 目标设备的 MAC 地址 | `AA:BB:CC:DD:EE:FF`                |
| `INTERFACE` | 广播使用的网卡接口 | `br0`                              |
| `LOG_LEVEL` | 日志级别 (可选) | `INFO` / `DEBUG`                   |

### 3. 运行

```bash
export UID="你的私钥"
export TOPIC="你的主题"
export MAC="目标MAC"
export INTERFACE="br-lan"

./wol-client

```

---

## ⚙️ 工作流程

1. **连接**：程序启动后连接至巴法云 TCP 服务器。
2. **订阅**：发送订阅指令关联你的 `TOPIC`。
3. **心跳**：每 60 秒发送一次 `ping` 包维持链路。
4. **唤醒**：当收到推送消息且命令码匹配时，执行：
   `/usr/sbin/ether-wake -b -i [INTERFACE] [MAC]`

## 测试配置
路由器: 斐讯 K2P
架构: mipsel
系统: [Padavan](https://github.com/tsl0922/padavan)

---

## 📄 开源协议

本项目基于 [MIT License](https://www.google.com/search?q=LICENSE) 开源。