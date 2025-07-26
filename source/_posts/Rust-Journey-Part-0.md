---
title: Rust Journey - Part0
tags:
  - Rust
date: 2025-07-26 14:06:00
updated: 2025-07-26 14:06:00
---

Rust Programming language 是一个相对年轻的编程语言（和 C 比较起来的话）也是一门潜力十足的语言。甚至不能说是“潜力”了，因为在产业中已经有很多应用的案例，比如最近的 DebConf25 大会上 Fabian 指出，Debian Sid 中大约有 8% 的源码包至少基于一个 librust* 包进行构建；又比如 Ubuntu 正在积极探索将基于 Rust 编写的 uutils 用于替换 GNU Core Utilities。

另外，Rust 语言本身的功能和特性也趋于稳定，其工具链也相当成熟了，同时 Rust 社区也是非常的活跃。还有就是 Linus 也比较支持 Rust 进入 Linux Kernel（虽然年初的时候发生了好几起有争议的事件）。基于以上，我觉得现在是一个很好的学习 Rust 的时机。

<!-- more -->

学习一门新语言当然要从搭建环境开始。正好今年购入一台 M4 的 MacBook Air，所以就在 MacOS 的环境中搭建 Rust 编程环境。

## Rust Rover

JetBrain 是 IDE 领域的龙头，他家的系列编程 IDE 都非常的好用，例如针对 C/C++ 开发人员推出的 Clion，针对 Go 开发人员的 GoLand。这次我使用的 Rust Rover 也是他家推出的。正好最近免费了（个人非商业用途），于是尝试一下。

直接从官方网站下载即可：[Rust Rover](https://www.jetbrains.com/rust/)。Mac 用户下载时注意区分一下 CPU 类型（Apple silicon / Intel）。

## Rust toolchian

安装 Rust 语言的工具链也很简单，打开终端，键入如下命令再敲回车即可：

```
$ curl --proto '=https' --tlsv1.2 https://sh.rustup.rs -sSf | sh
```

此外，如果 Mac 上没有安装过开发工具那么需要再执行如下命令：

```
$ xcode-select --install
```

安装之后 Mac 上就有了 C/C++ compiler 以及 linker 了。Rust 的工具链依赖这个。

## Hello, World!

准备工作做好之后就可以开始我们的 Rust 之旅了。

### 创建一个目录

任何一个项目都需要创建一个目录便于管理文件，Rust 项目也不例外。这个项目目录在哪里无管紧要，重要的是需要这样一个目录。

打开终端，键入：

```
$ mkdir ~/projects
$ cd ~/projects
$ mkdir hello_world
$ cd hello_world
```

### 编写并运行程序

首先在不使用 IDE 的情况下写一个程序，可以熟悉一下编译 Rust 程序的操作流程。Rust 程序源码文件总是用 `.rs` 结尾的。现在创建并打开 `main.rs` 文件并输入如下源码：

``` rust
fn main() {
    println!("Hello, world!");
}
```

保存并关闭文件之后回到终端里，进入目录 `~/projects/hello_world`，输入如下命令来编译和运行程序：

```
$ rustc main.rs
$ ./main
Hello, world!
```

到这里，我们就编写了第一个 Rust 程序。

## Rust 程序的构成

让我们更仔细地看一下 “Hello, world!” 程序的构成。首先是如下代码：

``` rust
fn main() {

}
```

这几行定义了一个名称为 `main` 的函数，`main` 函数比较特殊它是每个 Rust 程序执行的入口。这里定义的 `main` 函数没有形式参数以及返回值。如果一个函数有形式参数的话，这些参数会定义在圆括号内。

函数的函数体被包含在 `{}` 里。Rust 中要求所有的函数体都包含在花括号内。同时，一个比较好的编码风格是将花括号的第一个写在函数名的同一行，并且隔开一个空格。此外，如果懒得去注意这些编码风格，可以直接使用 Rust 工具链中自带的 `rustfmt` 程序来自动规范编码风格（当然使用 IDE 的话，IDE 基本都带自动 format 功能）。

我们的程序里，函数体里只有一行代码：

``` rust
println!("Hello, world!");
```

这行代码就实现了全部功能——打印文字到终端输出上。这里虽然只有一行代码，但是却需要注意3点：

第一，`println!` 调用的是 Rust 的**宏**，如果 `println` 是个函数的话，那么调用它应该直接用 `println`（没有感叹号）。目前我们只需要知道宏也是 Rust 的代码，但是这类代码是通过生成代码来拓展 Rust 语法的。调用宏需要使用感叹号，而调用函数不需要。

第二，`Hello, world!` 是一个字符串，我们把这个字符串作为参数传给了 `println!`。

第三，在这行的结尾处使用了分号，用于表示表达式的结束以及下一个表达式的开始。绝大多数的 Rust 代码都用分号结尾。

对于简单的程序，我们使用 `rustc` 来编译就够了，但是随着项目的开发我们需要一个工具来帮助我们管理各种选项甚至是和其他人共同开发，这就是 `Cargo` 的作用。使用它可以帮助我们写出 `real-world` Rust 程序。

