---
title: 搭建 iMX6Q SabreSD Yocto 项目开发环境
date: 2020-02-17 11:03:41
updated: 2020-02-18 13:50:17
tags:
- iMX6Q
- yocto
- mfgtool
---

本文将介绍 [iMX6Q SabreSD]() 开发板的 [Yocto Project]() 开发环境搭建，以及使用 Yocto 编译此开发板的烧录工具（MFGTool）镜像。

<!-- more -->

## 软件环境

- 操作系统: Ubuntu14.04

## 安装依赖包

{% codeblock Yocto Project 的依赖包 lang:bash %}
sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
build-essential chrpath socat
{% endcodeblock %}

{% codeblock 其他依赖包 lang:bash %}
sudo apt-get install libsdl1.2-dev xterm sed cvs subversion coreutils texi2html \
docbook-utils python-pysqlite2 help2man make gcc g++ desktop-file-utils \
libgl1-mesa-dev libglu1-mesa-dev mercurial autoconf automake groff curl lzop asciidoc
{% endcodeblock %}

{% codeblock u-boot 工具 lang:bash %}
sudo apt-get install u-boot-tools
{% endcodeblock %}

## 安装 `repo` 工具

`repo` 是一个基于 `git` 的工具。使用 repo 可以方便地管理存在多个软件源的项目。安装 repo 分为以下几个步骤:

0. 在 `home` 目录下创建一个 `bin` 目录。

{% codeblock 安装 repo lang:bash %}
mkdir ~/bin
curl http://commondatastorage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
chmod a+x ~/bin/repo
{% endcodeblock %}

1. 将下面2行加入 `.bashrc` 文件末尾，确保 `~/bin` 在 `PATH` 变量中。

{% codeblock 环境变量 lang:bash %}
export PATH=${PATH}:~/bin
export MACHINE=imx6qsabresd
{% endcodeblock %}

## 构建 Yocto 项目

`Freescale Yocto Project BSP Release` 目录包含如下内容:

- `sources` 目录包含一些用于构建的配方
- 一个或多个 `build` 目录
- 一些用于配置开发环境的脚本

以下步骤创建了一个 `fsl-release-bsp` 目录用于构建 Yocto 项目:

{% codeblock 创建目录 lang:bash %}
mkdir fsl-release-bsp
cd fsl-release-bsp
{% endcodeblock %}

{% codeblock 配置 git 信息 lang:bash %}
# git config --global user.name "Your Name"
git config --global user.name "ClarenceYk"
# git config --global user.email "Your Email"
git config --global user.email "xxx@xxx.com"
git config --list
{% endcodeblock %}

{% codeblock 同步源代码 lang:bash %}
repo init -u git://git.freescale.com/imx/fsl-arm-yocto-bsp.git -b imx-4.1.15-1.0.0_ga
repo sync
{% endcodeblock %}

上面采用了 `imx-4.1.15-1.0.0_ga` 分支，可根据实际需求使用[其他分支代码](http://git.freescale.com/git/cgit.cgi/imx/fsl-arm-yocto-bsp.git/)，如下:

![](/blog/2020/02/17/搭建-iMX6Q-SabreSD-Yocto-项目开发环境/fsl-arm-yocto-bsp-git.jpg)

## 编译构建 MFGTool

向开发板烧录系统镜像可使用 MFGTool 完成。编译 MFGTool 镜像文件的配方分别是 `linux-imx-mfgtool` 和 `u-boot-mfgtool`。编译命令如下:

{% codeblock 编译 MFGTool lang:bash %}
cd ~/fsl-release-bsp
source fsl-setup-release.sh -b build-mfgtools -e x11
bitbake fsl-image-mfgtool-initramfs
{% endcodeblock %}

*编译过程需下载大量的源代码，为确保过程顺利建议使用 VPN 或者采用其他加速网络访问的方式。*

成功编译之后，在目录 `~/fsl-mfgtools-bsp/build-mfgtools/tmp/deploy/images/imx6qsabresd` 下会产生如下文件:

- u-boot.imx（u-boot）
- zImage（内核）
- zImage-imx6q-sabresd.dtb（设备树）
- fsl-image-mfgtool-initramfs-imx6qsabresd.cpio.gz.u-boot（内存文件系统）

## 总结

通过以上步骤，我们完成了 `Freescale iMX6Q Yocto Project` 开发环境搭建，其中包含如下方面的工作:

- 安装项目依赖软件
- 安装 repo
- 配置 git
- 构建 Yocto 项目
- 编译 MFGTool 镜像
