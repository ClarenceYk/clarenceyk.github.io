---
title: iMX USB loader & UTP 使用方法介绍
date: 2020-02-18 14:04:49
tags:
- iMX6Q
- mfgtool
- imx_usb
- utp_com
---

本文将介绍 Freescale 提供的工具软件 iMX USB loader 以及 UTP 的使用方法。

<!-- more -->

在使用 iMX6Q SabreSD 开发板时，如果我们对 Linux 内核或者根文件系统进行了定制则需要将新的软件部署到开发板。将软件部署到开发板根据开发板的启动方式不同而不同，如果我们需要开发板从 eMMC 启动则需要将软件部署到 eMMC 存储器，使用 Freescale 提供的 `iMX USB loader` 和 `UTP` 2个工具能帮助我们完成这项工作。

## 软件环境

- 操作系统: Ubuntu14.04

## 开发板设置

在操作之前需要将开发板设置到下载模式，iMX6Q SabreSD 的说明文档中提到:

> | Mode | Switch |
> | ---  |  ---   |
> | download mode(MFGTool mode) | (SW6) 00001100 (from 1-8 bit) |
> | eMMC (MMC3) boot            | (SW6) 11100110 (from 1-8 bit) |
> | MMC4 (SD2) boot             | (SW6) 10000010 (from 1-8 bit) |
> | MMC2 (SD3) boot             | (SW6) 01000010 (from 1-8 bit) |
>
> ***Freescale*** -- *Android User Guide*

所以我们将开发板的启动拨码开关拨到如下位置:

![](/blog/2020/02/18/iMX-USB-loader-UTP-使用方法介绍/imx-usb-loader-boot-switch.jpg)

将开发板的 USB 接到电脑端，使用 `lsusb` 命令可看到如下信息:

{% codeblock 查看 USB 设备信息 lang:bash %}
lsusb
# ......
# Bus 001 Device 003: ID 15a2:0054 Freescale Semiconductor, Inc. i.MX 6Dual/6Quad SystemOnChip in RecoveryMode
# ......
{% endcodeblock %}

可看到电脑端成功识别了开发板，记录下 `ID` 信息:

- ID: 15a2:0054

*在开始后面步骤之前请将开发板串口与电脑端连接，方便通过串口查看调试信息。*

## iMX USB loader 安装配置

0. 安装 `libusb`。

{% codeblock install libusb lang:bash %}
sudo apt-get install libusb-1.0-0-dev
{% endcodeblock %}

1. 获取 iMX USB loader [源代码](https://github.com/boundarydevices/imx_usb_loader)。

{% codeblock git clone lang:bash %}
git clone https://github.com/boundarydevices/imx_usb_loader.git
{% endcodeblock %}

2. 编译

{% codeblock compile lang:bash %}
cd imx_usb_loader
make
{% endcodeblock %}

3. 修改配置文件

查看文件 `imx_usb.conf` 根据刚刚记录下来的 `ID` 值找到取对应的开发板配置文件:

{% codeblock imx_usb.conf %}
......
0x15a2:0x0054, mx6_usb_work.conf
......
{% endcodeblock %}

将文件 `mx6_usb_work.conf` 的内容修改为如下:

{% codeblock mx6_usb_work.conf %}
mx6_qsb
hid,1024,0x910000,0x10000000,1G,0x00900000,0x40000

firmware/u-boot.imx: dcd
firmware/zImage: load 0x12000000
firmware/fsl-image-mfgtool-initramfs-imx6qsabresd.cpio.gz.u-boot: load 0x12C00000
firmware/zImage-imx6q-sabresd.dtb: load 0x18000000
firmware/u-boot.imx: clear_dcd,load,plug,jump header
{% endcodeblock %}

4. 使用 USB loader

创建 `firmware` 目录:

{% codeblock 创建目录 lang:bash %}
mkdir firmware 
{% endcodeblock %}

将[生成的镜像文件](https://clarenceyk.github.io/blog/2020/02/17/%E6%90%AD%E5%BB%BA-iMX6Q-SabreSD-Yocto-%E9%A1%B9%E7%9B%AE%E5%BC%80%E5%8F%91%E7%8E%AF%E5%A2%83/)拷贝到 `firmware` 目录中后执行:

{% codeblock 运行 imx_usb lang:bash %}
./imx_usb 
{% endcodeblock %}

等待半分钟左右，可通过串口看到输出的启动信息:

{% codeblock 串口输出信息 lang:bash %}
......
Starting UTP
uuc 0.5 [built Jan  9 2020 12:41:11]
UTP: Waiting for device to appear
UTP: file/device node /dev/utp already exists
......
{% endcodeblock %}

此时开发板通过 USB 将自己模拟成一个 `sg` 设备，在电脑端可以通过以下命令查看此设备:

{% codeblock 查看 sg 设备 lang:bash %}
ls /dev/sg*
# /dev/sg0  /dev/sg1
{% endcodeblock %}

其中 `/dev/sg1` 就是开发板。通过此设备我们便可用 `UTP` 工具与开发板通信。

## UTP 安装配置

0. 安装 `libsgutils2`。

{% codeblock install libsgutils2 lang:bash %}
sudo apt-get install libsgutils2-dev
{% endcodeblock %}

1. 获取 UTP [源代码](https://github.com/ixonos/utp_com)。

{% codeblock git clone lang:bash %}
git clone https://github.com/ixonos/utp_com.git
{% endcodeblock %}

2. 编译

{% codeblock compile lang:bash %}
cd utp_com
make
{% endcodeblock %}

3. 使用 utp_com 与开发板通信

{% codeblock 使用 utp_com lang:bash %}
./utp_com -d /dev/sg1 -c "$ echo hello, world!"
{% endcodeblock %}

通过串口可看到开发板的输出信息:

{% codeblock 串口输出 lang:bash %}
UTP: received command '$ echo hello, world!'
UTP: executing "echo hello, world!"
hello, world!
UTP: sending Success to kernel for command $ echo hello, world!.
utp_poll: pass returned.
{% endcodeblock %}

## 总结

至此我们完成了 `imx_usb` 和 `utp_com` 的安装配置，使用 imx_usb 向开发板下载固件，以及使用 utp_com 与开发板通信向其发送并执行指令。具体工作如下:

- 开发板下载模式配置
- iMX USB loader 安装配置
- UTP com 安装配置
