---
title: ROSR 学习笔记 - 第1章
date: 2022-11-10 11:30:00
updated: 2022-11-10 11:30:00
tags:
- RISC-V
- OS
- Rust
---

ROSR 是『RISC-V OS using Rust』的缩写，是由 [Stephen Marz](https://osblog.stephenmarz.com/) 在他的系列博客中提供的操作系统开发教程。

本章描述使用汇编语言编写引导程序为 Rust 语言创建执行环境。

<!-- more -->

## 基本思路

1. 将源码编译出目标文件
2. 使用 linker script 通过链接器将目标文件链接成 ELF 文件
3. QEMU 使用 ELF 文件启动
4. QEMU 根据 ELF 文件指定的内存地址将代码段、数据段等放置到内存中相应的位置
5. QEMU 模拟器开始执行程序

## QEMU 内存映射

查看 [qemu/hw/riscv/virt.c](https://github.com/qemu/qemu/blob/master/hw/riscv/virt.c) 文件可知：

```C
static const MemMapEntry virt_memmap[] = {
    // ...
    [VIRT_DRAM] =         { 0x80000000,           0x0 },
};
```

QEMU 从内存地址 `0x80000000` 开始执行程序，所以我们需要把代码链接到此地址上，这一点从第0章的 linker script 也能看出。

## 启动并执行 Rust 程序

分为 3 个步骤：

1. 选择用于启动的 CPU （一般选择 id 为 #0 的）
2. 初始化 BSS 段为 0
3. 跳转 Rust

### 选择启动 CPU

选择一个 CPU 来执行启动流程，原因在于在这个阶段我们并不想去处理并发或者资源竞争等问题。通过 [RISC-V 特权级规范手册](https://github.com/riscv/riscv-isa-manual) 的 3.1.5 节可知，我们能使用 `mhartid` 寄存器来确定当前指令正在被哪个 CPU 执行。

```asm
	csrr	t0, mhartid
	bnez	t0, 3f

3:
	wfi
	j	3b
```

这段代码让 id#0 之外的 CPU 都跳转到 `wfi`（等待中断） 指令处。

### 初始化 BSS 段

确定好由哪个 CPU 执行启动流程后，就需要使用它将 BSS 段初始化清零：

```asm
	la        a0, _bss_start
	la        a1, _bss_end
	bgeu      a0, a1, 2f
1:
	sd        zero, (a0)
	addi      a0, a0, 8
	bltu      a0, a1, 1b
```

其中标号 `_bss_start` 以及 `_bss_end` 由 linker script 确定。

### 跳转 Rust

在跳转执行 Rust 代码前我们必须为 Rust 准备执行环境，也就是设置好栈指针（SP）以及异常程序计数器（mepc）然后利用中断返回（mret）让 CPU 跳转到主函数入口（kmain）。

但是此时的 CPU 并没有被中断，要使用中断返回就必须模拟一个中断的场景使得 mret 指令能够正确执行。

```asm
la        sp, _stack
li        t0, (0b11 << 11) | (1 << 7) | (1 << 3)
csrw      mstatus, t0
la        t1, kmain
csrw      mepc, t1
la        t2, asm_trap_vector
csrw      mtvec, t2
li        t3, (1 << 3) | (1 << 7) | (1 << 11)
csrw      mie, t3
```

上面这段代码中的第1行就是设置 SP 寄存器，同样 `_statck` 符号由 linker script 提供。

第2、3行将 `mstatus` 寄存器的 `MIE`、`MPIE`、`MPP` 位分别置 `b'1`、`b'1` 以及 `b'11`，使得 mret 执行后 `MIE` 为 `b'1`，`MPIE` 为 `b'1`，`MPP` 为 `b'00`，并且 CPU 处于 M 态。

第4、5行将 Rust 程序的入口写入 `mepc` 寄存器，当 mret 执行后 CPU 就会跳转到此处。

第6、7行设置陷阱向量基地址，暂时不用关心。

第8、9行开启软中断、时钟中断、外部中断。

### Rust Code

```Rust
// Steve Operating System
// Stephen Marz
// 21 Sep 2019
#![no_std]
#![feature(panic_info_message)]

use core::arch::asm;

// ///////////////////////////////////
// / RUST MACROS
// ///////////////////////////////////
#[macro_export]
macro_rules! print {
    ($($args:tt)+) => {{}};
}
#[macro_export]
macro_rules! println
{
	() => ({
		print!("\r\n")
	});
	($fmt:expr) => ({
		print!(concat!($fmt, "\r\n"))
	});
	($fmt:expr, $($args:tt)+) => ({
		print!(concat!($fmt, "\r\n"), $($args)+)
	});
}

// ///////////////////////////////////
// / LANGUAGE STRUCTURES / FUNCTIONS
// ///////////////////////////////////
#[no_mangle]
extern "C" fn eh_personality() {}
#[panic_handler]
fn panic(info: &core::panic::PanicInfo) -> ! {
    print!("Aborting: ");
    if let Some(p) = info.location() {
        println!(
            "line {}, file {}: {}",
            p.line(),
            p.file(),
            info.message().unwrap()
        );
    } else {
        println!("no information available.");
    }
    abort();
}
#[no_mangle]
extern "C" fn abort() -> ! {
    loop {
        unsafe {
            // The asm! syntax has changed in Rust.
            // For the old, you can use llvm_asm!, but the
            // new syntax kicks ass--when we actually get to use it.
            asm!("wfi");
        }
    }
}

#[no_mangle]
extern "C" fn kmain() {
    // Main should initialize all sub-systems and get
    // ready to start scheduling. The last thing this
    // should do is start the timer.
}
```

这里的代码和作者原先的代码有如下差异：

```plaintext
< #![feature(panic_info_message,asm)]
---
> #![feature(panic_info_message)]
>
> use core::arch::asm;
```

大概因为目前的 Rust 编译器以及提供了对 `asm` 稳定支持。

