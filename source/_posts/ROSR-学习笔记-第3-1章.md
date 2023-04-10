---
title: ROSR 学习笔记 - 第3.1章
date: 2022-11-21 13:42:39
updated: 2022-11-21 13:42:39
tags:
- RISC-V
- OS
- Rust
---

ROSR 是『RISC-V OS using Rust』的缩写，是由 [Stephen Marz](https://osblog.stephenmarz.com/) 在他的系列博客中提供的操作系统开发教程。

本章描述系统堆内存管理。

前面章节已经描述过我们通过 QEMU 给整个系统提供了 128M 字节内存空间，ELF 文件加载到内存中后除开代码、全局变量、栈等占用的内存之外，其余部分我们都分配了堆。所以堆的部分就由操作系统来管理分配。

管理系统堆空间时，分为 3 个部分：

1. **页分配**
2. 字节分配
3. 编程内存管理单元

本 3.1 节主要描述**页分配**。

<!-- more -->

按页分配表示以页为粒度来分配内存空间，RISC-V 和大多数架构一样最小页空间占用 4096 字节。按页分配的方法有很多，这里采用描述符分配方式。

在进入正题之前，我们先来看几个重要的参数：

```plaintext
PROVIDE(_heap_start = _stack_end);
PROVIDE(_heap_size = _memory_end - _heap_start);
```

linker script 提供了 2 个重要符号：堆起始地址（`_heap_start`）和堆大小（`_heap_size`），这些符号又通过 `mem.S` 文件作为全局常量传入 rust：

```
.global HEAP_START
HEAP_START: .dword _heap_start

.global HEAP_SIZE
HEAP_SIZE: .dword _heap_size
```

在 `page.rs` 的 `init` 函数中：

```rust
let num_pages = HEAP_SIZE / PAGE_SIZE;

// ...

ALLOC_START = align_val(
                    HEAP_START
                    + num_pages * size_of::<Page,>(),
                    PAGE_ORDER,
);
```

第一行算出堆内存总共的页数，最后一行算出用于分配的堆内存起始地址。

## 描述符分配法

每次分配都按照连续页分配，并且只存储首页地址。为了达到此目标必须每页内存都有一个字节大小的描述符。此描述符包含 2 种标志：1）此页是否被分配；2）是否为连续分配内存的最后一页。

由此定义如下数据结构：

```rust
#[repr(u8)]
pub enum PageBits {
    Empty = 0,
    Taken = 1 << 0,
    Last = 1 << 1,
}

pub struct Page {
    flags: u8,
}
```

### 分配内存页

```rust
pub fn alloc(pages: usize) -> *mut u8 {
    assert!(pages > 0);
    unsafe {
        let num_pages = HEAP_SIZE / PAGE_SIZE;
        let ptr = HEAP_START as *mut Page;
        for i in 0..num_pages - pages {
            let mut found = false;
            if (*ptr.add(i)).is_free() {
                found = true;
                for j in i..i + pages {
                    if (*ptr.add(j)).is_taken() {
                        found = false;
                        break;
                    }
                }
            }
            if found {
                for k in i..i + pages - 1 {
                    (*ptr.add(k)).set_flag(PageBits::Taken);
                }
                (*ptr.add(i+pages-1)).set_flag(PageBits::Taken);
                (*ptr.add(i+pages-1)).set_flag(PageBits::Last);
                return (ALLOC_START + PAGE_SIZE * i)
                       as *mut u8;
            }
        }
    }

    null_mut()
}
```

如上为内存页分配函数，函数参数为需要的页数。

第 2 行：判断页数是否有效；
第 4 行：算出堆内存空间总页数；
第 5 行：将堆起始地址类型转换为可变页指针；
第 6～26 行：找出满足要求连续空闲页并分配；
第 8～16 行：在第 i 页为空闲的前提下看其后是否有满足所需页数的连续页；
第 17～25 行：如果找到了满足所需页数的连续空闲页，将除最后一页的内存页标记为占用，最后一页标记为占用以及尾，并返回页指针。

### 回收内存页

```rust
pub fn dealloc(ptr: *mut u8) {
    assert!(!ptr.is_null());
    unsafe {
        let addr =
            HEAP_START + (ptr as usize - ALLOC_START) / PAGE_SIZE;
        assert!(addr >= HEAP_START && addr < HEAP_START + HEAP_SIZE);
        let mut p = addr as *mut Page;
        while (*p).is_taken() && !(*p).is_last() {
            (*p).clear();
            p = p.add(1);
        }
        assert!(
                (*p).is_last() == true,
                "Possible double-free detected! (Not taken found \
                 before last)"
        );
        (*p).clear();
    }
}
```

如上为内存页回收函数，函数参数为需要回收的页指针。

第 2 行：判断页指针是否有效；
第 4～7 行：算出此页内存所对应的页描述符；
第 8～17 行：依次将所有分配的内存页对应的页描述符标记清零，表示设置为未占用。

