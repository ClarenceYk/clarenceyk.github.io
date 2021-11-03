---
title: Asterisk 交叉编译
date: 2021-11-02 10:06:11
updated: 2021-11-02 10:06:11
tags:
- Asterisk
- Cross Compilation
---

本文将介绍如何交叉编译 Asterisk。

<!-- more -->

## 环境介绍

开发环境：

- 操作系统: `Ubuntu 20.04 LTS`
- CPU 架构: `x86_64`

目标环境：

- 操作系统: `Linux 平台(内核版本 2.6.32)`
- CPU 架构: `mipsel`

## Asterisk 依赖要求

Asterisk 与各模块间的依赖如下图：

![](/2021/11/02/asterisk-cross-compilation/dependencies.png)

## 开发环境准备

安装必备的构建工具：

```shell
sudo apt install -y \
  build-essential \
  autoconf \
  automake \
  bison \
  flex \
  git \
  libtool \
  libtool-bin \
  make \
  pkg-config
```

准备交叉编译工具链：

这一步需要注意的是，在安装交叉编译工具链时，需要指定一个目标架构，这里我们使用 `mipsel`，这个架构是 `mipsel-linux-gnu-gcc` 的目标架构。

对于 `mipsel` 架构所对应的交叉编译工具软件包可从上游厂商处获取。其他架构如 `arm`、`arm64` 可从 ARM 官方下载，或直接通过软件包管理工具获取（debian 仓库），或者下载第三方提供的二进制文件（如 Linaro 提供）。

工作目录准备：

```plaintxt
MIPSEL_Cross-Compile
├── build_opt
├── build_source
├── build_start
└── toolchain
```

建立目录 `MIPSEL_Cross-Compile` 与子目录 `build_opt`、`build_source`、`build_start`、`toolchain`，并将 `MIPSEL_Cross-Compile/toolchain` 目录中的交叉编译工具链添加到环境变量中，以便后续的编译操作可以使用。

```shell
export PATH="$PATH:/home/ubuntu/MIPSEL_Cross-Compile/toolchain/gcc-4.4-gnu/bin"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/ubuntu/MIPSEL_Cross-Compile/toolchain/gcc-4.4-gnu/mipsel-linux/lib"
```

- `build_opt`: 存放编译后产生的可执行文件、库文件等
- `build_source`: 存放源代码压缩包
- `build_start`: 存放源代码
- `toolchain`: 存放交叉编译工具链

准备源代码：

各个所需模块的源代码通过下载获取（搜索一下便可得到下载地址），并将源代码压缩包拷贝到 `build_source` 目录中。

```plaintxt
asterisk-18-current.tar.gz
asterisk-g72x.zip
bcg729-1.1.1.tar.gz
libedit-20210910-3.1.tar.gz
libuuid-1.0.3.tar.gz
libxml2-2.9.12.tar.gz
ncurses-6.2.tar.gz
openssl-1.1.1l.tar.gz
sqlite-autoconf-3360000.tar.gz
zlib-1.2.11.tar.gz
```

将这些压缩包都解压到 `build_start` 目录中。

## 交叉编译

编译顺序为：

```plaintxt
ncurses
libedit
sqlite3
libuuid
zlib
libxml2
openssl
asterisk
bcg729
asterisk-g72x
```

### ncurses

```shell
cd /home/ubuntu/MIPSEL_Cross-Compile/build_start/ncurses-6.2

CC=mipsel-linux-gcc CXX=mipsel-linux-g++ \
./configure --build=x86_64-linux-gnu --host=mipsel-none-linux-gnu \
--disable-stripping \
--with-shared --without-ada --enable-termcap \
--with-build-cc=gcc --with-build-cpp=gcc \
--prefix=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/ncurses_build_for_mipsel

make && make install
```

> Tips: 参数 `--with-build-cc` 以及 `--with-build-cpp` 指定的是开发环境本地编译使用的 gcc 工具链，而非交叉编译工具链。

### libedit

```shell
cd /home/ubuntu/MIPSEL_Cross-Compile/build_start/libedit-20210910-3.1

CC=mipsel-linux-gcc \
CFLAGS="-I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/ncurses_build_for_mipsel/include -I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/ncurses_build_for_mipsel/include/ncurses" \
LDFLAGS="-L/home/ubuntu/MIPSEL_Cross-Compile/build_opt/ncurses_build_for_mipsel/lib" \
./configure --build=x86_64-linux-gnu --host=mipsel-none-linux-gnu \
--prefix=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libedit_build_for_mipsel

make && make install
```

