---
title: Rust Journey - Part1
tags:
  - Rust
date: 2025-08-03 19:35:00
updated: 2025-08-03 19:35:00
---


诸如在任何一门其他的编程语言中存在的变量、基础数据类型、函数、流程控制以及注释这些基础概念，在 Rust 语言中同样存在。这一篇中来介绍在 Rust 语境中这些概念的用法。

<!-- more -->

## 变量及可变性

### let 变量

默认情况下，Rust 中的变量是不可变的。听上去有点反直觉，但是这是 Rust 建议的做法。同时 Rust 也提供了方法让变量可变。

我们通过 `cargo new variables` 创建一个新项目来探索一下可变性的概念，在 *src/main.rs* 中写上如下代码：

```rust
fn main() {
    let x = 5;
    println!("The value of x is: {x}");
    x = 6;
    println!("The value of x is: {x}");
}
```

保存然后编译会发现有报错：

```
$ cargo run
   Compiling variables v0.1.0 (file:///projects/variables)
error[E0384]: cannot assign twice to immutable variable `x`
 --> src/main.rs:4:5
  |
2 |     let x = 5;
  |         - first assignment to `x`
3 |     println!("The value of x is: {x}");
4 |     x = 6;
  |     ^^^^^ cannot assign twice to immutable variable
  |
help: consider making this binding mutable
  |
2 |     let mut x = 5;
  |         +++

For more information about this error, try `rustc --explain E0384`.
error: could not compile `variables` (bin "variables") due to 1 previous error
```

错误信息当中，已经明确说明了：`cannot assign twice to immutable variable`，我们给一个不可变变量第二次赋值，所以报错。这里值得注意的是这个错误报在了编译期间，Rust 在编译的时候检查出了写的代码和我们的意图不一致，因为变量 *x* 是不可变的，但是我们却在给它第二次赋值。所以 Rust 保证的是如果我们希望一个变量是不可变的那么它一定不会被修改。

但是，变量可变同样对程序员很重要，因为写程序会方便一些。我们可以在定义变量的时候在变量名之前加一个关键字 `mut` 来表达意图：这个变量是可变的。

```rust
fn main() {
    let mut x = 5;
    println!("The value of x is: {x}");
    x = 6;
    println!("The value of x is: {x}");
}
```

运行程序：

```
$ cargo run
   Compiling variables v0.1.0 (file:///projects/variables)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 0.30s
     Running `target/debug/variables`
The value of x is: 5
The value of x is: 6
```

当希望将绑定在 `x` 上的值从 `5` 改为 `6` 就必须使用 `mut`。

### 常量

和不可变变量一样，常量绑定的值也不可修改的。但是，它们之间也有一些不同的地方。首先不能在常量前加 `mut`，常量永远都是不可变的；第二，定义常量用的是 `const` 而不是 `let`；第三，定义常量时必须同时指定数据类型；第四，常量可以定义在任意作用范围包括全局作用范围，而变量只能定义在函数内部；最后，常量只能用常量表达式来初始化，也就是定义常量的值必须是能够在编译阶段就决定的而不是在运行时。

定义常量的例子：

```rust
const THREE_HOURS_IN_SECONDS: u32 = 60 * 60 * 3;
```

这里，常量的名称是 `THREE_HOURS_IN_SECONDS`，其值为 60 乘以 60 乘以 3。Rust 中命名常量的习惯是用全大写字符和下划线。

在整个程序的运行期间，常量在其定义的作用范围中都是有效的。这样当我们需要在程序的多个部分都使用同一个值时，使用常量就会很方便，并切含义一致。

### 变量遮蔽

当定义后一个变量时使用前一个变量的名称，我们就说后一个变量遮蔽（shadow）了前一个变量。对于编译器来说在此之后它就只会看到第二个变量。那什么时候这个作用会消失呢？那就有 2 种情况，第一，第二个变量继续被第三个变量遮蔽；第二，第二个变量的作用域结束。

```rust
fn main() {
    let x = 5;

    let x = x + 1;

    {
        let x = x * 2;
        println!("The value of x in the inner scope is: {x}");
    }

    println!("The value of x is: {x}");
}
```

上面这段代码，我们首先绑定 `x` 到 `5` 上，然后重复用 `let x = ` 创建了一个新变量 `x`，此时的 `x` 遮蔽了之前的 `x`。同时绑定到值 `5 + 1` 也就是 `6`。在花括号内部，我们继续创建第三个新变量 `x`，绑定值到 `6 * 2` 也就是 `12`，同时遮蔽了第一和第二个 `x`，当花括号结束后，第三个 `x` 超出作用域，于是遮蔽失效，编译器就看到了第二个 `x`，其值就是 `6`。

运行这段代码，结果就是：

```rust
$ cargo run
   Compiling variables v0.1.0 (file:///projects/variables)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 0.31s
     Running `target/debug/variables`
The value of x in the inner scope is: 12
The value of x is: 6
```

和 `mut` 不同的是，如果我们意外给 `x` 重新赋值，而没有用 `let` 来遮蔽，那么编译器就会报错，因为 `x` 是不可修改的。另外一点是，因为我们实际上是创建一个新的变量 `x` 所以第二个 `x` 的类型是可以和第一个 `x` 不一样的，例如：

```rust
    let spaces = "   ";
    let spaces = spaces.len();
```

## 变量类型

to be continued...

