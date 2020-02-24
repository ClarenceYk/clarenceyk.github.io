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

2. feed watchdog

{% codeblock api_watchdog_feed lang:c %}
ioctl(watchdog_fd, WDIOC_KEEPALIVE, NULL);
{% endcodeblock %}

可以将上面的这些操作[封装成函数接口](https://gist.github.com/ClarenceYk/c71502b63378e3fbcd763fdaa658803d)，在函数内部做一些错误处理。

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

如上，先初始化 watchdog 然后每秒 feed watchdog 一次，此时系统如常运行。当 kill 此程序后，由于前面在内核开启了 `Disable watchdog shutdown on close` 选项 watchdog 会继续工作，同时没有继续 feed 的操作所以系统进入重启流程。

## 引入外部 Kick 信号

以一个具体的应用场景为例，如下:

{% codeblock 硬件连接 %}
-------   GPIO  --------
| CPU | <------ | FPGA |
-------         --------
{% endcodeblock %}

FPGA 通过 CPU 的 GPIO 外设向开发板输入一个周期性翻转信号。CPU 在每一个周期开始时重置 watchdog，我们可以检测 GPIO 的上升沿或者下降沿获得周期开始的信息。

对 GPIO 的操作在 Linux 环境中有很多方法实现，这里我们使用 [libgpiod](https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/) 库来实现对 GPIO 上升沿信号的检测。

`libgpiod` 中封装了很多便于使用的 API。在当前使用场景中，只需调用函数 `gpiod_ctxless_event_monitor` 就可实现我们想要的功能，其函数签名以及相应的文档注释如下:

```c
/**
 * @brief Wait for events on a single GPIO line.
 * @param device Name, path, number or label of the gpiochip.
 * @param event_type Type of events to listen for.
 * @param offset GPIO line offset to monitor.
 * @param active_low The active state of this line - true if low.
 * @param consumer Name of the consumer.
 * @param timeout Maximum wait time for each iteration.
 * @param poll_cb Callback function to call when waiting for events.
 * @param event_cb Callback function to call for each line event.
 * @param data User data passed to the callback.
 * @return 0 if no errors were encountered, -1 if an error occurred.
 * @note The way the ctxless event loop works is described in detail in
 *       ::gpiod_ctxless_event_monitor_multiple - this is just a wrapper aound
 *       this routine which calls it for a single GPIO line.
 */
int gpiod_ctxless_event_monitor(const char *device, int event_type,
                                unsigned int offset, bool active_low,
                                const char *consumer,
                                const struct timespec *timeout,
                                gpiod_ctxless_event_poll_cb poll_cb,
                                gpiod_ctxless_event_handle_cb event_cb,
                                void *data) GPIOD_API;
```

如上可以看出几个关键参数，通过 `device`、`offset` 参数指定使用的 GPIO 管脚，`event_type` 指定检测事件（如，上升沿事件），`event_cb` 是触发指定事件后调用的回调函数，其调用方法大致如下:

{% codeblock 上升沿检测 lang:c %}
// WDOG_GPIO_SIG_CHIP "/dev/gpiochip6"
// WDOG_GPIO_SIG_PORT 7
gpiod_ctxless_event_monitor(WDOG_GPIO_SIG_CHIP, GPIOD_CTXLESS_EVENT_RISING_EDGE,
                        WDOG_GPIO_SIG_PORT, false, "wdog", &timeout,
                        NULL, gpio6_port7_rising_edge_handle_cb, NULL);
{% endcodeblock %}

接下来只需要再定义回调函数即可，回调函数的函数签名如下:

```c
/**
 * @brief Simple event callback signature.
 *
 * The callback function takes the following arguments: event type (int),
 * GPIO line offset (unsigned int), event timestamp (const struct timespec *)
 * and a pointer to user data (void *).
 *
 * This callback is called by the ctxless event loop functions for each GPIO
 * event. If the callback returns ::GPIOD_CTXLESS_EVENT_CB_RET_ERR, it should
 * also set errno.
 */
typedef int (*gpiod_ctxless_event_handle_cb)(int, unsigned int,
                                             const struct timespec *, void *);
```

所以我们定义的上升沿事件回调函数大致如下:

{% codeblock 定义回调函数 lang:c %}
int gpio6_port7_rising_edge_handle_cb(int type, unsigned int offset,
                            const struct timespec *timestamp, void *arg)
{
    wdog_count ++;

    if (wdog_count >= WDOG_FEED_PERIOD / FPGA_WDOG_SIG_PERIOD) {
        api_watchdog_feed();
        wdog_count = 0;
    }

    return GPIOD_CTXLESS_EVENT_CB_RET_OK;
}
{% endcodeblock %}

其中关键在于 `api_watchdog_feed()`。

以上就实现了外部 Kick 信号的引入。

## 配置 systemd 服务

我们希望 watchdog 程序随着系统自动启动，所以配置如下的 systemd 服务:

{% codeblock wdog_fpga.service %}
[Unit]
Description=Watchdog systemd service.
ConditionKernelCommandLine=!disable_wdog

[Service]
Type=simple
ExecStart=/usr/bin/wdog_fpga

[Install]
WantedBy=multi-user.target
{% endcodeblock %}

其中我们定义了 `ConditionKernelCommandLine` 参数为 `!disable_wdog`。这样当系统启动之后 systemd 会检测内核启动命令行参数中是否存在`disable_wdog`，如果不存在则启动 watchdog，反之依然。这样配置的好处是，我们可以在系统启动之前配置是否开启 watchdog。

## U-Boot 环境设置内核命令行参数

如果我们不想开启 watchdog，则在 U-Boot 启动后暂停引导内核，然后将 `disable_wdog` 参数设置到内核命令行参数中，如下:

```shell
setenv bootargs ... 其他参数 ... disable_wdog
```

然后在引导内核:

```shell
run bootcmd
```

## 总结

文本介绍了如何在 Linux 操作系统中使用 watchdog，分为以下方面的工作:

- 内核选项配置
- 部分 API 介绍
- 设备操作接口封装
- 实际应用场景示例
