---
title: 用 LXD 容器化 VoIP 服务
tags:
  - LXD
  - VoIP
  - Asterisk
  - FreePBX
  - systemd-nspawn
date: 2020-07-25 10:39:18
updated: 2020-07-26 16:31:33
---

{% img /2020/07/25/create-a-linux-container-of-debian-with-asterisk-freepbx-installed/top_pic.jpg '"top_pic" "top_pic"' %}

将有繁琐配置的开发环境打包进容器中能减少我们的开发负担，本文将介绍如何把基于 Asterisk 的 VoIP 服务容器化。

<!--more-->

## Why

如果你的 VoIP 服务开发环境、测试环境和生产环境在同一台物理机上，也许不会察觉到环境搭建的繁琐以及消耗在配置环境上的时间。让我们来设想这样一个场景：客户指定使用某一个 Linux 发行版安装 Asterisk（或者配合 FreePBX）开发一套存在特定需求的 VoIP 服务，基于这个前提你拿到一台新的设备开始工作：

1. 首先安装配置某个指定 Linux 发行版（花费1、2个小时）；
2. 接着编译、安装、配置 Asterisk（1个小时）；
3. 然后安装配置 FreePBX（花费2小时解决无数个问题）；
4. 最后完成测试（花费1小时）。

终于在5、6个小时后你搭建好了开发环境，接着发现还有测试环境和生产环境需要搭建，于是你再花上2倍于之前的时间完成了环境搭建工作。总算可以开始开发了，你开始实现某一项功能，为了完成这项功能可能：

1. 需要变动 Asterisk 的配置；
2. 需要变动 FreePBX 的接口；
3. 系统某些环境变量、配置参数需要改动。

于是你将这些变动都同步执行到测试和生产环境以保证各平台的环境一致，这将耗费开发人员大量的时间。同时，一个项目中面临几十上百项功能的开发需求，很快你就会发现因为环境配置失步导致的开发流程失控，比如开发环境能实现的功能在测试环境失效，在生产环境出现的问题在开发环境无法复现。

问题总结：

1. 开发人员在搭建环境和保证各平台环境配置同步的过程中耗费大量时间；
2. 手动操作进行配置同步难免会出错，出现运行环境失控的情况。

为了解决这2方面问题，我们可以将服务程序及其所依赖的环境打包进容器中使得整套环境容器化，然后在各个平台分发此容器以保证环境的一致性。

## How

本次测试的环境是 `Debian 10`，以下分4个步骤完成 VoIP 服务容器化：

1. 安装并配置 LXD；
2. 制作根文件系统，其中安装了 Asterisk 以及 FreePBX；
3. 将此根文件系统打包作为基础镜像导入 LXD；
4. 从 LXD 中的基础镜像启动一个实例，并配置网络。

### 安装并配置 LXD

