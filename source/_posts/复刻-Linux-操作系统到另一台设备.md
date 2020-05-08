---
title: 复刻 Linux 操作系统到另一台设备
date: 2020-05-08 14:32:27
updated: 2020-05-08 14:32:27
tags:
  - Linux
  - rsync
  - clone file system
---

[rsync](https://rsync.samba.org/) 是一款快速且灵活的文件拷贝工具，既能实现本地拷贝也能完成远程同步。本文将使用 `rsync` 作为核心工具来介绍如何复刻 Linux 操作系统。

<!--more-->

## 准备工作

需要注意的是源设备和目标设备之间的 CPU 架构需**保持一致**。

### 硬件

1. 一台被复刻的设备作为源设备（参考[搭建 IPPBX 基础开发环境](/blog/2020/05/06/搭建-IPPBX-基础开发环境)）
2. 一台新设备作为目标设备
3. 一个 Linux USB 启动盘（制作方法网络上很多这里不多作介绍，我制作的是 `Debian10` USB 启动盘）

### 软件

1. rsync
2. openssh-server（可选）

## 配置源设备

在[搭建 IPPBX 基础开发环境](/blog/2020/05/06/搭建-IPPBX-基础开发环境)一文中我们已经搭建好了一个开发环境，其中的 `/dev/sdb2` 设备里包含了一个完整的用于生产环境的操作系统。现在我们将 `/dev/sdb2` 用作拷贝源。

### 源设备网络

简单的方法是将源设备接到路由器上，由路由器自动分配 IP 地址。也可以采用和目标设备直接连接的方式，则需要自己配置静态 IP 地址。

### 挂载 `/dev/sdb2`

```bash
sudo mkdir -p /mnt/debian_10
sudo mount /dev/sdb2
```

### 安装&配置 rsync

```bash
$ sudo -i
enter your passwd:

apt install rsync
cat << EOF > /etc/rsyncd.conf
uid = root
gid = root
use chroot = no

[all]
    path = /
EOF
rsync --daemon
exit
```

## 配置目标设备

将目标设备从 USB 启动盘启动，然后将[必备的软件](#软件)安装好。

### 目标设备网络

简单的方法是将目标设备接到路由器上，由路由器自动分配 IP 地址。也可以采用和源设备直接连接的方式，则需要自己配置静态 IP 地址。

## 使用 rsync 复刻系统

使用 `rsync` 将源设备上 Linux 系统复刻到目标设备上的方法有两种：

1. 使用[脚本](https://gist.github.com/ClarenceYk/d79d486097350a48ff653ccd028064c8)操作
2. 手动操作

推荐使用脚本，用 root 用户执行脚本之后根据提示信息操作即可。如果需要手动操作，则接着往下看（后面的操作均使用 root 用户执行）。

### 对目标硬盘分区

首先查看一下分区信息：

```bash
$ lsblk
NAME   MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
sda      8:0    0 119.2G  0 disk
├─sda1   8:1    0     4G  0 part
└─sda2   8:2    0 115.2G  0 part
sdb      8:16   1  14.3G  0 disk
└─sdb1   8:17   1  14.3G  0 part /
```

其中 `sdb` 是 USB 启动盘 `sda` 是目标设备上的硬盘。对 `sda` 分区：

```bash
dd if=/dev/zero of=/dev/sda bs=1024 count=1
sfdisk --force /dev/sda << EOF
1M,4G,S,
,,L,*
EOF
mkswap /dev/sda1
mkfs.ext4 /dev/sda2
```

这里我们将 `sda` 划了2个分区，第一个是4G的交换分区，剩余的空间格式化为 ext4 文件系统用作系统分区（分区方式需和源设备保持一致）。

### 从源设备同步文件

首先将刚刚创建好的系统分区挂载到本地目录：

```bash
mkdir -p /mnt/rsync_tmp
mount /dev/sda2 /mnt/rsync_tmp
```

检查一下与源设备的连通性（假定源设备的 IP 地址是 `192.168.198.140`）：

```bash
ping -c 1 192.168.198.140
```

开始同步：

```bash
rsync -avHX 192.168.198.140::all/mnt/debian_10/ /mnt/rsync_tmp/
```

注意：路径结尾处的 `/` 必须有。

### 重新安装 GRUB 启动器

从源设备同步完成之后，目标设备上的 GRUB 启动器以及其配置都未更新，所以需要更新。

首先获取2个分区的 `UUID`：

```bash
$ blkid /dev/sda1
/dev/sda1: UUID="XXXX" TYPE="swap" PARTUUID="XXX"
$ blkid /dev/sda2
/dev/sda2: UUID="XXXX" TYPE="ext4" PARTUUID="XXX"
```

将对应分区的 `UUID` 更新到 `/mnt/rsync_tmp/etc/fstab` 文件中后执行：

```bash
mount --bind /proc /mnt/rsync_tmp/proc
mount --bind /sys /mnt/rsync_tmp/sys
mount --bind /dev /mnt/rsync_tmp/dev
mount --bind /run /mnt/rsync_tmp/run
chroot /mnt/rsync_tmp
```

进入到 `chroot` 环境后，更新 GRUB：

```bash
grub-install /dev/sda
update-grub
```

### 重启设备

```bash
reboot
```

进入 BIOS 中将启动设备设置为硬盘后启动，即可进入完成复刻的 Linux 系统。
