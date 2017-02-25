Guide
---

# 知识储备

  1. cache的作用以及运作方式(不同的映射方式 剔除方式等)
  2. C代码基础

# 题目

  [官方文档](http://csapp.cs.cmu.edu/3e/cachelab.pdf)

  * Part A 做一个cache模拟器
  * Part B 利用cache知识优化代码 尽量少的产生miss

# Solution

`git diff default:lab5-cache finish:lab5-cache`

安装`valgrind`,Part B会用

    sudo apt-get install valgrind

**注意** 关于每个Part都有三个部分 被作者分开在pdf 的前中后...我做到一半才发现还有相关介绍

## Part A

**任务**:在`csim.c`中写一个cache模拟器(模拟的是LRU 剔除 映射机制 不用保存数据 也没有提供数据)

作者已经提供了一个可执行的LRU的的`csim-ref`,细节使用方式见pdf

非常变态的`Your csim.c file must compile without warnings in order to receive credit` 禁止代码有`warning`

你的模拟器需要使用`malloc`,在你main的最后要调一个`printSummary(hit_count, miss_count, eviction_count);`函数

**测试**:用pdf上的一步一步测，或者`./test-csim`一次测完整个Part A

这么说是要改代码了哦,想想还有点小激动,然后我打开`csim.c`一看,哎卧槽,有且只有一个main 几个意思呢???

根据下面的建议使用`getopt.h` 这也就是用来读入分析参数的,感谢不用自己写解析了,由此我们的读入也就写好了

 * s set的索引位数   也就是index范围
 * E 每个set的行数   也就是多路cache的路数
 * b block大小的位数 也就是每一个块的大小offset的上限

每一条对内存访问的记录格式是 [空格]操作符 地址,大小。

 * I 指令载入前面没有空格,不算在内存访问中
 * M 表示数据修改，需要一次载入 + 一次存储，也就是相当于两次访问 (hit/miss)+hit
 * S 表示数据存储 (hit/miss)
 * L 表示数据载入 (hit/miss)

其共性都会根据当前的cache引起(hit/miss)

根据cache的映射知识

```
访问地址
addr = | tag |  index  | offset |
       |     |  s bits | b bits |

划分以后

对应到cache的index组里的offset位置
```

因为longlong 是64位 这里也就用到了这个来"有技巧"的创造有效位,因为tag的长度小于64,所以我用一个tag不可能达到的值来表示无效

`#define INVALID      (1<<(sysbits-s-b))`

我的代码里的LRU是[0~most recent] 从最远到最新的

具体以上实现见我的代码

测试 `make clean && make && ./test-csim` 27分则满了
```
                        Your simulator     Reference simulator
Points (s,E,b)    Hits  Misses  Evicts    Hits  Misses  Evicts
     3 (1,1,1)       9       8       6       9       8       6  traces/yi2.trace
     3 (4,2,4)       4       5       2       4       5       2  traces/yi.trace
     3 (2,1,4)       2       3       1       2       3       1  traces/dave.trace
     3 (2,1,3)     167      71      67     167      71      67  traces/trans.trace
     3 (2,2,3)     201      37      29     201      37      29  traces/trans.trace
     3 (2,4,3)     212      26      10     212      26      10  traces/trans.trace
     3 (5,1,5)     231       7       0     231       7       0  traces/trans.trace
     6 (5,1,5)  265189   21775   21743  265189   21775   21743  traces/long.trace
    27

TEST_CSIM_RESULTS=27
```

这个Part A让我们具体的实现了 直接+多路映射 的cache

**注意** 这里操作中的length是一个没有用的参数[测csim-ref的结果应该是没用] 是刚好出题者让每次的访问的length都在一个block内,还是生成的log一次只会访问一个block内的? 如果有length会导致跨index的话,还应该在模拟访问时再加一层`for(addr~addr+length-1)` (持有疑问)

**以及** 我的代码实现的比较按照我个人代码风格,更建议使用struct来模拟valid tag lru

**再以及** 针对写lab就直接全局变量了,更建议使用局部+传参

**思考**
 1. 如果我说运行的电脑是72位的 或者变成传参数(<=128) 代码要怎么改?
 2. 对于 注意 中的有什么具体的办法可以测试 length 在作者给的demo simulator 中是否有用?

## Part B

修改`trans.c`矩阵转置 把矩阵A转置为B 使用刚刚的cache知识 尽量少的miss 限定了临时变量个数等 具体见pdf (总之限定的意思就是 你别搞额外的东西来降低miss 要的就是cache映射知识),A不能写,B想怎么玩怎么玩

测三个 32x32, 64x64 , 61x67 这里写特判三种分别处理就好

`1KB direct mapped cache with a block size of 32 bytes`

`1 int = 4 bytes 所以 1 block = 32 bytes = 8 int`

`1K = 32*8*4`

**`32x32`**

```
00000000 11111111 22222222 33333333
↓
28....28                   31....31

00000000 11111111 22222222 33333333
↓
28....28                   31....31

00000000 11111111 22222222 33333333
↓
28....28                   31....31

00000000 11111111 22222222 33333333
↓
28....28                   31....31
```
非对角线上复制 一定不冲突并用且只用一次miss

对角线上会冲突 可以通过借用降低miss 但提高访问次数 如对角线上的8x8块
 1. 把A[0->28]左上角这块 移到B[3->31]右下角 再移到 B[0->28]
 2. 把   A[1->29]   这块 移到B[3->31]右下角 再移到    B[1->29]
 3. 把      A[2->30]这块 移到B[3->31]右下角 再移到       B[2->30]
 4. 最后把     A[3->31]  移到B[3->31]右下角 只有这里会有额外的miss

当然我没这样 我发现 分完8x8 再在对角线上小优化就已经满了 具体见代码

**`64x64`**

```
00000000 ... 77777777
↓
24....24 ... 31....31
00000000 ... 77777777
↓
24....24 ... 31....31

.            .
.            .
.            .

00000000 ... 77777777
↓
24....24 ... 31....31
00000000 ... 77777777
↓
24....24 ... 31....31
```

可以看到现在的8x8块已经和32的不一样了 这次对角线处理基本和以上一样,再分化为4个4x4

而非对角线的

```
AB   变   AC
CD   为   BD
```
注意左 AB为一块 CD为一块 右AC为一块 BD为一块
 1. 左A 转置到 右A 无额外miss
 2. 左B 转置到 右C 无额外miss
 3. 右C 第1行 放入4个临时变量 左C第1列写入右C第1行 4个临时变量写入右B第1行 无额外miss
 4. 右C 第2行 放入4个临时变量 左C第2列写入右C第2行 4个临时变量写入右B第2行 无额外miss
 5. 右C 第3行 放入4个临时变量 左C第3列写入右C第3行 4个临时变量写入右B第3行 无额外miss
 6. 右C 第4行 放入4个临时变量 左C第4列写入右C第4行 4个临时变量写入右B第4行 无额外miss
 7. 左D 转置到 右D [可融入上面3-6]

再一次实现非对角线块转置无必要miss之外的miss 具体见代码

**`61x67`** 很明显都不是8的倍数,这个靠手感,试了几个数发现17x17的划分转移可以满分..套了4个for 具体见代码

测试

```
>./driver.py
Part B: Testing transpose function
Running ./test-trans -M 32 -N 32
Running ./test-trans -M 64 -N 64
Running ./test-trans -M 61 -N 67

Cache Lab summary:
                        Points   Max pts      Misses
Csim correctness          27.0        27
Trans perf 32x32           8.0         8         287
Trans perf 64x64           8.0         8        1203
Trans perf 61x67          10.0        10        1950
          Total points    53.0        53
```

