---
title: 搭建 IPPBX 基础开发环境
tags:
  - Linux
  - systemd-nspawn
  - rsync
date: 2020-05-06 13:58:46
updated: 2020-05-08 17:42:23
---


软件开发中应对不同的需求有不同的解决方案，进而产生不同的软件项目。针对不同的软件项目需搭建特定的开发环境以适应各个项目的开发需要。本文以在 Linux 环境中搭建 IPPBX 开发环境为例来介绍如何搭建一个符合需求的开发环境。

<!--more-->

## 需求

整个开发环境里有如下设备：

1. 一台开发机，包含2块硬盘
2. N 台目标设备（最终业务运行在这些设备上）

实际的开发场景应该是：开发机随时运行着，其运行状态为：2块硬盘各有一个 Linux 操作系统，其中一个为主系统另一个以类似容器的形式运行在主系统中。主系统中只安装各类开发工具，不运行任何与项目业务相关的服务，所有和项目业务相关的服务都运行在“容器<sup>[1](#注释)</sup>”中。开发人员通过 SSH 远程登陆到主系统中完成开发，或者本地开发完成后将软件服务同步到主系统，维护人员再将新的内容同步到“容器”中。

实际的系统部署场景应该是：当需要将系统部署到新设备上时，维护人员只需将开发机上2块硬盘中存放“容器”的那块硬盘中的数据“拷贝<sup>[2](#注释)</sup>”到新设备的硬盘中即可。

## 目标

在着手搭建环境之前需要了解清楚开发环境搭建针对的目标，本例中的目标很简单只有如下2个：

1. 便于开发工作的进行。
2. 能方便地将完成了开发工作的系统部署到新设备上。

## 拆分需求

结合前面`需求`和`目标`可得到如下细分的功能：

1. 开发机上需要在其中一块硬盘中安装并运行一个 Linux 操作系统。
2. 开发机上的另一块硬盘安装一个 Linux 操作系统，此系统能脱离主系统独立运行，同时又能作为“容器”运行在主系统之中。
3. 需要一个同步方法，能将“容器”同步到新设备的硬盘中。

## 选择操作系统及软件工具

对于开发机的主操作系统这部分没有严格的要求，选择一个适合自己使用习惯的较新的 Linux 发行版即可，这里作为演示我选择的是 `Ubuntu20.04`。

容器中运行的操作系统，同时也是最终设备中运行的操作系统，这部分对系统的稳定性以及很多软件的兼容性要求更高一些，所以我们选择 `Debian10`。

对于“容器化”的部分，目前有很多容器化技术，如功能非常少但使用非常简单的 `chroot`（甚至不能作为容器技术🙈）；又如功能非常强大但使用起来有一定门槛的 `docker`、`podman` 或者 Ubuntu 推出的 `LXC` 等；还有介于前面两种之间的 `systemd-nspawn`。鉴于本例中的使用场景，我们选择 `systemd-nspawn`，因为其不仅能在我们的[需求](#需求)范围内很好地隔离主系统与“容器”而且使用方法相对简单。

对于“同步”这部分，我们选择 `rsync`，这是一个基于增量传输的文件同步软件，既可用通过单机本地同步也可通过网络同步，能满足我们开发环境中的同步需求。

## 安装

### 主系统

主操作系统我们选用的是 `Ubuntu20.04` 安装方法网络上很多，搜索一下即可，这里不再介绍。

### 容器中的系统

本例中我们采用的是 `Debian10`，安装方法同上。值得注意的是，此系统需安装到开发机的另一块硬盘上，同时在安装前可先取下安装了主系统的硬盘，这样主系统的启动条目就不会出现在“容器”的 grub 启动选项中。此外，分区方式推荐如下：

```plaintext
sdb      8:16   0 232.9G  0 disk
├─sdb1   8:17   0    16G  0 part [SWAP]
└─sdb2   8:18   0 216.8G  0 part
```

其中交换分区根据自己的实际需求划分，剩下的空间全部作为 `/` 分区。

### 工具软件

在主系统中安装软件包 `systemd-contianer`（此中软件包中包含 `systemd-nspawn` 工具）以及安装同步工具 `rsync`。

```shell
sudo apt update
sudo apt install systemd-contianer
sudo apt install rsync
```

*注：关于使用 `rsync` 来同步容器到新设备的方法放在[另一篇文章](/blog/2020/05/08/复刻-Linux-操作系统到另一台设备)中阐述。*

## 运行“容器”

将2块装好了操作系统的硬盘都接到主板上，然后启动开发机进入 `BIOS` 将装有 Ubuntu20.04 操作系统的硬盘设置到启动序列的第一位。进入主系统后，查看硬盘分区，大致如下：

```shell
$ lsblk
sda      8:0    0 931.5G  0 disk
├─sda1   8:1    0  15.3G  0 part [SWAP]
├─sda2   8:2    0 122.1G  0 part /
└─sda3   8:3    0 794.2G  0 part /home
sdb      8:16   0 232.9G  0 disk
├─sdb1   8:17   0    16G  0 part
└─sdb2   8:18   0 216.8G  0 part
```

其中 `sda` 中安装了我们的主系统（Ubuntu）也就是当前运行的系统，`sdb` 中安装了“容器”的系统（Debian）。

- 创建挂载点

```shell
sudo mkdir /mnt/debian_10
```

- 挂载 `sdb` 中的文件系统

```shell
sudo mount /dev/sdb2 /mnt/debian_10
```

- 使用 `systemd-nspawn` 进入“容器”环境

```shell
$ sudo systemd-nspawn -D /mnt/debian_10 -b
pawning container debian10 on /mntdebian_10.
Press ^] three times within 1s to kill container.

...

Welcome to Debian GNU/Linux 10 (buster)!

...

Debian GNU/Linux 10 debian console

debian login:
```

到此我们就进入了“容器”环境。在容器内部我们可以认为在另一个操作系统里，于是可以执行任何命令以及安装各种软件但是并不影响主系统。例如我们可以[安装 Asterisk](https://gist.github.com/ClarenceYk/2995d607e1b7678fe0c37665546217aa#file-install_asterisk-sh) 与 [FreePBX](https://gist.github.com/ClarenceYk/2995d607e1b7678fe0c37665546217aa#file-install_freepbx-sh)

- 退出容器

```shell
sudo shutdown now
```

或者在 1s 内按下 `^]` 3次。（`control 键` + `] 键`）

## 容器中安装平台环境

### SSH

```shell
sudo apt update
sudo apt install openssh-server
sudo systemctl start ssh
sudo systemctl enable ssh
```

### Asterisk

参考 Asterisk 自动安装[脚本](https://gist.github.com/ClarenceYk/2995d607e1b7678fe0c37665546217aa#file-install_asterisk-sh)。

*注意：此脚本仅在 `Ubuntu18.04` 以及 `Debian10` 中测试通过。*

### FreePBX

参考 FreePBX 自动安装[脚本](https://gist.github.com/ClarenceYk/2995d607e1b7678fe0c37665546217aa#file-install_freepbx-sh)。

*注意：此脚本仅在 `Ubuntu18.04` 以及 `Debian10` 中测试通过。*

## 总结

本文介绍了 `搭建 IPPBX 基础开发环境` 中的操作系统、工具软件选择以及容器的制作与运行。下一篇将接着介绍如何将容器`同步`到新设备上。

## 注释

<sub>[1] 这里的容器不是指完整的容器技术，仅仅采用了 Linux 容器的概念。</sub>

<sub>[2] 只借用“拷贝”的概念，实际操作要更复杂一些。</sub>
