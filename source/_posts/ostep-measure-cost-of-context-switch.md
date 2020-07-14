---
title: 【OSTEP 练习题】测量操作系统切换上下文的耗时
date: 2020-07-11 16:49:16
updated: 2020-07-12 23:00:06
tags:
- Linux
- Operating System
- Context Switch
---

{% img /2020/07/11/ostep-measure-cost-of-context-switch/measure_cost_of_context_switch.svg '"measure_ctx_switch" "measure_ctx_switch"' %}

尝试解答 `OSTEP` <sup>[1](#注释)</sup> 第6章 “Limited Direct Execution” 的练习题 —— 测量操作系统切换上下文的耗时。

<!--more-->

## 环境

使用的测试平台 CPU 是 Intel i5-6200U，主频 2.3GHz，操作系统是 Arch Linux，内核版本 5.7.7。

## 方法

为了尽量准确地测量进程上下文切换耗时，需要有如下几个前提：

1. 创建 2 个测试进程；
2. 保证这 2 个进程运行在同一个 CPU 核上（鉴于目前的计算平台一般为多核 CPU）；
3. 保证正在运行这 2 个测试进程的 CPU 核上没有运行其他用户进程；
4. 尽量使操作系统在指定时刻执行进程调度（与此同时执行上下文切换）。

### 进程创建

使用 `fork` 系统调用创建子进程：

```c
switch (fork()) {
case -1: // error handling
    exit(EXIT_FAILURE);
case 0: // child process
    // do something in child process
    exit(EXIT_SUCESS);
default: // parent process
    // do something in parent process
    exit(EXIT_SUCESS);
}
```

### sched_setaffinity

使用 `sched_setaffinity` 指定进程运行在 `CPU3` 这个核心上：

```c
#define CPU_NUM 3

cpu_set_t set;

CPU_ZERO(&set);
CPU_SET(CPU_NUM, &set);
if (sched_setaffinity(getpid(), sizeof(set), &set) < 0) {
    // error handling
}
// set successfully
// do other things
```

此操作只需在父进程 fork 子进程之前执行即可，子进程默认情况下会和父进程运行在同一个 CPU 核上。

### 内核启动参数

为了保证指定的 CPU 核上只有用于测试的 2 个用户进程，需要设置 `isolcpus` 这个启动参数给内核。此参数告诉内核在调度其余用户进程时排除指定的 CPU 核心。

```bash
cat /proc/cmdline
# BOOT_IMAGE=... isolcpus=3 ...
```

重新启动计算机，在进入 GRUB 启动界面时选择启动项并按下 `e`，进入启动项编辑界面，找到 `linux` 为开头的一行在行尾添加如下启动参数：

```bash
ioslcpus=3 # 3 代表测试平台中 CPU 的一个核的编号
```

查看设备 CPU 核心数可在进入系统后命令行执行：

```shell
lscpu
```

或者：

```shell
cat /proc/cpuinfo
```

设置完后可以用 `stress` 命令测试一下：

```shell
sudo pacman -S stress
stress --cpu 8
```

从下图可以看出内核在调度用户进程时绕开了 `CPU3`（从 0 开始计数）。

{% img /2020/07/11/ostep-measure-cost-of-context-switch/stress_cpu.png '"stress_cpu" "stress_cpu"' %}

### Pipe

以上几个步骤保证了在 `CPU3` 核心上只有用于测试的 2 个用户进程，接下来只需要让内核来回切换（调度）这 2 个测试进程即可。

使用的方法是：创建 2 个 `pipe`，子进程向 `pipe0` 写入一个字符然后从 `pipe1` 读取一个字符，父进程从 `pipe0` 读取一个字符然后将读取到的字符写入 `pipe1`，这样重复 N/2 次。当子进程在“等待”读取时内核就执行调度切换到父进程，当父进程在“等待”读取时内核执行调度切换到子进程，所以测试进程一次“读写”完成后内核执行了 2 次上下文切换，既总共完成了 N 次切换。更加直观的过程如[题图](#top)所示。

## 结果

在我的平台<sup>[2](#注释)</sup> 上的测试结果为：操作系统上下文切换平均耗时 3.42 微秒。

## 源码

{% gist 75ddfa9545480b9a44d329a5bf8c22f1 measure_cost_of_ctx.c %}

## 注释

<sub>[1] Operating System: Three Easy Pieces - ARPACI-DUSSEAU</sub>
<sub>[2] Intel i5-6200U @ 2.3GHz, Arch Linux, Kernel 5.7.7</sub>
