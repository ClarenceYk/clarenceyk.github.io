---
title: iMX6Q 2GB 内存支持
date: 2020-03-16 16:18:41
tags:
- iMX6Q
- 内存拓展
---

iMX6Q SABRESD 开发板的板载 DDR 内存大小为 1GB 其配套的 codebase 也针对 1GB 内存大小适配。我们的运行平台上配备了 2GB 内存，直接使用原有 codebase 将只能使用其中一半内存空间。以下将介绍如何让系统识别并使用 2GB 内存。

<!-- more -->

## 系统启动过程现象

系统启动过程中可看到 U-Boot 输出如下信息:

{% codeblock 终端输出 lang:bash %}
U-Boot 2019.10-rc1-00134-gacda5922db-dirty (Mar 16 2020 - 15:27:54 +0800)

CPU:   Freescale i.MX6Q rev1.6 996 MHz (running at 792 MHz)
CPU:   Automotive temperature grade (-40C to 125C) at 36C
Reset cause: POR
Model: Freescale i.MX6 Quad SABRE Smart Device Board
Board: MX6-SabreSD
I2C:   ready
DRAM:  1 GiB
PMIC:  PFUZE100 ID=0x10
MMC:   FSL_SDHC: 0, FSL_SDHC: 3
{% endcodeblock %}

其中 `DRAM:  1 GiB` 这一行表示 U-Boot 识别到的内存空间大小为 1GB。

进入操作系统之后，终端执行如下命令:

{% codeblock 终端输出 lang:bash %}
ubuntu@arm:~$ cat /proc/meminfo
MemTotal:        1031138 kB
...
{% endcodeblock %}

可知操作系统识别到的内存空间大小也是 1GB。

## 识别内存大小的过程

查看 U-Boot 源码，初始化函数 `board_init_f` 会执行一系列初始化操作:

{% codeblock u-boot/common/board_f.c lang:c %}
if (initcall_run_list(init_sequence_f))
	hang();
{% endcodeblock %}

`init_sequence_f` 是一个数组，所有初始化函数都放在这个数组中，其中有2个函数:

{% codeblock imx6q/u-boot/common/board_f.c lang:c %}
static const init_fnc_t init_sequence_f[] = {
    // ...
    announce_dram_init,
    dram_init,		/* configure available RAM banks */
    // ...
}
{% endcodeblock %}

`announce_dram_init` 函数只有一行代码作用是输出 `DRAM:`。真正需要我们关心的是 `dram_init`，这个函数继续调用了另一个函数 `imx_ddr_size`，接着往下追发现如下注释信息:

{% codeblock u-boot/arch/arm/mach-imx/cpu.c lang:c %}
/*
 * imx_ddr_size - return size in bytes of DRAM according MMDC config
 * The MMDC MDCTL register holds the number of bits for row, col, and data
 * width and the MMDC MDMISC register holds the number of banks. Combine
 * all these bits to determine the meme size the MMDC has been configured for
 */
unsigned imx_ddr_size(void)
{
    struct esd_mmdc_regs *mem = (struct esd_mmdc_regs *)MEMCTL_BASE;
    // ...
}
{% endcodeblock %}

可看到函数 `imx_ddr_size` 根据 `MMDC` 的配置返回内存空间大小，配置信息存储在寄存器 `MMDC_MDCTL` 中。

## 适配 2GB 内存

从前面的代码中可看到寄存器 `MMDC_MDCTL` 的地址在定义 `MEMCTL_BASE` 中，接着查看 MEMCTL_BASE 的定义，可得到如下信息:

```
MEMCTL_BASE
 |
 v
MMDC_P0_BASE_ADDR
 |
 v
AIPS2_OFF_BASE_ADDR + 0x30000
 |
 v
ATZ2_BASE_ADDR + 0x80000 + 0x30000
 |
 v
AIPS2_ARB_BASE_ADDR + 0x80000 + 0x30000
 |
 v
0x02100000 + 0x80000 + 0x30000 = 0x021b0000
```

所以 MMDC_MDCTL 寄存器地址为 `0x021b0000`。此处代码是从寄存器中读取配置的值，那么一定会有其他的代码去配置这个寄存器的值，继续查找可发现:

{% codeblock u-boot/board/freescale/mx6sabresd/mx6sabresd.c lang:c %}
static int mx6q_dcd_table[] = {
    //...
    0x021b0040, 0x00000027,
    0x021b0000, 0x831A0000,
    //...
}
{% endcodeblock %}

原始 codebase 中向寄存器 MMDC_MDCTL 配置了值 `0x831A0000` 以及向寄存器 MMDC_MDASP 配置了值 `0x00000027`。

接着从 iMX6Q 芯片参考手册中可查到寄存器 MMDC_MDCTL 数据位的分配:

![]()
{% img /2020/03/16/iMX6Q-2GB-内存支持/IMX6DQRM_44_12_1_0.png '"寄存器 MMDC_MDCTL" "寄存器 MMDC_MDCTL"' %}

从而得知原始代码中，寄存器 MMDC_MDCTL 配置的含义为：

- 使能 CS0
- 关闭 CS1
- 行地址宽度 14bit
- 列地址宽度 10bit
- burst 长度 8
- 数据总线大小 64bit

我们运行平台上的内存芯片和 iMX6Q SABRESD 开发板上的内存芯片是一样的封装，只是容量是后者的2倍，也就是`行地址宽度`多了1bit，所以将 MMDC_MDCTL 寄存器的值配置为 `0x841A0000`。

对于寄存器 `MMDC_MDASP`，芯片手册中描述如下:

{% blockquote Freescale, i.MX 6Dual/6Quad Applications Processor Reference Manual %}
MMDCx_MDASP[CS0_END] should be set to DDR_CS_SIZE/32MB + 0x7 (DDR base address begins at 0x10000000)
{% endblockquote %}

所以寄存器 MMDC_MDASP 的值应该配置为 `内存大小/32MB + 0x7` 也就是 `0x00000047`，最后配置如下:

{% codeblock u-boot/board/freescale/mx6sabresd/mx6sabresd.c lang:c %}
static int mx6q_dcd_table[] = {
    //...
    0x021b0040, 0x00000047,
    0x021b0000, 0x841A0000,
    //...
}
{% endcodeblock %}

## 设备树修改

将 U-Boot 以及内核设备树与内存大小相关的地方修改为 2GB:

{% codeblock u-boot/arch/arm/dts/imx6qdl-sabresd.dtsi lang:dts %}
memory@10000000 {
    reg = <0x10000000 0x80000000>;
};
{% endcodeblock %}

最后，将以上修改保存到设备中，启动系统之后可以看到 U-Boot 以及操作系统均识别到 2GB 内存空间。

## 总结

本文介绍了 U-Boot 在启动时识别内存大小的过程，以及 iMX6Q 关于内存信息的几个寄存器的配置，最终实现让系统平台识别并且使用 2GB 内存空间。
