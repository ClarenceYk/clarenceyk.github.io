---
title: ROSR 学习笔记 - 第2章
date: 2022-11-11 10:38:40
updated: 2022-11-11 10:38:40
tags:
- RISC-V
- OS
- Rust
---

ROSR 是『RISC-V OS using Rust』的缩写，是由 [Stephen Marz](https://osblog.stephenmarz.com/) 在他的系列博客中提供的操作系统开发教程。

本章描述使用 Rust 的 unsafe 功能操作 MMIO 来控制 UART 进而实现 `println!` 等宏。

<!-- more -->

## Universal Asynchronous Reciever / Transmitter (UART)

QEMU 内部虚拟了一个 NS16550A UART 芯片，查看 [qemu/hw/riscv/virt.c](https://github.com/qemu/qemu/blob/master/hw/riscv/virt.c) 代码可知：

```C
static const MemMapEntry virt_memmap[] = {
    // ...
    [VIRT_UART0] =        { 0x10000000,         0x100 },
};
```

通过物理地址 0x10000000，即可访问 NS16550A 芯片的寄存器。BTW，这种通过物理地址访问的 IO 被称为 MMIO。

从下图可以看到发送（THR）接收（RBR）寄存器都是 8 bit（1 byte），我们只要从地址 0x10000000 读取就能从 UART 接收一个字节，向地址 0x10000000 写入就能发送一个字节。

![](/2022/11/11/ROSR-学习笔记-第2章/ns16550a.png)

## Rust 中的 MMIO

```Rust
/// # Safety
///
/// We label the mmio function unsafe since
/// we will be working with raw memory. Rust cannot
/// make any guarantees when we do this.
fn unsafe mmio_write(address: usize, offset: usize, value: u8) {
    // Set the pointer based off of the address
    let reg = address as *mut u8;

    // write_volatile is a member of the *mut raw
    // and we can use the .add() to give us another pointer
    // at an offset based on the original pointer's memory
    // address. NOTE: The add uses pointer arithmetic so it is
    // new_pointer = old_pointer + sizeof(pointer_type) * offset
    reg.add(offset).write_volatile(value);
}

/// # Safety
///
/// We label the mmio function unsafe since
/// we will be working with raw memory. Rust cannot
/// make any guarantees when we do this.
fn unsafe mmio_read(address: usize, offset: usize, value: u8) -> u8 {
    // Set the pointer based off of the address
    let reg = address as *mut u8;

    // read_volatile() is much like write_volatile() except it
    // will grab 8-bits from the pointer and give that value to us.
    // We don't add a semi-colon at the end here so that the value
    // is "returned".
    reg.add(offset).read_volatile()
}
```
## Rust UART 驱动

UART 驱动包括：

- 数据结构用于存储基地址
- 初始化方法
- 输出一个字符函数（put）
- 读取一个字符函数（get）
- 实现 Write traits

### 数据结构定义

```Rust
pub struct UartDriver {
    base_address: usize,
}
```

### 增加初始化方法

```Rust
impl UartDriver {
    // ...

    pub fn init(&self) {
        let ptr = self.base_address as *mut u8;
        unsafe {
            let lcr = (1 << 0) | (1 << 1);
            
            ptr.add(3).write_volatile(lcr);
            ptr.add(2).write_volatile(1 << 0);
            ptr.add(1).write_volatile(1 << 0);
            
            let divisor: u16 = 12;
            let divisor_least: u8 = (divisor & 0xff) as u8;
            let divisor_most:  u8 = (divisor >> 8) as u8;
            ptr.add(3).write_volatile(lcr | 1 << 7);
            ptr.add(0).write_volatile(divisor_least);
            ptr.add(1).write_volatile(divisor_most);
            
            ptr.add(3).write_volatile(lcr);
        }
    }

    // ...
}
```

7~9行，设置8位数据、无停止位、无奇偶（因为是模拟的 UART，一般硬件设置为 8N1）

10、11行，开启收中断、开启 FIFO

13~18行，设置波特率 115200（因为是模拟的 UART 所以波特率设置不影响）

根据 NS16550A 芯片资料，其内部有一个全局时钟频率为 22.729 MHz（22729000 Hz），所以要设置 115200 波特率： divisor = 22729000 / (115200 * 16) = 12。

并且在设置波特率之前需把 LCR 寄存器的 DLAB 位置1，设置完后再置0。

### 增加 put 方法

```Rust
impl UartDriver {
    // ...
    fn put(&self, c: u8) {
        let ptr = self.base_address as *mut u8;
        unsafe {
            ptr.add(0).write_volatile(c);
        }
    }
    // ...
}
```

### 增加 get 方法

```Rust
impl UartDriver {
    // ...
    pub fn get(&self) -> Option<u8> {
        let ptr = self.base_address as *mut u8;
        unsafe {
            if ptr.add(5).read_volatile() & 1 == 0 {
                None
            }
            else {
                Some(ptr.add(0).read_volatile())
            }
        }
    }
    // ...
}
```

这里我们的内核还没处理中断，所以采用 poll 的方式检测是否有数据。

### 实现 Write traits

这一步是为了 print! 宏做准备。

```Rust
use core::fmt::{Error, Write};

// ...

impl Write for UartDriver {
    fn write_str(&mut self, s: &str) -> Result<(), Error> {
        for c in s.bytes() {
            self.put(c);
        }
        Ok(())
    }
}
```

## 实现 print! 宏

第1章中，`lib.rs` 文件中的 print! 宏的实现是空，现在我们可以填入内容了：

```Rust
#[macro_export]
macro_rules! print
{
    ($($args:tt)+) => {{
        use core::fmt::Write;
        let _ = write!(uart::UartDriver::new(0x1000_0000), $($args)+);
    }};
}
```

## Hello, World!

在 kmain 入口处初始化 UART 后就可以向终端输出信息了：

```Rust
#[no_mangle]
extern "C" fn kmain() {
    let my_uart = uart::UartDriver::new(0x1000_0000);
    my_uart.init();

    println!("Hello, World!");
}
```

同样也能从终端获取用户输入：

```Rust
#[no_mangle]
extern "C" fn kmain() {
    // ...

    loop {
        if let Some(c) = my_uart.get() {
            match c {
                8 | 127 => {
                    print!("{}{}{}", 8 as char, ' ', 8 as char);
                },
                10 | 13 => {
                    println!();
                },
                _ => {
                    print!("{}", c as char);
                }
            }
        }
    }
}
```

