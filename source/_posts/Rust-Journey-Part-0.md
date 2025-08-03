---
title: Rust Journey - Part0
tags:
  - Rust
date: 2025-07-26 14:06:00
updated: 2025-07-27 13:06:00
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

## Cargo

`Cargo` 是一个工具用来编译 Rust 项目以及进行包管理。因为 Cargo 可以简化很多操作包括但不限于以下：

- 编译项目源码
- 自动下载并编译项目依赖的库

所以几乎所有 Rust 程序员都会在项目中使用这个工具。像我们前面写的简单项目，只能用到 Cargo 的部分功能（编译），但随着项目的逐渐变大需要用到第三方库时，使用 Cargo 就能简化这个过程。

前面我们在安装 Rust 工具链中已经包含了 Cargo，可以执行以下命令来确认：

```
$ cargo --version
```

### 使用 Cargo 来创建项目

这次我们用 Cargo 来创建一个“Hello, world!”项目，看看和前文创建的项目有什么不一样。回到我们的项目目录下，同时执行以下命令：

```
$ cargo new hello_cargo
$ cd hello_cargo
```
第一行命令会创建一个新的目录名叫 `hello_cargo` 同时这也是项目的名字，Cargo 会在这个目录下自动创建相应的项目文件。

进入这个目录当中同时列出所有的文件，我们会看到 Cargo 创建了 2 个文件以及一个目录：

- Cargo.toml
- src
- src/main.rc

同时 Cargo 会初始化一个 git 仓库。需要注意的是如果指定的目录下已经存在一个 git 仓库了 Cargo 就不会执行这个动作。

打开 `Cargo.toml` 文件，我们可以看到以下内容：

```toml
[package]
name = "hello_cargo"
version = "0.1.0"
edition = "2024"

[dependencies]
```

此文件是 Cargo 的配置文件，采用 TOML 文件格式。`[package]` 是配置段的头，表示之后的内容用于配置此软件包。后面的三行表示这个软件包的配置内容，例如，软件包的名字、版本信息以及使用什么版本的 rustc 来编译源代码。

最后一个行 `[dependencies]` 表示依赖段，我们可以将项目依赖的其他包列在这里。在 Rust 中一般把软件包称作 *crates*。目前这个项目还不依赖任何软件包，所以这个可以留空。

现在打开 `src/main.rs` 看看：

```rust
fn main() {
    println!("Hello, world!");
}
```

Cargo 自动生成了一个程序和我们之前写的一模一样。到目前 Cargo 生成的项目和之前的项目不同的地方在于源码文件 `main.rs` 放在了 `src` 目录中以及多了一个 `.toml` 文件。

使用 Cargo 来组织项目文件的话，最好把源代码文件都放在 src 目录下，而项目顶层目录可以放一些和源码无关的文件，例如：README、授权信息文件、配置文件等。

### 编译和运行 Cargo 项目

通过 Cargo 来编译项目只需要执行以下命令：

```
$ cargo build
   Compiling hello_cargo v0.1.0 (file:///projects/hello_cargo)
    Finished dev [unoptimized + debuginfo] target(s) in 2.85 secs
```

这条命令会编译出一个可执行文件 `target/debug/hello_cargo`。因为默认是编译 debug 文件，所以路径是 debug 下。这个文件可以直接执行：

```
$ ./target/debug/hello_cargo # or .\target\debug\hello_cargo.exe on Windows
Hello, world!
```

可以看到终端中输出了“Hello, world!”。当第一次执行 `cargo build` 时，Cargo 会创建一个 *Cargo.lock* 文件，这个文件用于记录目录使用的第三方库的版本，我们不用自己去维护这个文件，交给 Cargo 就好。

我们也可以直接执行：

```
$ cargo run
    Finished dev [unoptimized + debuginfo] target(s) in 0.0 secs
     Running `target/debug/hello_cargo`
Hello, world!
```

来运行编译之后的文件。可以看到这次没有编译项相关的输出，这是因为我们没有修改源码，Cargo 判断出不需要重新编译。但是如果我们修改了源码，再执行这个条命令的话：

```
$ cargo run
   Compiling hello_cargo v0.1.0 (file:///projects/hello_cargo)
    Finished dev [unoptimized + debuginfo] target(s) in 0.33 secs
     Running `target/debug/hello_cargo`
Hello, world!
```

Cargo 就会重新编译代码之后再执行程序。

另外，Cargo 还提供了一个 check 命令。这个命令的作用是确保源代码没有错误可以编译通过，但不会创建可执行文件，这样就比 `cargo build` 要执行得快一些。

总结一下 Cargo 的用法：

- 创建项目用：`cargo new`
- 编译项目用：`cargo build`
- 编译并运行项目用：`cargo run`
- 确保项目没有错误用：`cargo check`

### 编译 Release 目标文件

当项目最后需要释放了，就可以使用 `cargo build --release` 来编译目标文件，这个命令会需要更多的时间来执行，但是会优化代码使得最终的可执行文件运行效率更高。同时这个可执行文件在目录 *target/release* 中。

