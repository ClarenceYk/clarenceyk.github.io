---
title: Rust Journey - Part2
tags:
  - Rust
date: 2025-09-01 17:15:00
updated: 2025-09-01 17:15:00
---


对于一门编程语言其需要具备根据*条件状态*来执行不同的代码或者根据*条件状态*来决定是否继续重复执行代码。通常这样的结构就是 `if` 表达式和 `loops`。

<!-- more -->

## if 表达式

`if` 表达式允许你基于条件状态来执行不同的代码。

```rust
fn main() {
    let number = 3;

    if number < 5 {
        println!("condition was true");
    } else {
        println!("condition was false");
    }
}
```

`if` 表达式使用 `if` 关键字开头，接着一个条件状态。在这个例子中，条件状态检查 `number` 变量是否小于 5。然后我们把条件为 `true` 时执行的代码块用花括号括起来放在后面，这样的一块代码在 rust 中被称为 `arms`，就像 `match` 表达式中一样。

我们还能在后面继续写一个 `else` 表达式，这个不是必须的。这个 arm 是在条件为 `false` 时执行的。如果我们不提供 `else` 表达式，那么当条件为 `false` 时程序就会直接跳过整段 `if` 代码块去执行后面的代码。

执行这段程序：

```
$ cargo run
   Compiling branches v0.1.0 (file:///projects/branches)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 0.31s
     Running `target/debug/branches`
condition was true
```

如果把 `number` 改为 `7`，再执行程序，可以得到：

```
$ cargo run
   Compiling branches v0.1.0 (file:///projects/branches)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 0.31s
     Running `target/debug/branches`
condition was false
```

条件状态的类型必须是 `bool`，否则会报错。例如：

```rust
fn main() {
    let number = 3;

    if number {
        println!("number was three");
    }
}
```

这里条件状态的值为 `3`, Rust 会报错：

```
$ cargo run
   Compiling branches v0.1.0 (file:///projects/branches)
error[E0308]: mismatched types
 --> src/main.rs:4:8
  |
4 |     if number {
  |        ^^^^^^ expected `bool`, found integer

For more information about this error, try `rustc --explain E0308`.
error: could not compile `branches` (bin "branches") due to 1 previous error
```

报错信息指出 `if` 表达式期望传入的值是 `bool` 而不是一个整型。Rust 不会像其他一些语言一样隐式地将非 bool 型的值转换为 bool 类型。

### 使用 `else if` 处理多个条件

我们还能加入 `else if` 表达式来处理更多的条件：

```rust
fn main() {
    let number = 6;

    if number % 4 == 0 {
        println!("number is divisible by 4");
    } else if number % 3 == 0 {
        println!("number is divisible by 3");
    } else if number % 2 == 0 {
        println!("number is divisible by 2");
    } else {
        println!("number is not divisible by 4, 3, or 2");
    }
}
```

这个程序中有 4 条路可以走，如果运行它：

```
$ cargo run
   Compiling branches v0.1.0 (file:///projects/branches)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 0.31s
     Running `target/debug/branches`
number is divisible by 3
```

需要注意的是 Rust 只会执行第一个条件为 `true` 的分支，如果剩下的分支还有条件为 `true` 的也不会执行。

太多的 `else if` 会导致你的代码变得杂乱，如果遇到这样的情况可能会需要重构一下代码，后面会介绍一个更厉害的结构 `match`。

### 在 `let` 语句中使用 `if`

因为 `if` 是表达式，所以我们可以将它写在 `let` 语句的右边来将产生的值赋给变量。

```rust
fn main() {
    let condition = true;
    let number = if condition { 5 } else { 6 };

    println!("The value of number is: {number}");
}
```

这里 `number` 变量的值绑定到 `if` 表达式产生的结果，执行这段代码：

```
$ cargo run
   Compiling branches v0.1.0 (file:///projects/branches)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 0.30s
     Running `target/debug/branches`
The value of number is: 5
```

前面提到过代码块的值就是代码块中最后一个表达式的值，数字本身也是表达式。在这个例子中整个 `if` 表达式的值是由执行哪段代码块而决定的，也就意味着 `if` arm 和 `else` arm 的值类型必须一致，否则会报错。

```rust
fn main() {
    let condition = true;

    let number = if condition { 5 } else { "six" };

    println!("The value of number is: {number}");
}
```

当我们编译这段代码，会报错：

```
$ cargo run
   Compiling branches v0.1.0 (file:///projects/branches)
error[E0308]: `if` and `else` have incompatible types
 --> src/main.rs:4:44
  |
4 |     let number = if condition { 5 } else { "six" };
  |                                 -          ^^^^^ expected integer, found `&str`
  |                                 |
  |                                 expected because of this

For more information about this error, try `rustc --explain E0308`.
error: could not compile `branches` (bin "branches") due to 1 previous error
```

