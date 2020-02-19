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
具体操作请参考此[文档](https://clarenceyk.github.io/blog/2020/02/18/iMX-USB-loader-UTP-%E4%BD%BF%E7%94%A8%E6%96%B9%E6%B3%95%E4%BB%8B%E7%BB%8D/)。

1. 开发板系统软件
制作开发板系统软件请参考此[文档](https://clarenceyk.github.io/blog/2020/02/19/iMX6Q-SABRE-SD-%E5%BC%80%E5%8F%91%E6%9D%BF%E7%B3%BB%E7%BB%9F%E8%BD%AF%E4%BB%B6/)。

## 烧录流程

烧录过程中会使用 `utp_com` 向开发板发送指令，开发板收到指令后执行。

### 创建分区表