> Tips: `libedit` 依赖于 `ncurses`，所以需要指定 `CFLAGS`、`LDFLAGS` 等参数，以便编译时能够正确搜索 `ncurses` 相关的头文件和库文件。

### sqlite3

```shell
cd /home/ubuntu/MIPSEL_Cross-Compile/build_start/sqlite-autoconf-3360000

CC=mipsel-linux-gcc CXX=mipsel-linux-g++ \
./configure --build=x86_64-linux-gnu --host=mipsel-none-linux-gnu \
--prefix=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/sqlite3_build_for_mipsel

make && make install
```

### libuuid

```shell
cd /home/ubuntu/MIPSEL_Cross-Compile/build_start/libuuid-1.0.3

CC=mipsel-linux-gcc CXX=mipsel-linux-g++ \
./configure --build=x86_64-linux-gnu --host=mipsel-none-linux-gnu \
--prefix=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libuuid_build_for_mipsel

make && make install
```

### zlib

```shell
cd /home/ubuntu/MIPSEL_Cross-Compile/build_start/zlib-1.2.11

CC=mipsel-linux-gcc CXX=mipsel-linux-g++ \
./configure --prefix=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/zlib_build_for_mipsel

make && make install
```

### libxml2

```shell
cd /home/ubuntu/MIPSEL_Cross-Compile/build_start/libxml2-2.9.12

./autogen.sh CC=mipsel-linux-gcc \
CFLAGS="-I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/zlib_build_for_mipsel/include" \
LDFLAGS="-L/home/ubuntu/MIPSEL_Cross-Compile/build_opt/zlib_build_for_mipsel/lib" \
--build=x86_64-linux-gnu --host=mipsel-none-linux-gnu \
--prefix=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libxml2_build_for_mipsel

make && make install
```

> Tips: `libxml2` 依赖于 `zlib`，所以需要指定 `CFLAGS`、`LDFLAGS` 等参数，以便编译时能够正确搜索 `zlib` 相关的头文件和库文件。
>
> Notice: 与前面编译命令不同 `libxml2` 使用 `./autogen.sh` 而不是 `./configure`，autogen.sh 会生成并运行 configure 同时将传给 autogen.sh 的编译参数传给 configure。

### openssl

```shell
cd /home/ubuntu/MIPSEL_Cross-Compile/build_start/openssl-1.1.1l

./Configure linux-mips32 \
--prefix=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/openssl_build_for_mipsel \
--openssldir=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/openssl_build_for_mipsel \
shared

sed -i "s/\(CROSS_COMPILE=\).*/\1mipsel-linux-/g" Makefile
sed -i "s/\(CFLAGS=.*\)/\1 -mabi=32 -mglibc -march=mips32/g" Makefile
sed -i "s/\(CXXFLAGS=.*\)/\1 -mabi=32 -mglibc -march=mips32/g" Makefile

make && make install
```

> Notice: `Configure` 是大写的 `C`，并且执行完成后需要手动修改 Makefile。

### asterisk

设置编译 Asterisk 使用的 `CFLAGS`、`LDFLAGS` 参数：

```shell
AST_CFLAGS="-I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/ncurses_build_for_mipsel/include"
AST_CFLAGS="$AST_CFLAGS -I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/ncurses_build_for_mipsel/include/ncurses"
AST_CFLAGS="$AST_CFLAGS -I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libedit_build_for_mipsel/include"
AST_CFLAGS="$AST_CFLAGS -I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/sqlite3_build_for_mipsel/include"
AST_CFLAGS="$AST_CFLAGS -I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libuuid_build_for_mipsel/include"
AST_CFLAGS="$AST_CFLAGS -I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/zlib_build_for_mipsel/include"
AST_CFLAGS="$AST_CFLAGS -I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libxml2_build_for_mipsel/include"
AST_CFLAGS="$AST_CFLAGS -I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libxml2_build_for_mipsel/include/libxml2"
AST_CFLAGS="$AST_CFLAGS -I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/openssl_build_for_mipsel/include"

AST_LDFLAGS="-L/home/ubuntu/MIPSEL_Cross-Compile/build_opt/ncurses_build_for_mipsel/lib"
AST_LDFLAGS="$AST_LDFLAGS -L/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libedit_build_for_mipsel/lib"
AST_LDFLAGS="$AST_LDFLAGS -L/home/ubuntu/MIPSEL_Cross-Compile/build_opt/sqlite3_build_for_mipsel/lib"
AST_LDFLAGS="$AST_LDFLAGS -L/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libuuid_build_for_mipsel/lib"
AST_LDFLAGS="$AST_LDFLAGS -L/home/ubuntu/MIPSEL_Cross-Compile/build_opt/zlib_build_for_mipsel/lib"
AST_LDFLAGS="$AST_LDFLAGS -L/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libxml2_build_for_mipsel/lib"
AST_LDFLAGS="$AST_LDFLAGS -L/home/ubuntu/MIPSEL_Cross-Compile/build_opt/openssl_build_for_mipsel/lib"
AST_LDFLAGS="$AST_LDFLAGS -lncurses"
```