类型不一致会报错，因为编译器没法提前知道 `number` 的类型，进而不能对 `number` 的使用做检查了。

## 使用循环重复执行代码

程序当中经常遇到需要将一段代码反复执行。对于这样的任务，Rust 提供了多种 `loops`，它们都能实现将一段代码执行到末尾然后立即返回到开头继续执行。

### 使用 `loop` 循环

`loop` 关键字让 Rust 一直执行一段代码，直到我们显式地让其停下来。

```rust
fn main() {
    loop {
        println!("again!");
    }
}
```

这段代码执行时，我们会看到 `again!` 一直打印到屏幕上直到我们手动地将其关闭。

```
$ cargo run
   Compiling loops v0.1.0 (file:///projects/loops)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 0.08s
     Running `target/debug/loops`
again!
again!
again!
again!
^Cagain!
```

`^C` 表示我们按下了 `ctrl`-`c`。你可能看到的 `^C` 出现在不同的位置，这取决于按下 `ctrl`-`c` 的时机。

Rust 另外提供了 2 个关键字可以在循环体中使用，一个是 `break`，另一个是 `continue`：

- `break` 会终止整个循环
- `continue` 会终这次循环并开始下一次循环

### 从循环带值返回

通过在 `break` 后面带上一个值，这个值就会是整个循环表达式的值。

```rust
fn main() {
    let mut counter = 0;

    let result = loop {
        counter += 1;

        if counter == 10 {
            break counter * 2;
        }
    };

    println!("The result is {result}");
}
```

在循环体中使用 `return` 也能终止循环，但是需要注意的是 `return` 不止终止了循环还将整个函数的执行也终止了。

> 注意：`break` 之后的分号也是可选的。`break` 和 `return` 非常类似，两者都能选择性地跟上一个表达式作为参数。在 `break` 和 `return` 语句执行之后的代码都是不执行的，所以 Rust 编译器会将 `break` 和 `return` 表达式的值置为单元值 `()`。

### 在多重循环中使用循环标签消除模糊性

在循环内部再使用循环时，`break` 和 `continue` 会作用在其出现的最内层循环上。如果我们给循环加上一个 `循环标签` 再使用 `break` 或者 `continue` 并带上这个标签，那么就可以让这 2 个关键字作用在指定的循环上而不是最内层循环。`循环` 标签必须用单引号开头。

```rust
fn main() {
    let mut count = 0;
    'counting_up: loop {
        println!("count = {count}");
        let mut remaining = 10;

        loop {
            println!("remaining = {remaining}");
            if remaining == 9 {
                break;
            }
            if count == 2 {
                break 'counting_up;
            }
            remaining -= 1;
        }

        count += 1;
    }
    println!("End count = {count}");
}
```

这段代码中的第一个 `break` 没有指定循环标签，所以它终止内层循环，第二个 `break` 带上了循环标签，所以它终止外层循环。

```
$ cargo run
   Compiling loops v0.1.0 (file:///projects/loops)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 0.58s
     Running `target/debug/loops`
count = 0
remaining = 10
remaining = 9
count = 1
remaining = 10
remaining = 9
count = 2
remaining = 10
End count = 2
```

### 使用 `while` 条件循环

我们写程序时经常会出现需要在循环体中判断一个条件，如果条件成立则继续执行循环，否则结束循环。我们可以使用 `loop`、`if`、`else` 和 `break` 来实现这样的逻辑。但是由于这个模式太过于场景，于是在 Rust 中就提供了一个内建的结构来实现。这就是 `while` 循环。

```rust
fn main() {
    let mut number = 3;

    while number != 0 {
        println!("{number}!");

        number -= 1;
    }

    println!("LIFTOFF!!!");
}
```

使用 `while` 循环就消除了一些不必要的 `if` 和 `else` 使得程序更加的清晰。

### 使用 `for` 循环遍历集合

使用 `while` 循环可以实现遍历一个集合中的每一个元素，例如数组。

```rust
fn main() {
    let a = [10, 20, 30, 40, 50];
    let mut index = 0;

    while index < 5 {
        println!("the value is: {}", a[index]);

        index += 1;
    }
}
```

这段代码中会每次将 `index` 加一作为索引并使用它去访问数组，直到索引超出数组有效范围：

```
$ cargo run
   Compiling loops v0.1.0 (file:///projects/loops)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 0.32s
     Running `target/debug/loops`
the value is: 10
the value is: 20
the value is: 30
the value is: 40
the value is: 50
```

但是，这样很容易出错，比如我们修改了数组长度，也需要同时修改循环的判断条件。一种更加简介的代替方式是使用 `for` 循环：

```rust
fn main() {
    let a = [10, 20, 30, 40, 50];

    for element in a {
        println!("the value is: {element}");
    }
}
```

结果和使用 `while` 循环是一样的，但是更不容易出错了。

