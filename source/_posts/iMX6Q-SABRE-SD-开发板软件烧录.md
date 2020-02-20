---
title: iMX6Q SABRE-SD 开发板软件烧录
date: 2020-02-19 17:04:39
tags:
- iMX6Q
---

本文将介绍 `iMX6Q SABRE-SD` 开发板的软件烧录流程。

<!--more-->

## 软件环境

- 操作系统: Ubuntu14.04

## 准备工作

0. 使开发板进入工厂模式
具体操作请参考此[文档](/blog/2020/02/18/iMX-USB-loader-UTP-使用方法介绍)。

1. 开发板系统软件
制作开发板系统软件请参考此[文档](/blog/2020/02/19/iMX6Q-SABRE-SD-开发板系统软件)。

## 烧录流程

烧录过程中会使用 `utp_com` 向开发板发送指令，开发板收到指令后执行。

### 创建分区表

0. 分区脚本

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
cat <<END >/tmp/mkmmc.sh
#!/bin/sh

node=\$1
# partition size in MB
BOOT_ROM_SIZE=10
# wait for the SD/MMC device node ready
while [ ! -e \${node} ]
do
sleep 1
echo "wait for \${node} appear"
done
# destroy the partition table
dd if=/dev/zero of=\${node} bs=1024 count=1
# call sfdisk to create partition table
sfdisk --force \${node} <<EOF
1M,,L,*
EOF
END
{% endcodeblock %}

1. 发送并执行分区脚本

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "send" -f /tmp/mkmmc.sh
./utp_com -d /dev/sg1 -c "$ sh \$FILE /dev/mmcblk3"
{% endcodeblock %}

### 建立 U-Boot 分区

0. boot 分区写使能

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "$ echo 0 > /sys/block/mmcblk3boot0/force_ro"
{% endcodeblock %}

1. 发送 SPL、U-Boot 并写入

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "send" -f <SPL 文件路径>
./utp_com -d /dev/sg1 -c "$ dd if=\$FILE of=/dev/mmcblk3boot0 bs=1024 seek=1"
./utp_com -d /dev/sg1 -c "send" -f <U-Boot 文件路径>
./utp_com -d /dev/sg1 -c "$ dd if=\$FILE of=/dev/mmcblk3boot0 bs=1024 seek=69"
{% endcodeblock %}

2. boot 分区只读使能

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "$ echo 1 > /sys/block/mmcblk3boot0/force_ro"
{% endcodeblock %}

3. 使能 boot 分区

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "$ mmc bootpart enable 1 1 /dev/mmcblk3"
{% endcodeblock %}

### 创建系统分区

0. 建立 EXT4 分区

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "$ while [ ! -e /dev/mmcblk3p1 ]; do sleep 1; echo \"waiting...\"; done"
./utp_com -d /dev/sg1 -c "$ mkfs.ext4 -L rootfs /dev/mmcblk3p1"
{% endcodeblock %}

1. 挂载系统分区

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "$ mkdir -p /mnt/rootfs"
./utp_com -d /dev/sg1 -c "$ mount /dev/mmcblk3p1 /mnt/rootfs"
{% endcodeblock %}

### 安装操作系统

0. 写入根文件系统

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "pipe tar -x[文件压缩方式] -C /mnt/rootfs" -f <rootfs 文件路径>
./utp_com -d /dev/sg1 -c "frf"
./utp_com -d /dev/sg1 -c "$ chown root:root /mnt/rootfs"
./utp_com -d /dev/sg1 -c "$ chmod 755 /mnt/rootfs"
{% endcodeblock %}

1. 设置 `uname_r`

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "$ echo 'uname_r=<内核版本信息>' >> /mnt/rootfs/boot/uEnv.txt"
{% endcodeblock %}

2. 写入内核、设备树、内核模块

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "send" -f <kernel 文件路径>
./utp_com -d /dev/sg1 -c "$ cp \$FILE /mnt/rootfs/boot/vmlinuz-<内核版本信息>"
./utp_com -d /dev/sg1 -c "send" -f <dtb 文件路径>
./utp_com -d /dev/sg1 -c "$ mkdir -p /mnt/rootfs/boot/dtbs/<内核版本信息>/"
./utp_com -d /dev/sg1 -c "$ tar xf \$FILE -C /mnt/rootfs/boot/dtbs/<内核版本信息>/"
./utp_com -d /dev/sg1 -c "send" -f <modules 文件路径>
./utp_com -d /dev/sg1 -c "$ tar xf \$FILE -C /mnt/rootfs/"
{% endcodeblock %}

3. 更新 fstab

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "$ echo '/dev/mmcblk2p1  /  auto  errors=remount-ro  0  1' >> /mnt/rootfs/etc/fstab"
{% endcodeblock %}

4. 同步，取消挂载

{% codeblock netop@mfgtools:~/utp_com$ lang:bash %}
./utp_com -d /dev/sg1 -c "$ sync"
./utp_com -d /dev/sg1 -c "$ umount /mnt/rootfs"
{% endcodeblock %}

## 启动开发板

将开发板的[启动拨码开关](/blog/2020/02/18/iMX-USB-loader-UTP-使用方法介绍#软件环境)拨至 `11100110`，然后上电启动可看到串口输出:

{% codeblock 串口输出 lang:bash %}
Ubuntu 18.04.3 LTS arm ttymxc0

default username:password is [ubuntu:temppwd]

arm login: 
{% endcodeblock %}

## 总结

本文介绍了 `iMX6Q SABRE-SD` 开发板的软件烧录流程，总结为以下4个步骤:

- 创建分区表
- 建立 U-Boot 分区
- 创建系统分区
- 安装操作系统
