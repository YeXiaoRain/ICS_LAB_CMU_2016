# Guide

**Error:** `sys/cdefs.h`: 没有那个文件或目录 . Fix with`apt-get install libc6-dev-i386`

## 知识储备

  1. C语言基本知识(位运算 if/while)
  2. 整数的二进制表示，补码

## 题目

改写所有function

每个Expr表达式只能包含下面4种
  1. 整数 0 ~ 255 (0xFF),不能用更大的例如 0xffffffff.
  2. 函数变量为局部变量,不能用全局.
  3. 一元运算符 ! ~
  4. 二元运算符 & ^ | + << >>
  
禁止：
  1. 控制语句例如 if, do, while, for, switch, etc.
  2. 定义/使用 宏.
  3. 定义额外的函数.
  4. 调用函数.
  5. 使用其它运算符例如 &&, ||, -, or ?:
  6. 使用任何形式的 casting.
  7. 使用非int的数据结构例如数组,structs,unions.

假设运行的机器是 
  1. 2进制,32位整形.
  2. 算数右移.
  3. 当位移数大于数字长度时行为未知.

  使用 `./dlc -e bits.c` 测试合法性
  使用 `make btest && ./btest` 测试正确性

## Solution

  `git diff default:lab1-data/bits.c finish:lab1-data/bits.c`





