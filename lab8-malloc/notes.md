Guide
---

# 知识储备

  1. 内存分配机制

# 题目

  [官方文档](http://csapp.cs.cmu.edu/3e/malloclab.pdf)

任务为 理解多种动态内存分配方式并实现一种

# Solution

`git diff default:lab8-malloc finish:lab8-malloc`

首先看一遍README 知道每个文件都是干嘛用的 再看pdf

我们要改的只有`mm.c`,看看mm.h也没发现什么额外有用的信息

 * `mm_init` 比如初始化堆 如果没空间返回-1 否则返回0
 * `*mm_malloc (size_t size)` 就是分配内存了 返回指向的指针 分配>=size就行 **注意8字节对齐**
 * `mm_free (void *ptr)` 释放指向的内存 作者说保证是通过之前malloc 等拿到的 不会给你错误的位置
 * `*mm_realloc(void *ptr, size_t size)`重新分配 如果ptr==NULL 就和malloc一样的操作 如果size==0 就和`mm_free`一样的操作 其它情况你自己控制

pdf建议我们写一个`mm check`来作为中间检查(当然你可以任性不写...

我们可以用的函数
 * `void *mem_sbrk(int incr)` 用来在heap上开incr字节的空间 返回指向首部第一个字节的指针
 * `void *mem_heap_lo(void)` 返回指向heap头的指针
 * `void *mem_heap_hi(void)` 返回指向heap尾的指针
 * `size_t mem_heapsize(void)` 返回当前heap的大小(字节单位)
 * `size_t mem_pagesize(void)` 返回当前系统的页的大小(字节单位 4K on Linux systems).

然后讲了几个**规矩**  再看看Hints 作者说`gprof`对性能优化会有用
 * gprof是GNU profiler工具。可以显示程序运行的“flat profile”，包括每个函数的调用次数，每个函数消耗的处理器时间。

下面看看作者给的初始代码
 * init 啥都没干
 * malloc 直接sbrk申请
 * free 什么都没干
 * realloc 如果新申请的更大 就通过malloc新申请 再把数据复制过去 调用一下什么都没干的free

正确性就不说了,如果为了过测试做了一个不正确的，也不是这个lab的目的。我写过三种分配()，猜也猜得到，未曾拿到满分。

这个lab的目的在我看来是了解内存分配的一些方法,和数据结构的一同使用，在我的课程结构里是先学ICS 后学数据结构的。难度在于如何设计，设计完后实现的c代码难度较小。

关于文档，我推荐大家google搜索"malloc lab " 有很多关于如何管理的建议 比之前的lab能搜到的多多了 作为蒟蒻的我就不献丑了 [TODO还是仔细再写一篇攻略？可我感觉这是数据结构的事+一点点面向测试编码]

关于代码，可以参考[SJTU-SE](https://github.com/SJTU-SE/awesome-se) 里各位同学lab6(被老师调整过顺序)的实现[测试时间2017年01月29日04:25:38]
 * Azard tcbbd silencious 97 似乎都带有奇怪的数字:-) 
 * codeworm96 ComMouse 96 也有神奇数字:-)
 * wizardforcel 90(`seg_mm.c`) 89(`exp_mm.c`) 55(`imp_mm.c`) 
 * abucraft zackszhu mycspring 85
 * 测过但是爆了pwwpche gaocegege NoteBookie YeXiaoRain

96 97 分的代码里都有神奇数字，各位可以试一试把神奇数字去掉会怎样(当然也许真实的管理也有类似的神奇数字)，或制造更多的数据来测试各种的代码分配管理更好

测试
 * `./mdriver -t ./traces/` 吐槽:讲道理我们老师还很友好的提供了`trace`和config.h的TRACEDIR值修改

重写[TODO]如果有爸爸看到这里句 说明我还没有重写这个lab 可以发邮件`562178188@qq.com`提醒我重写这个`mm.c`

