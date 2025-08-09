---
title: Rust Journey - Part1
tags:
  - Rust
date: 2025-08-03 19:35:00
updated: 2025-08-09 15:35:00
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

## 数据类型

在 Rust 中的每一个值都有一个特定的类型，这样 Rust 就知道如果处理这个值。在 Rust 中，基本数据类型可以分为 2 种：标量和复合类型。

Rust 是一门静态类型语言，这表示必须在编译阶段就知道所有变量的类型。Rust 编译器同时也很强大，可以在编译的时候通过变量的值以及我们r如何使用变量来推断其类型。当遇到一个值可能会出现多种类型时，例如，将字符串转换成数字时，得到的结果可能是很多类型（i32，u32等），此时我们就必须显示标记变量类型：

```rust
let guess: u32 = "42".parse().expect("Not a number!");
```

如果没有标记类型，那么在编译的时候就会报错，提示我们编译器不知道具体要使用哪种类型：

```
$ cargo build
   Compiling no_type_annotations v0.1.0 (file:///projects/no_type_annotations)
error[E0284]: type annotations needed
 --> src/main.rs:2:9
  |
2 |     let guess = "42".parse().expect("Not a number!");
  |         ^^^^^        ----- type must be known at this point
  |
  = note: cannot satisfy `<_ as FromStr>::Err == _`
help: consider giving `guess` an explicit type
  |
2 |     let guess: /* Type */ = "42".parse().expect("Not a number!");
  |              ++++++++++++

For more information about this error, try `rustc --explain E0284`.
error: could not compile `no_type_annotations` (bin "no_type_annotations") due to 1 previous error
```

### 标量类型

标量类型用来表示单个值。在 Rust 中有 4 种基本的标量类型：整型、浮点型、布尔型以及字符。

#### 整型

整型是不带小数的数字。下表中列出了 Rust 中自带的所有整型类型：

| Length | Signed | Unsigned |
| - | - | - |
| 8-bit | i8 | u8 |
| 16-bit | i16 | u16 |
| 32-bit | i32 | u32 |
| 64-bit | i64 | u64 |
| 128-bit | i128 | u128 |
| architecture dependent | isize | usize |

其中，`isize`、`usize` 类型依赖于程序运行的电脑上的 CPU 架构。例如 CPU 是 64bit 的那么它们的大小就是 64 位。

在程序中书写整型可以使用以下格式中的任一种：

| Number literals | Example |
| - | - |
| Decimal | 98_222 |
| Hex | 0xff |
| Octal | 0o77 |
| Binary | 0b1111_0000 |
| Byte(`u8` only) | b'A' |

还能在数字字面值的后面加上后缀，例如 `57u8`，去指定类型。

#### 浮点型

Rust 也有 2 中基本的浮点数类型：`f32` 和 `f64`。

```rust
fn main() {
    let x = 2.0; // f64

    let y: f32 = 3.0; // f32
}
```

浮点数遵循 IEEE-754 标准。

#### 数字运算

Rust 支持所有基本的数学运行操作：加减乘除以及取余。整数除法会丢掉商的小数部分。下面是一个做数学运算的例子：

```rust
fn main() {
    // addition
    let sum = 5 + 10;

    // subtraction
    let difference = 95.5 - 4.3;

    // multiplication
    let product = 4 * 30;

    // division
    let quotient = 56.7 / 32.2;
    let truncated = -5 / 3; // Results in -1

    // remainder
    let remainder = 43 % 5;
}
```

#### 布尔型

和大多数其他编程语言一样，在 Rust 中布尔类型有 2 种可能的值：`true` 以及 `false`。布尔值在大小上占用一个字节。在 Rust 中用 `bool` 表示布尔型：

```rust
fn main() {
    let t = true;

    let f: bool = false; // with explicit type annotation
}
```

主要使用布尔型值的场景是条件判断，比如在 `if` 表达式中使用。

#### 字符类型

Rust 的字符类型是语言中最基本的字母类型。以下是一些字符类型变量的示例：

```rust
fn main() {
    let c = 'z';
    let z: char = 'ℤ'; // with explicit type annotation
    let heart_eyed_cat = '😻';
}
```

这里，我们使用单引号指定 `char` 字面量，而字符串字面量则使用双引号。Rust 的 `char` 类型大小为 4 个字节，表示 Unicode 标量值，这意味着它不仅可以表示 ASCII 码，还可以表示很多其他字符。在 Rust 中，重音字母、中文、日文和韩文字符、表情符号和零宽度空格都是有效的字符值。Unicode 标量值的范围为 `U+0000` 至 `U+D7FF` 和 `U+E000` 至 `U+10FFFF`（含）。不过，“字符”在 Unicode 中并不是一个真正的概念，所以我们在直觉上对“字符”的理解可能与 Rust 中的字符不一致。我们将在后面详细讨论这个问题。

