---
title: Linux 系统中 Watchdog 的应用
date: 2020-02-22 10:08:56
tags:
- Linux
- Watchdog
- Embedded System
---

本文将介绍如何在 Linux 系统的用户层使用 Watchdog。

<!--more-->

## 操作环境

- 硬件环境: iMX6Q SABRE-SD 开发板
- 操作系统内核: Linux 4.19.72-armv7
- 操作系统发行版: Ubuntu18.04

## 开启 Watchdog

在[编译内核](/blog/2020/02/19/iMX6Q-SABRE-SD-开发板系统软件#Linux-内核)时开启 `Watchdog`:

{% codeblock ./tools/rebuild.sh lang:bash %}
# 内核配置选项位置
Device Drivers -> Watchdog Timer Support
{% endcodeblock %}

![](/blog/2020/02/22/Linux-系统中-Watchdog-的应用/compile-kernel-watchdog.png)

*开启 `Disable watchdog shutdown on close` 这一项。*

内核文档中关于此选项的解释如下:

{% blockquote Christer Weingel https://www.kernel.org/doc/Documentation/watchdog/watchdog-api.txt The Linux Watchdog driver API. %}
When the device is closed, the watchdog is disabled, unless the "MagicClose" feature is supported (see below).  This is not always such a good idea, since if there is a bug in the watchdog daemon and it crashes the system will not reboot.  Because of this, some of the drivers support the configuration option "Disable watchdog shutdown on close", CONFIG_WATCHDOG_NOWAYOUT.  If it is set to Y when compiling the kernel, there is no way of disabling the watchdog once it has been started.  So, if the watchdog daemon crashes, the system will reboot after the timeout has passed. Watchdog devices also usually support the nowayout module parameter so that this option can be controlled at runtime.

Magic Close feature:

If a driver supports "Magic Close", the driver will not disable the watchdog unless a specific magic character 'V' has been sent to /dev/watchdog just before closing the file.  If the userspace daemon closes the file without sending this special character, the driver will assume that the daemon (and userspace in general) died, and will stop pinging the watchdog without disabling it first.  This will then cause a reboot if the watchdog is not re-opened in sufficient time.
{% endblockquote %}

简单来说，开启此选项之后一旦我们打开了 Watchdog 设备，只要程序没对此 Watchdog 做文档中指定的操作，那么 Watchdog 就会重启系统。

## 基本操作

操作系统内核会将 CPU 芯片上的 Watchdog 外设抽象为文件系统中的一个字符设备:

{% codeblock ubuntu@arm:~$ lang:bash %}
ls /dev/watchdog*
# /dev/watchdog  /dev/watchdog0
{% endcodeblock %}

上面命令列出来系统中的 Watchdog 设备（`/dev/watchdog`、`/dev/watchdog0` 在底层指向同一个硬件）。使用其中任一设备都可以。

0. 打开设备

{% codeblock api_watchdog_open lang:c %}
watchdog_fd = open(watchdog_dev, O_RDWR);
{% endcodeblock %}

根据头文件 `linux/watchdog.h` 中的定义，我们可以通过得到的`文件描述符`对 watchdog 设备做后面这些操作。

1. 设置 watchdog 超时时间

{% codeblock api_watchdog_settimeout lang:c %}
ioctl(watchdog_fd, WDIOC_SETTIMEOUT, &seconds);
{% endcodeblock %}

2. ping watchdog

{% codeblock api_watchdog_settimeout lang:c %}
ioctl(watchdog_fd, WDIOC_KEEPALIVE, NULL);
{% endcodeblock %}

可以将上面的这些操作[封装成函数接口](/blog/2020/02/22/Linux-系统中-Watchdog-的应用/main.c)，在函数内部做一些错误处理。

## 测试

{% codeblock main.c lang:c %}
int main(void)
{
	struct timespec wait_time = { 1, 0 };

	if (api_watchdog_init(WATCHDOG_DEV, WATCHDOG_TIMEOUT) < 0)
		return -1;

	printf("Watchdog opened!\n");

	while (1) {
		api_watchdog_feed();
		nanosleep(&wait_time, NULL);
	}
}
{% endcodeblock %}

如上，先初始化 watchdog 然后每秒 ping watchdog 一次，此时系统如常运行。当 kill 此程序后，由于前面在内核开启了 `Disable watchdog shutdown on close` 选项 watchdog 会继续工作，同时没有继续 ping 的操作所以系统进入重启流程。

## 总结

文本介绍了如何在 Linux 操作系统中使用 watchdog，分为以下方面的工作:

- 内核选项配置
- 部分 API 介绍
- 设备操作接口封装
