---
title: ROSR 学习笔记 - 第0章
date: 2022-11-10 10:04:39
updated: 2022-11-10 10:04:39
tags:
- RISC-V
- OS
- Rust
---

ROSR 是『RISC-V OS using Rust』的缩写，是由 [Stephen Marz](https://osblog.stephenmarz.com/) 在他的系列博客中提供的操作系统开发教程。

本章描述实验环境的搭建。

<!-- more -->

## 开发环境搭建

现在 Rust 编译器已经增加了对 RISC-V 的支持，所以搭建开发环境变得容易了很多：

1. 安装 rustup
2. 配置 Rust 编译环境
3. 安装 RISC-V 交叉编译工具链
4. 安装 QEMU 的 RISC-V 模拟环境

在 Ubuntu 20.04 上搭建开发环境的具体步骤如下：

### 安装 rustup

参考 Rust 官方文档。

### 配置 Rust 编译环境

命令行执行：

```bash
rustup default nightly
rustup target add riscv64gc-unknown-none-elf
cargo install cargo-binutils
```

使用 nightly 构建是因为需要使用一些 Rust 的稳定版本中没有的语言特性 `#![features]`。

### 安装 RISC-V 交叉编译工具链

使用 ubuntu 软件仓提供的工具链：

```bash
sudo apt update
sudo apt install gcc-riscv64-unknown-elf
```

### 安装 QEMU 的 RISC-V 模拟环境

```bash
sudo apt install qemu-system-riscv64
```

## 创建工程

```bash
cargo new myos --lib
```

在 `Cargo.toml` 项目配置文件中增加：

```toml
[lib]
crate-type = ["staticlib"]
```

创建 `.cargo/config` 文件：

```toml
[build]
target = "riscv64gc-unknown-none-elf"
rustflags = ['-Clink-arg=-Tsrc/lds/virt.lds']

[target.riscv64gc-unknown-none-elf]
runner = "qemu-system-riscv64 -machine virt -cpu rv64 -smp 4 -m 128M -drive if=none,format=raw,file=hdd.dsk,id=foo -device virtio-blk-device,scsi=off,drive=foo -nographic -serial mon:stdio -bios none -device virtio-rng-device -device virtio-gpu-device -device virtio-net-device -device virtio-tablet-device -device virtio-keyboard-device -kernel "
```

在 `src/lds/` 目录中创建 linker script `virt.lds`：

```lds
OUTPUT_ARCH( "riscv" )

ENTRY( _start )

MEMORY
{
    ram   (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 128M
}

PHDRS
{
    text PT_LOAD;
    data PT_LOAD;
    bss PT_LOAD;
}

SECTIONS
{
    .text : {
    PROVIDE(_text_start = .);
    *(.text.init) *(.text .text.*)
    PROVIDE(_text_end = .);
    } >ram AT>ram :text
    PROVIDE(_global_pointer = .);
    .rodata : {
    PROVIDE(_rodata_start = .);
    *(.rodata .rodata.*)
    PROVIDE(_rodata_end = .);
    } >ram AT>ram :text

    .data : {
    . = ALIGN(4096);
    PROVIDE(_data_start = .);
    *(.sdata .sdata.*) *(.data .data.*)
    PROVIDE(_data_end = .);
    } >ram AT>ram :data

    .bss :{
    PROVIDE(_bss_start = .);
    *(.sbss .sbss.*) *(.bss .bss.*)
    PROVIDE(_bss_end = .);
    } >ram AT>ram :bss

    PROVIDE(_memory_start = ORIGIN(ram));
    PROVIDE(_stack = _bss_end + 0x80000);
    PROVIDE(_memory_end = ORIGIN(ram) + LENGTH(ram));
    PROVIDE(_heap_start = _stack);
    PROVIDE(_heap_size = _memory_end - _stack);
}
```

创建 makefile：

```makefile
#####
## BUILD
#####
CC=riscv64-unknown-elf-g++
CFLAGS=-Wall -Wextra -pedantic -Wextra -O0 -g -std=c++17
CFLAGS+=-static -ffreestanding -nostdlib -fno-rtti -fno-exceptions
CFLAGS+=-march=rv64gc -mabi=lp64d
INCLUDES=
LINKER_SCRIPT=-Tsrc/lds/virt.lds
TYPE=debug
RUST_TARGET=./target/riscv64gc-unknown-none-elf/$(TYPE)
LIBS=-L$(RUST_TARGET)
SOURCES_ASM=$(wildcard src/asm/*.S)
LIB=-lsos -lgcc
OUT=os.elf

#####
## QEMU
#####
QEMU=qemu-system-riscv64
MACH=virt
CPU=rv64
CPUS=4
MEM=128M
DRIVE=hdd.dsk

all:
    cargo build
    $(CC) $(CFLAGS) $(LINKER_SCRIPT) $(INCLUDES) -o $(OUT) $(SOURCES_ASM) $(LIBS) $(LIB)
    
run: all
    $(QEMU) -machine $(MACH) -cpu $(CPU) -smp $(CPUS) -m $(MEM)  -nographic -serial mon:stdio -bios none -kernel $(OUT) -drive if=none,format=raw,file=$(DRIVE),id=foo -device virtio-blk-device,scsi=off,drive=foo


.PHONY: clean
clean:
    cargo clean
    rm -f $(OUT)
```

创建硬盘镜像：

```bash
dd if=/dev/zero of=hdd.dsk count=32 bs=1M
```

