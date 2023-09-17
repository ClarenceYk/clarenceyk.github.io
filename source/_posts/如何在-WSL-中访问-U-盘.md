---
title: 如何在 WSL 中访问 U 盘
date: 2023-09-17 10:14:11
updated: 2023-09-17 10:14:11
tags:
- WSL
---

在 WSL 中可以通过 `/mnt/` 目录很方便地访问 Windows 主机的文件系统，但是如果我们 PC 上接 U 盘，这个 U 盘只能在 Windows 系统里访问而不能在 WSL 里访问。如果想要在 WSL 中使用 `dd` 命令读写 U 盘或者挂载 U 盘到 WSL 系统，则需要让 USB 设备连接到 WSL，但是默认情况下 WSL 不支持这种操作所以我们需要对 WSL 进行一些修改。

<!-- more -->

## 自己编译内核

首先需要做的是编译自己的 WSL 内核并且让 WSL 使用我们自己编译的内核，具体的操作步骤可参考这篇：[如何在 WSL 中编译加载内核模块](/2023/09/16/如何在-WSL-中编译加载内核模块)。

在编译之前需要通过 `make menuconfig` 配置内核，将 USB 存储相关的驱动打开：

```
Device Drivers
     ---> USB support
          ---> USB Mass Storage support
```

## 安装 usbip

usbip 是一个服务程序，它可以将 USB 协议通过网络转发，从而实现将一个设备的 USB 设备通过网络转发到另外一台设备。同样我们可以利用这个程序将 USB 设备从 Windows 主机通过本地网络转发到 WSL 系统。

### Windows 上的操作

我们需要安装 `usbipd` 这个服务，可以通过下载安装包的方式，也可以通过 `winget` 安装，我使用后一种：

```
winget install usbipd
```

### WSL 中的操作

直接通过 Ubuntu 的软件仓库安装并启用 `usbip`：

```
sudo apt install linux-tools-generic hwdata
sudo update-alternatives --install /usr/local/bin/usbip usbip /usr/lib/linux-tools/*-generic/usbip 20
```

## usbip 的使用

### Windows 上的操作

首先将 U 盘插入到 PC 上，然后使用管理员权限打开 Windows PowerShell，输入以下命令：

列出所有可用的 USB 设备：

```
usbipd wsl list
```

会得到类似如下输出：

```
BUSID  VID:PID    DEVICE                                                        STATE
1-3    04f2:b569  Integrated Camera                                             Not attached
1-4    138a:0011  Synaptics FP Sensors (WBF) (PID=0011)                         Not attached
1-7    8087:0a2a  英特尔(R) 无线 Bluetooth(R)                                    Not attached
3-2    05e3:0736  USB 大容量存储设备                                             Not attached
```

可以看到 BUSID `3-2` 就是我们的 U 盘，然后就可以把它加入到 WSL 中了：

```
usbipd wsl attach --busid 3-2
```

### WSL 中的操作

使用 `lsusb` 查看 USB 设备是否已经连接到了 WSL 中：

```
lsusb
```

可以看到输出：

```
Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
Bus 001 Device 002: ID 05e3:0736 Genesys Logic, Inc. Colour arc SD Card Reader [PISEN]
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
```

这个 `Bus 001` 就是我们的 U 盘，说明已经成功连上 USB 设备了。

然后通过 `lsblk` 查看是否有新的存储设备出现：

```
lsblk
```

可以看到输出：

```
sdd      8:48   1   7.2G  0 disk
└─sdd1   8:49   1   7.2G  0 part
```

说明 USB 存储设备也成功识别了。接下来我们就可以通过 `dd` 命令进行镜像烧写，或者在 U 盘中已经有文件系统的情况下，挂载 U 盘到 WSL 从而进行文件访问。

在 WSL 中使用完 U 盘之后，我们需要回到 Windows 系统将 U 盘取消连接，同样在 powershell 中输入如下命令：

```
usbipd wsl detach --busid 3-2
```

就可以取消连接 WSL。
