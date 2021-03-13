---
title: 使用 QEMU 启动 ARM aarch64 架构 Ubuntu 虚拟机
date: 2021-03-01 09:35:03
updated: 2021-03-13 15:51:43
tags:
- 操作方法
- QEMU
- ARM64
- Ubuntu
---

本文将介绍如何使用 QEMU 启动 aarch64 架构的 Ubuntu 操作系统以及宿主机上相关配置的操作方法。

<!-- more -->

## 环境

本文所有操作均在 `Ubuntu20.04` 操作系统上完成。

## 安装 QEMU

安装启动 aarch64 ubuntu 所需的依赖程序：

```shell
sudo apt install qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils virtinst virt-manager qemu
sudo apt install qemu-system-aarch64
sudo apt install qemu-efi-aarch64
sudo apt install qemu-utils
```

## 创建镜像

创建启动必备的 flash 镜像：

```shell
dd if=/dev/zero of=flash1.img bs=1M count=64
dd if=/dev/zero of=flash0.img bs=1M count=64
dd if=/usr/share/qemu-efi-aarch64/QEMU_EFI.fd of=flash0.img conv=notrunc
```

下载想要启动的镜像，本文选用 Ubuntu16.04：

```shell
wget http://ports.ubuntu.com/ubuntu-ports/dists/xenial-updates/main/installer-arm64/current/images/netboot/mini.iso
```

创建空硬盘镜像，此镜像用于安装操作系统：

```shell
qemu-img create ubuntu-image.img 20G
```

至此准备工作完成。

## 安装系统

启动系统安装器：

```shell
qemu-system-aarch64 -nographic -machine virt,gic-version=max -m 512M -cpu max -smp 4 \
-netdev user,id=vnet,hostfwd=:127.0.0.1:0-:22 -device virtio-net-pci,netdev=vnet \
-drive file=ubuntu-image.img,if=none,id=drive0,cache=writeback -device virtio-blk,drive=drive0,bootindex=0 \
-drive file=mini.iso,if=none,id=drive1,cache=writeback -device virtio-blk,drive=drive1,bootindex=1 \
-drive file=flash0.img,format=raw,if=pflash -drive file=flash1.img,format=raw,if=pflash 
```

根据提示信息完成操作系统安装。

安装完成之后通过 `Ctrl-a x` 退出 QEMU 。

## 启动镜像

这里我们希望将虚拟机接入宿主机所在网络，如下图：

{% codeblock 网络连接图 line_number:false %}
+-----------------------------------------------------------------+
|  Host                                                           |
| +---------------------+                                         |
| |                     |                                         |
| | br0:                |                                         |
| |   192.168.199.32/24 +-----+                                   |
| |                     |     |                                   |
| +----+----------------+     |       +-------------------------+ |
|      |                      |       |  Guest                  | |
|      |                      |       | +---------------------+ | |
| +----+----------------+  +--+---+   | |                     | | |
| |                     |  |      |   | | eth0:               | | |
| | enp3s0:             |  | tap0 |   | |   192.168.199.33/24 | | |
| |   192.168.199.30/24 |  |      +-----+                     | | |
| |                     |  |      |   | +---------------------+ | |
| +---------------------+  +------+   +-------------------------+ |
+-----------------------------------------------------------------+
{% endcodeblock %}

配置网桥 `br0`：

```shell
sudo ip link add name br0 type bridge
sudo ip link set dev br0 down
sudo ip addr flush dev br0
sudo ip addr add 192.168.199.32/24 dev br0
sudo ip link set dev br0 up
```

配置 tap 设备 `tap0`：

```shell
sudo ip tuntap add name tap0 mode tap
sudo ip link set dev tap0 up
```

将宿主机网络接口 `enp3s0` 和 `tap0` 接入网桥 `br0`：

```shell
sudo ip link set enp3s0 master br0
sudo ip link set tap0 master br0
```

启动虚拟机系统：

```shell
sudo qemu-system-aarch64 -nographic -machine virt,gic-version=max -m 1G -cpu max -smp 4 \
-netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no \
-device virtio-net-pci,netdev=mynet0,mac=$(qemu-mac-hasher.py ubuntu1604-arm64) \
-drive file=ubuntu-image.img,if=none,id=drive0,cache=writeback \
-device virtio-blk,drive=drive0,bootindex=0 \
-drive file=flash0.img,format=raw,if=pflash -drive file=flash1.img,format=raw,if=pflash \
-device virtio-rng-pci
```

以上命令指定 qemu 使用 tap 设备作为虚拟机网络接口并且指定使用刚刚创建的 `tap0` 接口。

为了指定虚拟器的 MAC 地址我们使用一个脚本来为每个虚拟机生成特定地址。MAC 地址的生成脚本如下：

{% codeblock qemu-mac-hasher.py lang:python %}
#!/usr/bin/env python
# usage: qemu-mac-hasher.py <VMName>

import sys
import zlib

crc = str(hex(zlib.crc32(sys.argv[1].encode("utf-8"))))[-8:]
print("52:54:%s%s:%s%s:%s%s:%s%s" % tuple(crc))
{% endcodeblock %}

虚拟机启动后根据需要配置其网络参数即可。

## 优化网络

以上步骤完成后虚拟机可与宿主机所在网络的其他设备互连（包括宿主机），也可以通过指定的网关连接互联网，但是此时宿主机无法连接互联网，解决方法如下：

删除 `enp3s0` 接口的默认网关：

```shell
sudo ip route del default dev enp3s0
```

为 `br0` 添加默认网关：

```shell
sudo ip route add default via 192.168.199.1 dev br0
```