### 复合类型

复合类型可以将多个值组合在一个类型中。Rust 中有 2 个基本的复合类型：元组和数组（tuple and array）。

#### 元组

元组用于将不同类型的值放在一起组合成一个类型。元组具有固定长度，也就是一旦定义它的大小就不能改变。

定义元组可以用以下方式：

```rust
fn main() {
    let tup: (i32, f64, u8) = (500, 6.4, 1);
}
```

将不同类型在圆括号中用逗号隔开。类型可以相同也可以不同。

上面定义的 `tup` 变量绑定到整个元组，因为元组被视为单个元素。如果想获取元组中的单个值，我们可以使用模式匹配来取消元组组合：

```rust
fn main() {
    let tup = (500, 6.4, 1);

    let (x, y, z) = tup;

    println!("The value of y is: {y}");
}
```

我们还能直接通过 `.` 后面跟上一个索引值来访问对应的值：

```rust
fn main() {
    let x: (i32, f64, u8) = (500, 6.4, 1);

    let five_hundred = x.0;

    let six_point_four = x.1;

    let one = x.2;
}
```

不包含任何值的元组具有特别的含义，称为 *unit*。它的值以及类型都写作 `()` 用来表示一个空值或者空回返类型。任何表达式如果不返回其他任何值那么它就隐式返回 unit 值。

我们还可以单独修改可变元组中的一个值：

```rust
fn main() {
    let mut x: (i32, i32) = (1, 2);
    x.0 = 0;
    x.1 += 5;
}
```

#### 数组

将多个值放在一个集合中的另外一种方式就是数组。和元组不同的是，在数组中的每一个元素的类型必须相同；和部分其他编程语言中的数组不同的是 Rust 中的数组是固定长度的。

数组的值用在方括号中以逗号将各个元素列出的方式定义：

```rust
fn main() {
    let a = [1, 2, 3, 4, 5];
}
```

当我们遇到以下场景的时候数组就很有用：

- 将一组数据分配在栈上而不是堆上
- 在使用一组数据时希望它的元素个数是固定的

我们这样来表达数组的类型：方括号里面先写元素类型，紧接着跟上一个分号，最后写上元素个数：

```rust
let a: [i32; 5] = [1, 2, 3, 4, 5];
```

初始化一个每个元素值都是相同的数组可以用一个简便写法：

```rust
let a = [3; 5];
```

这样写的效果和 `let a = [3, 3, 3, 3, 3];` 是一样的。

##### 数组元素的访问

和绝大多数编程语言一致，Rust 中数组元素的访问也用方括号里写上元素索引来表示：

```rust
fn main() {
    let a = [1, 2, 3, 4, 5];

    let first = a[0];
    let second = a[1];
}
```

##### 数组的非法访问

当我们越界访问数组的元素在 Rust 中会发生什么呢？下面这个程序示范了这种情况：

```rust
use std::io;

fn main() {
    let a = [1, 2, 3, 4, 5];

    println!("Please enter an array index.");

    let mut index = String::new();

    io::stdin()
        .read_line(&mut index)
        .expect("Failed to read line");

    let index: usize = index
        .trim()
        .parse()
        .expect("Index entered was not a number");

    let element = a[index];

    println!("The value of the element at index {index} is: {element}");
}
```

这段代码能编译通过，但是在执行程序时，如果我们提供输入为 `10` 的时候就会发生：

```
thread 'main' panicked at src/main.rs:19:19:
index out of bounds: the len is 5 but the index is 10
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace
```

程序在索引操作中使用无效值时出现运行时错误。程序带着错误信息退出，并且没有执行最后的 `println!` 当你尝试使用索引访问元素时，Rust 会检查你指定的索引是否小于数组长度。如果索引大于或等于数组长度，Rust 就会触发 panic 机制。这种检查必须在运行时进行，因为编译器不可能提前知道用户稍后运行代码时会输入什么值。

这是 Rust 内存安全原则发挥作用的一个例子。在许多底层语言中，这种检查是不存在的，当你提供了一个不正确的索引时，无效的内存就会被访问。而 Rust 会立即退出，而不是允许访问内存并继续，从而防止出现这种错误。

## 函数

to be continued...

