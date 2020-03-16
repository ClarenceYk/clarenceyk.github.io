---
title: VirtualBox 安装及配置 Ubuntu14.04 服务器版
date: 2020-02-15 16:02:18
updated: 2020-02-19 10:19:30
tags:
- iMX6Q
- 开发环境
---

本文将以安装 Ubuntu14.04 操作系统到虚拟机中为例来介绍 VirtualBox 的使用。

<!-- more -->

## 运行环境

- 操作系统: Windows 10
- VirtualBox: 6.0

## 下载及安装 VirtualBox

本文创建时使用的 VirtualBox 版本为 `6.0`，你可根据自己的需要选择其他或者最新版本。

0. 访问 Oracle VirtualBox [主页](https://www.virtualbox.org/)。

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualbox.png)

1. 点击`下载`按钮，下载安装程序后双击此程序安装 VirtualBox。

*此处可一路点击`下一步`直至安装完成，然后重启计算机。*

## 下载 Ubuntu14.04 服务器版镜像文件

0. 访问 Ubuntu 镜像[发布页](http://releases.ubuntu.com/)，点击如下图[链接](http://releases.ubuntu.com/trusty/)。

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/ubuntu-trusty.jpg)

1. 在如下页面中找到并下载***服务器版本***镜像。

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/ubuntu-trusty-image.jpg)

## 创建虚拟机

接下来开始创建虚拟机。

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualbox-new.jpg)

0. 点击新建，选项配置如下：

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-config.jpg)

- 名称: 自定义，如 `ubuntu1404-mfgtools` 表示此虚拟机操作系统为 `ubuntu14.04` 用于制作 `mfgtools`
- 文件夹: 自定义，此虚拟机相关文件存放位置
- 类型: Linux
- 版本: Ubuntu (64-bit)
- 内存大小: 不影响主机性能的情况下取最大

其他选项如上图，配置完成后点击`创建`。

1. 创建虚拟磁盘:

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-disk.jpg)

- 文件大小: 根据需求配置
- 固定或动态大小: 同上

配置完成点击`创建`。

## 设置启动镜像

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-settings.jpg)

0. 点击`设置`，选择`存储`：

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-settings-storage.jpg)

1. 依次点击如下所示选项：

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-settings-storage-sel-image.jpg)

然后在弹出的选项中点击`选择一个虚拟光盘文件`，然后在弹出的选项框中选择之前下载的`系统镜像`文件。

2. 选择`系统`，将`启动顺序`改为如下图般:

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-settings-system-boot.jpg)

配置完成后点击`确认`。

## 安装系统

0. 主界面点击`启动`，进入系统安装界面。

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-install.jpg)

1. 根据提示操作，完成安装后如下图：

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-install-complete.jpg)

选择 `Continue` 结束安装。关闭窗口。

2. 回到主界面点击`设置`，选择`系统`并将`启动顺序`改为如下图般:

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-settings-system-boot-1.jpg)

点击 `ok` 保存设置。

## 额外的配置

完成前面的步骤后一个可用的虚拟机已经创建完成，后面的步骤可以让虚拟机使用起来更方便。

0. 设置端口转发

回到主界面点击`设置`，选择`网络`并点击`高级`选项卡:

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-settings-network.jpg)

点击`端口转发`之后，添加如下配置:

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-settings-port.jpg)

其中`子系统IP`填为虚拟机的 `IP` 地址。

*这一步的作用是将虚拟机的 `22` 端口转发到主机的 `2200` 端口上，方面我们使用 `SSH` 连接虚拟机。*

1. 安装增强功能

{% codeblock 登录虚拟机 lang:bash %}
# ssh -p <端口> <用户>@<主机地址>
ssh -p 2200 test@192.168.1.101
{% endcodeblock %}

{% codeblock 安装依赖软件 lang:bash %}
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential
{% endcodeblock %}

点击如下图所示的`安装增强功能`:

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-install-enhance.jpg)

{% codeblock 挂载光盘 lang:bash %}
sudo mkdir /mnt/cdrom
sudo mount /dev/cdrom /mnt/cdrom/
# mount: block device /dev/sr0 is write-protected, mounting read-only
{% endcodeblock %}

{% codeblock 运行安装程序 lang:bash %}
cd /mnt/cdrom/
sudo ./VBoxLinuxAdditions.run
sudo shutdown -h 0 -r
{% endcodeblock %}

2. 共享文件夹

{% codeblock 加入 vboxsf 组 lang:bash %}
# sudo usermod -a -G vboxsf <用户>
sudo usermod -a -G vboxsf test
{% endcodeblock %}

回到`设置`，选择`共享文件夹`并点击如下所示按钮:

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-new-share.jpg)

如下般设置:

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-new-share-settings.jpg)

*挂载点不用填。*

{% codeblock 重启虚拟机 lang:bash %}
sudo shutdown -h 0 -r
{% endcodeblock %}

3. USB 设备支持

访问 VirtualBox 官网下载[拓展包](https://www.virtualbox.org/wiki/Download_Old_Builds_6_0)。*注意，请找到对应自己版本的拓展包。*

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-extension-pack.jpg)

回到 VirtualBox 主界面，选择`管理`->`全局设定`->`拓展`:

![](/blog/2020/02/15/VirtualBox-安装及配置-Ubuntu14-04-服务器版/virtualmachine-extension-pack-sel.jpg)

选择刚刚下载的拓展包，安装。

## 总结

以上我们就在虚拟机中安装好了 Ubuntu 操作系统，并做了以下设置:

- 将虚拟机的`22`端口转发到主机`2200`端口。
- 创建一个共享文件夹用于虚拟机与主机间的文件共享。
- 安装了拓展包使虚拟机支持 USB2.0 以及 USB3.0 设备。