设置 Asterisk 运行时路径：

```shell
AST_RUNTIME_DIR="/root"
```

开始编译：

```shell
CC=mipsel-linux-gcc CXX=mipsel-linux-g++ AR=mipsel-linux-ar RANLIB=mipsel-linux-ranlib \
CFLAGS=$AST_CLFAGS LDFLAGS=$AST_LDFLAGS \
./configure --build=x86_64-linux-gnu --host=mipsel-none-linux-gnu \
--prefix=$AST_RUNTIME_DIR/build_opt/asterisk_build_for_mipsel \
--with-libedit=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libedit_build_for_mipsel \
--with-sqlite3=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/sqlite3_build_for_mipsel \
--with-libxml2=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/libxml2_build_for_mipsel \
--with-crypto=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/openssl_build_for_mipsel \
--with-ssl=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/openssl_build_for_mipsel \
--without-bluetooth \
--without-dahdi \
--with-jansson-bundled \
--with-pjproject-bundled \
--without-ldap \
--without-netsnmp \
--without-pri \
--disable-xmldoc

make menuselect/menuselect menuselect-tree menuselect.makeopts
menuselect/menuselect --enable-all

make
make install DESTDIR=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/asterisk_build_for_mipsel
make samples DESTDIR=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/asterisk_build_for_mipsel
```

> Tips: 如果第 19 行报错，则再运行一次即可。

由于指定的运行路径和安装路径有可能不一致，所以需要修复一下：

```shell
mv /home/ubuntu/MIPSEL_Cross-Compile/build_opt/asterisk_build_for_mipsel/$AST_RUNTIME_DIR/build_opt/asterisk_build_for_mipsel /tmp/ast_tmp
rm -rf /home/ubuntu/MIPSEL_Cross-Compile/build_opt/asterisk_build_for_mipsel
mv /tmp/ast_tmp /home/ubuntu/MIPSEL_Cross-Compile/build_opt/asterisk_build_for_mipsel
```

### bcg729

```shell
cd /home/ubuntu/MIPSEL_Cross-Compile/build_start/bcg729-1.1.1

CC=mipsel-linux-gcc \
cmake . -DCMAKE_INSTALL_PREFIX=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/bcg729_build_for_mipsel

make && make install
```

> Tips: bcg729 最近的几个版本改为使用 cmake 构建，如果需要用 configure 的构建方式下载旧版本即可。

### asterisk-g72x

```shell
cd /home/ubuntu/MIPSEL_Cross-Compile/build_start/asterisk-g72x-master

./autogen.sh
CC=mipsel-linux-gcc \
CFLAGS="-O3 -fomit-frame-pointer -I/home/ubuntu/MIPSEL_Cross-Compile/build_opt/bcg729_build_for_mipsel/include" \
LDFLAGS="-L/home/ubuntu/MIPSEL_Cross-Compile/build_opt/bcg729_build_for_mipsel/lib" \
./configure --build=x86_64-linux-gnu --host=mipsel-none-linux-gnu \
--prefix=/home/ubuntu/MIPSEL_Cross-Compile/build_opt/asterisk_build_for_mipsel \
--with-bcg729 \
--with-asterisk-includes=/home/ubuntu/MIPSEL_Cross-Compile/build_start/asterisk-18.7.1/include

make && make install
```

> Tips: `--with-asterisk-includes` 参数指定 Asterisk 源代码路径中的头文件。