LXD 是一下代开源系统容器、虚拟机管理器。关于 LXD 更为详细的介绍请参考[官方文档](https://linuxcontainers.org/lxd/docs/master/index)。

使用 `snap` 安装 LXD，如果系统没有 snap 先安装 snap：

```shell
sudo apt install snap
sudo snap install core
```

安装 `LXD`：

```shell
sudo snap install lxd
```

将用户添加到 lxd 组：

```shell
sudo usermod -a -G lxd $YOUR_USERNAME
```

#### 配置 LXD

一般情况下直接在命令行中输入 `lxd init` 然后全部选择都使用默认选项即可。这里我希望使用自定义的存储设备作为 lxd 的 `storage pool`。

首先创建一个大小合适的空文件：

```shell
mkdir $HOME/lxd_storage
dd if=/dev/zero of=$HOME/lxd_storage/disk bs=1M count=51200 # 50G
```

创建新的 `loop device` 并将刚刚创建的空文件关联到此设备：

```shell
losetup /dev/loop14 $HOME/lxd_storage/disk # 先查看 /dev 目录下是否已有 loop14，有则换一个如：loop15
sudo reboot # 重启生效配置
```

初始化 lxd 配置：

```shell
uklar@debian:~# lxd init
Would you like to use LXD clustering? (yes/no) [default=no]:
Do you want to configure a new storage pool? (yes/no) [default=yes]:
Name of the new storage pool [default=default]:
Name of the storage backend to use (btrfs, dir) [default=btrfs]:
Would you like to create a new btrfs subvolume under /var/lib/lxd? (yes/no) [default=yes]: no
Create a new BTRFS pool? (yes/no) [default=yes]:
Would you like to use an existing block device? (yes/no) [default=no]: yes
Path to the existing block device: /dev/loop14
Would you like to connect to a MAAS server? (yes/no) [default=no]:
Would you like to create a new local network bridge? (yes/no) [default=yes]:
What should the new bridge be called? [default=lxdbr0]:
What IPv4 address should be used? (CIDR subnet notation, “auto” or “none”) [default=auto]:
What IPv6 address should be used? (CIDR subnet notation, “auto” or “none”) [default=auto]: none
Would you like LXD to be available over the network? (yes/no) [default=no]:
Would you like stale cached images to be updated automatically? (yes/no) [default=yes] no
Would you like a YAML "lxd init" preseed to be printed? (yes/no) [default=no]:
```

### 制作根文件系统

这里使用 `debootstrap` 获取 debian 的根文件系统：

```shell
sudo apt install debootstrap
mkdir /tmp/debian
sudo debootstrap buster /tmp/debian
```

{% img /2020/07/25/create-a-linux-container-of-debian-with-asterisk-freepbx-installed/debootstrap.png '"debootstrap" "debootstrap"' %}

使用 `systemd-nspawn` 以 `chroot` 模式切换到 `/tmp/debian` 中：

```shell
sudo apt install systemd-container # 此软件包中包含了 systemd-nspawn
sudo systemd-container -D /tmp/debian
```

{% img /2020/07/25/create-a-linux-container-of-debian-with-asterisk-freepbx-installed/systemd-nspawn.png '"systemd-nspawn" "systemd-nspawn"' %}

做一些基本配置，如设置 root 用户密码、添加普通用户以及配置网络等，然后退出：

```shell
passwd

useradd -m $USERNAME
passwd $USERNAME

apt install sudo
usermod -a -G sudo $USERNAME
usermod --shell /bin/bash $USERNAME

cat <<EOF >/etc/network/interfaces
# This file describes the network interfaces available on your system
# and how to activate them. For more information, see interfaces(5).

source /etc/network/interfaces.d/*

# The loopback network interface
auto lo
iface lo inet loopback

# The eth0 network interface
auto eth0
iface eth0 inet dhcp
EOF

exit
```

接着使用 `systemd-nspawn` 的容器模式启动 `/tmp/debian` 以 root 身份登陆：

```shell
sudo systemd-nspawn -D /tmp/debian --boot
```

{% img /2020/07/25/create-a-linux-container-of-debian-with-asterisk-freepbx-installed/systemd-nspawn-login-root.png '"systemd-nspawn-login-root" "systemd-nspawn-login-root"' %}

在容器中安装 Asterisk 以及 FreePBX，过程请参考：

- [Asterisk 安装脚本](https://gist.github.com/ClarenceYk/2995d607e1b7678fe0c37665546217aa#file-install_asterisk-sh)
- [FreePBX 安装脚本](https://gist.github.com/ClarenceYk/2995d607e1b7678fe0c37665546217aa#file-install_freepbx-sh)

配置 hostname：

```shell
apt install dbus
hostnamectl set-hostname debian.voip.net
echo '127.0.0.1 debian.voip.net' >>/etc/hosts
```

访问服务器 Web 页面验证容器中的 VoIP 服务是否正常运行：

{% img /2020/07/25/create-a-linux-container-of-debian-with-asterisk-freepbx-installed/container-freepbx.png '"container-freepbx" "container-freepbx"' %}

从 `systemd-nspawn` 容器中退出：

```shell
shutdown now
```

### 创建 LXD 镜像

将刚刚配置完成的根文件系统打包并压缩：

```shell
mkdir -p ~/container/images/voip_base
cd ~/container/images/voip_base
sudo tar czf voip_base.tar.gz -C /tmp/debian .
```

为镜像创建 metadata 文件：

```shell
cat <<EOF >./metadata.yaml
architecture: "x86_64"
creation_date: $(date +%s)
properties:
  architecture: "x86_64"
  description: "Debian(buster) with preinstalled Asterisk13&FreePBX15 ($(date +%Y%m%d))"
  os: "debian"
  release: "buster"
EOF

tar czf metadata.tar.gz metadata.yaml
```

将压缩包作为镜像导入 LXD：

```shell
lxc image import \
~/container/images/voip_base/metadata.tar.gz \
~/container/images/voip_base/voip_base.tar.gz \
--alias voip-base
```

查看镜像是否成功导入：

```shell
lxc image list
```

{% img /2020/07/25/create-a-linux-container-of-debian-with-asterisk-freepbx-installed/lxc-image-list.png '"lxc-image-list" "lxc-image-list"' %}

### 从镜像启动实例

有了基础镜像之后，启动一个新的 VoIP 服务实例只需一行命令：

```shell
lxc launch voip-base test
```

查看实例运行状态：

```shell
lxc list
```

{% img /2020/07/25/create-a-linux-container-of-debian-with-asterisk-freepbx-installed/lxc-instance-list.png '"lxc-instance-list" "lxc-instance-list"' %}

可以看到 VoIP 服务实例（名字为 test）正在运行，分配的内部 IP 地址是 `10.72.18.48`。

可用以下命令登陆到实例中：

```shell
lxc exec test bash
```

查看各类服务使用的端口：

```shell
lsof -i -P -n
```

{% img /2020/07/25/create-a-linux-container-of-debian-with-asterisk-freepbx-installed/listen-ports.png '"listen-ports" "listen-ports"' %}

### 宿主机网络配置

让宿主机（IP 为 `192.168.0.107`）所在网络（`192.168.0.0/24`）的其他设备能够访问 VoIP 服务（IP 为 `10.72.18.48`），需要在宿主机上配置网络地址转换（NAT）：

- 发送到宿主机 `5060` 端口的 UDP 包（PJSIP）转发到容器实例
- 发送到宿主机 `5160` 端口的 UDP 包（SIP）转发到容器实例
- 发送到宿主机 `10000 - 20000` 端口的 UDP 包（语音）转发到容器实例
- 发送到宿主机 `80` 端口的 TCP 包（Web 管理服务）转发到容器实例

以上配置可根据实际需求更改。

这里使用 `iptables` 来完成，使用 `exit` 从实例退出返回宿主机，执行：

```shell
sudo iptables -t nat -A PREROUTING --dst 192.168.0.107 -p udp --dport 5060 -j DNAT --to 10.72.18.48
sudo iptables -t nat -A PREROUTING --dst 192.168.0.107 -p udp --dport 5160 -j DNAT --to 10.72.18.48
sudo iptables -t nat -A PREROUTING --dst 192.168.0.107 -p udp --dport 10000:20000 -j DNAT --to 10.72.18.48
sudo iptables -t nat -A PREROUTING --dst 192.168.0.107 -p tcp --dport 80 -j DNAT --to 10.72.18.48
```

查看 NAT 配置状态：

```shell
sudo iptables -t nat -v -L PREROUTING -n
```

{% img /2020/07/25/create-a-linux-container-of-debian-with-asterisk-freepbx-installed/iptables-nat.png '"listen-ports" "iptables-nat"' %}

现在即可通过宿主机的 IP 访问宿主机上容器化的 VoIP 服务了。

### Asterisk 网络配置

配置 Asterisk 的 SIP 参数：

- `externip` 为 `192.168.0.107`
- `localnet` 为 `10.72.18.0/24`

这样 VoIP 服务才能向 SIP 终端正确地发送 `contact` 参数，否则终端与服务器的 SIP 协议交互会出错。

直接使用 FreePBX 完成配置（Settings -> Asterisk SIP Settings）：

{% img /2020/07/25/create-a-linux-container-of-debian-with-asterisk-freepbx-installed/asterisk-sip-settings.png '"asterisk-sip-settings" "asterisk-sip-settings"' %}

## 总结

本文详细介绍了如何将 VoIP 服务容器化，涉及的内容包括 LXD 的使用、systemd-nspawn 的使用、根文件系统制作、容器镜像制作以及和 VoIP 服务相关的网络配置。通过容器化的操作能让各平台的运行环境保持一致，减少开发人员不必要的时间损耗。至于选择 LXD 作为实现容器化的平台原因有2：

1. VoIP 服务软件组成复杂，部署在一个带根文件系统的容器中更为方便；
2. LXD 的实现性能较好，具体研究请参考论文 [Performance analysis of multi services on container Docker, LXC, and LXD](http://www.beei.org/index.php/EEI/article/viewFile/1953/1596)
