---
title: iMX6Q SABRE-SD 开发板系统软件
date: 2020-02-19 14:34:46
tags:
- iMX6Q
- U-Boot
- Linux kernel
---

本文将介绍适用于 `iMX6Q SABRE-SD` 开发板的启动加载器（U-Boot）、Linux 内核以及根文件系统的制作。

<!--more-->

## 软件环境

- 操作系统: Ubuntu14.04

## ARM 交叉编译工具链

0. 下载 & 解压

{% codeblock netop@mfgtools:~$ lang:bash %}
wget -c https://releases.linaro.org/components/toolchain/binaries/6.5-2018.12/arm-linux-gnueabihf/gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf.tar.xz
tar xf gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf.tar.xz
{% endcodeblock %}

1. 测试

{% codeblock netop@mfgtools:~$ lang:bash %}
export CC=`pwd`/gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
${CC}gcc --version
{% endcodeblock %}

{% codeblock 终端输出 lang:bash %}
arm-linux-gnueabihf-gcc (Linaro GCC 6.5-2018.12) 6.5.0
Copyright (C) 2017 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
{% endcodeblock %}

## 启动加载器: U-Boot

0. 下载源码

{% codeblock netop@mfgtools:~$ lang:bash %}
git clone https://github.com/u-boot/u-boot
cd u-boot/
git checkout v2019.07 -b tmp
{% endcodeblock %}

1. 打补丁

{% codeblock netop@mfgtools:~/u-boot$ lang:bash %}
wget -c https://raw.githubusercontent.com/eewiki/u-boot-patches/master/v2019.07-rc4/0001-mx6sabresd-fixes.patch
patch -p1 < 0001-mx6sabresd-fixes.patch
{% endcodeblock %}

2. 修改代码

用编辑器打开文件 `~/u-boot/include/configs/mx6sabre_common.h`，找到如下代码:

{% codeblock vi ~/u-boot/include/configs/mx6sabre_common.h lang:c %}
......
#define CONFIG_BOOTCOMMAND \
	"setenv interface mmc;" \
	"setenv mmcdev 0;" \
	"run mmcboot;" \
......
{% endcodeblock %}

添加2行代码，结果如下:

{% codeblock vi ~/u-boot/include/configs/mx6sabre_common.h lang:c %}
......
#define CONFIG_BOOTCOMMAND \
	"setenv interface mmc;" \
	"setenv mmcdev 3;" \
	"run mmcboot;" \
	"setenv mmcdev 0;" \
	"run mmcboot;" \
......
{% endcodeblock %}

3. 配置 & 编译

{% codeblock netop@mfgtools:~/u-boot$ lang:bash %}
make ARCH=arm CROSS_COMPILE=${CC} distclean
make ARCH=arm CROSS_COMPILE=${CC} mx6sabresd_defconfig
make ARCH=arm CROSS_COMPILE=${CC}
{% endcodeblock %}

## Linux 内核

内核我们采用 `4.19` 长期支持版。

0. 下载源码

{% codeblock netop@mfgtools:~$ lang:bash %}
git clone https://github.com/RobertCNelson/armv7-multiplatform
cd armv7-multiplatform/
git checkout origin/v4.19.x -b tmp
{% endcodeblock %}

1. 编译

{% codeblock netop@mfgtools:~/armv7-multiplatform$ lang:bash %}
./build_kernel.sh
{% endcodeblock %}

## 根文件系统

根文件系统我们采用 `Ubuntu18.04 LTS`。

0. 下载

{% codeblock netop@mfgtools:~$ lang:bash %}
wget -c https://rcn-ee.com/rootfs/eewiki/minfs/ubuntu-18.04.3-minimal-armhf-2020-02-10.tar.xz
{% endcodeblock %}

1. 验证

{% codeblock netop@mfgtools:~$ lang:bash %}
sha256sum ubuntu-18.04.3-minimal-armhf-2020-02-10.tar.xz
# 输出信息: b28b356d75153bfb3beb5c96bf8eabe92025cf5e665e1a564b469bc70e5a363b  ubuntu-18.04.3-minimal-armhf-2020-02-10.tar.xz
{% endcodeblock %}

2. 解压

{% codeblock netop@mfgtools:~$ lang:bash %}
tar xf ubuntu-18.04.3-minimal-armhf-2020-02-10.tar.xz
{% endcodeblock %}

## 总结

本文介绍了 `iMX6Q SABRE-SD` 开发板系统软件的制作。通过这些操作可得到以下文件:

0. 启动加载器, `目录: ~/u-boot/`

- SPL
- u-boot.img

1. 内核相关，`目录: ~/armv7-multiplatform/deploy`

- 4.19.xx-armv7-xxx.zImage
- 4.19.xx-armv7-xxx-dtbs.tar.gz
- 4.19.xx-armv7-xxx-modules.tar.gz

2. 根文件系统，`目录: ~/imx6q/ubuntu-18.04.3-minimal-armhf-2020-02-10`

- armhf-rootfs-ubuntu-bionic.tar
