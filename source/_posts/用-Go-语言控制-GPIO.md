---
title: 用 Go 语言控制 GPIO
date: 2020-03-18 18:28:40
updated: 2020-03-18 18:28:40
tags:
- GPIO
- Linux
- memory map
---

GPIO 也就是通用输入/输出外设，在嵌入式开发中会经常遇到的一种外部设备。通过使用软件控制 GPIO 可实现对外输出电平信号；通过读取 GPIO 管脚上的信号可在软件中获取外部信息。操作 GPIO 外设的方法很多，这里将介绍一种在 Linux 环境中通过 `mmap` 系统调用来完成控制的方法。

<!--more-->

## 软硬件环境

- CPU: iMX6Q(ARM)
- Kernel: Linux 4.19.72
- OS: Ubuntu18.04

## GPIO 控制流程简介

iMX6Q 芯片手册上有如下描述:

{% img /2020/03/18/用-Go-语言控制-GPIO/IMX6DQRM_28_4_3_0.png '"GPIO Programming" "GPIO Programming"' %}

由此可知读取 GPIO 某一个管脚上的信号分为3个步骤:

1. 配置管脚复用寄存器，以将此管脚设置为 GPIO 模式。
2. 配置管脚方向寄存器，设置此管脚为输入。
3. 从管脚数据寄存器中读取值。

反之，如果要向外输出信号则为以下3步骤:

1. 配置管脚复用寄存器，以将此管脚设置为 GPIO 模式。
2. 配置管脚方向寄存器，设置此管脚为输出。
3. 向管脚数据寄存器写入值。

## 寄存器地址

iMX6Q 有7组 GPIO 每组有8个 32-bit 的寄存器，每个寄存器上 1-bit 控制着其对应的1个管脚，也就是每组 GPIO 有32个管脚。关于 iMX6Q GPIO 更为详细的介绍可参阅 [Definitive GPIO guide](https://www.kosagi.com/w/index.php?title=Definitive_GPIO_guide)。

对于只使用输入/输出功能的情况，8个寄存器中我们只需关心数据寄存器和方向寄存器这2个（对于管脚复用寄存器，因为默认状态下几乎所有管脚都是 GPIO 模式所以基本不用关心，具体情况请参考芯片手册）。

GPIO 控制寄存器的基地址为 `0x0209C000`，地址宽度为 `0x4000` 也就是第一组 GPIO 的控制寄存器在地址 `0x0209C000` 上，第二组在 `0x020A0000`，以此类推。每一个寄存器是 32-bit 大小所以，第一组 GPIO 的数据寄存器在 `0x0209C000` 方向寄存器在 `0x0209C004`，如下图:

{% img /2020/03/18/用-Go-语言控制-GPIO/IMX6DQRM_28_5_0.png '"GPIO register" "GPIO register"' %}

由此可以在代码中做如下定义:

{% codeblock gpio_imx6q.go lang:go %}
const (
    gpioBaseAddr  uint32 = 0x0209C000
    gpioAddrWidth uint32 = 0x4000
)
{% endcodeblock %}

对于指定的某一组 GPIO 的寄存器地址可这样获得:

{% codeblock gpio_imx6q.go lang:go %}
func getGPIOMMapper(group int) ([]uint32, error) {
    gaddr := gpioBaseAddr + (uint32(group)-1)*gpioAddrWidth
    // ...
}
{% endcodeblock %}

## 内存地址转换

当程序运行起来后操作系统为每一个进程分配了一个虚拟地址空间，而寄存器的地址在实地址空间中，所以需要使用内存地址转换将实地址映射到虚拟地址空间，这样我们才能通过内存地址访问 GPIO 寄存器。

执行终端命令 `man mmap` 可查阅关于 mmap 详细的资料，这里我们通过 Go 语言 `unix` 包中封装的 `Mmap` 函数来使用 mmap 系统调用，如下:

{% codeblock gpio_imx6q.go lang:go %}
func getGPIOMMapper(group int) ([]uint32, error) {
    // ...
    b, err := unix.Mmap(int(memMapFile.Fd()), int64(gaddr), 8, unix.PROT_READ|unix.PROT_WRITE, unix.MAP_SHARED)
    if err != nil {
        return nil, err
    }
    // ...
}
{% endcodeblock %}

其中指定映射的地址长度为8字节，原因是这里我们只使用前2个寄存器。`memMapFile` 可由如下函数获得:

{% codeblock gpio_imx6q.go lang:go %}
func initGPIOMemMap() *os.File {
    file, err := os.OpenFile("/dev/mem", os.O_RDWR, 0600)
    check(err)
    return file
}
{% endcodeblock %}

完成地址转换之后 `unix.Mmap` 函数返回一个 `byte` 类型的切片，而每个寄存器是 32-bit 宽度，也就是一个寄存器对应 4 个 byte，这样操作起来比较麻烦。为了简化操作可以将 `[]byte` 转换为 `[]uint32` 类型:

{% codeblock gpio_imx6q.go lang:go %}
func byte2uint32(b []byte) []uint32 {
    sl := reflect.SliceHeader{}
    sl.Cap = len(b) / 4
    sl.Len = len(b) / 4
    sl.Data = uintptr(unsafe.Pointer(&b[0]))
    return *(*[]uint32)(unsafe.Pointer(&sl))
}
{% endcodeblock %}

## 通过 GPIO 外设输出或读取值

完成以上操作之后，假定得到的内存映射保存在 `var mapper []uint32` 中。如果我们要设置管脚 10 为输出，并且对外输出高电平，则可通过以下操作完成:

{% codeblock gpio_imx6q.go lang:go %}
// 设置方向寄存器中第 10 bit 为1表示管脚10为输出
mapper[1] = mapper[1] | uint32(0x00000001<<10)
// 设置数据寄存器中第 10 bit 为1表示管脚10输出高电平
mapper[0] = mapper[0] | uint32(0x00000001<<10)
{% endcodeblock %}

如果我们要设置管脚 10 为输入，并且读取管脚上的电平信息，则可通过以下操作完成:

{% codeblock gpio_imx6q.go lang:go %}
// 设置方向寄存器中第 10 bit 为0表示管脚10为输入
mapper[1] = mapper[1] & ^uint32(0x00000001<<10)
// 读取数据寄存器中第 10 bit 的值
value := mapper[0] & uint32(0x00000001<<10)
{% endcodeblock %}

## 总结

本文介绍了 GPIO 的基本操作、iMX6Q GPIO 外设寄存器的分布信息以及 Go 语言中使用内存映射的方法，最后实现了使用 Go 语言对 GPIO 进行控制。对于以上的过程我们可以进一步将其封装成 `struct` 方便使用，类似如下:

{% codeblock gpio_imx6q.go lang:go %}
type GPIOPin struct {
    Group   int
    Pin     int
    mmapper []uint32
}
func (pin *GPIOPin) SetDir(dir string) {}
func (pin *GPIOPin) Direction() string {}
func (pin *GPIOPin) Set(v string) {}
func (pin *GPIOPin) Read() string {}
{% endcodeblock %}

除了内存映射之外，还可以使用其他方法操作 GPIO，例如通过 `sysfs` 文件系统或者使用 `/dev/gpiochip` 设备文件等，这里就不做过多介绍了。
