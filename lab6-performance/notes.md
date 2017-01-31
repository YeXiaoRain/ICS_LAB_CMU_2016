Guide
---

# 知识储备

  1. cache的知识 和上一个lab相似

# 题目

  [官方文档](http://csapp.cs.cmu.edu/3e/perflab.pdf)

  * 代码优化 矩阵旋转
  * 代码优化 矩阵模糊

# Solution

`git diff default:lab6-performance finish:lab6-performance`

正方形的图片(三元(红绿蓝)矩阵)旋转 光滑 模糊 大小为NxN(N=32;64;128;256;512)

旋转如pdf上图 分两个操作 转置+交换行

光滑如pdf上表达式 取每个点为 它+它相邻点(八向) 的均值

我们只用修改`kernels.c`

测试方法`make driver;./driver -t` 参数`t`表示 不检查组名 先看看默认的效果

```
./driver -t
Rotate: Version = naive_rotate: Naive baseline implementation:
Dim           64     128     256     512     1024     Mean
Your CPEs     3.0    4.2     5.8     9.8     20.4
Baseline CPEs 14.7   40.1    46.4    65.9    94.5
Speedup       4.9    9.6     8.0     6.7     4.6     6.5

Rotate: Version = rotate: Current working version:
Dim           64     128     256     512     1024     Mean
Your CPEs     2.8    3.9     5.8     9.7     20.6
Baseline CPEs 14.7   40.1    46.4    65.9    94.5
Speedup       5.3    10.3    8.0     6.8     4.6     6.7

Smooth: Version = smooth: Current working version:
Dim           32     64      128     256     512     Mean
Your CPEs     56.0   59.9    59.9    56.2    57.4
Baseline CPEs 695.0  698.0   702.0   717.0   722.0
Speedup       12.4   11.7    11.7    12.8    12.6    12.2

Smooth: Version = naive_smooth: Naive baseline implementation:
Dim           32     64      128     256     512     Mean
Your CPEs     56.1   56.1    56.2    56.2    57.2
Baseline CPEs 695.0  698.0   702.0   717.0   722.0
Speedup       12.4   12.4    12.5    12.8    12.6    12.5

Summary of Your Best Scores:
  Rotate: 6.7 (rotate: Current working version)
  Smooth: 12.5 (naive_smooth: Naive baseline implementation)
```

发现 默认的naive并不是1.0 [为什么 求教!!!]而真正的加速比应为 我们实现的除naive的才是真正的加速比

看看作者代码中已经提供给我们的

数据结构

```
typedef struct {
  unsigned short red; /* R value */
  unsigned short green; /* G value */
  unsigned short blue; /* B value */
} pixel;
```

通过行列`i,j`获取在I中的Index:`#define RIDX(i,j,n) ((i)*(n)+(j))`

根据pdf 再看看`kernel.c`里的
 * `naive_rotate` 我们的任务就是重写这个函数,用code motion, loop unrolling and blocking技术让它运行得更快
 * `naive_smooth` 任务是优化smooth(可以优化avg 也可以不完全不用avg)

之后的pdf是对其它部分代码介绍 我们能修改的只有`kernel.c`

## Rotate

我们有的加速知识
 * 电脑里 速度差异 `寄存器>cache>memory`
 * 循环展开 在cache lab里用过
 * 代码级pipeline
 * 根据大神说 **写**的缓存优化 比 **读**的缓存优化 效率高

说是旋转90° 细化到每一步

 * 读src第一列 写入dst倒数第一行
 * 读src第二列 写入dst倒数第二行
 * ...
 * 读src第n列 写入dst第一行

```
AAAAAAAA         AB...Z
BBBBBBBB         AB...Z

...        =>    ...

ZZZZZZZZ         AB...Z
```

首先假设我们的分块参数为`blocksize` 把 上面的操作分组为(不理解为什么要分块的见上一个lab的分块映射讲解)

 * 把`src`中`0~blocksize-1`列写到`dst`中`0~blocksize-1`行
 * 把`src`中`blocksize~2*blocksize-1`列写到`dst`中`blocksize~2*blocksize-1`行
 * ...

这样每步的操作是`src`中 blocksize列 n行的块 也就是 `dst`中 n列 blocksize行的块,一下操作只考虑一个`blocksize*n`的操作

根据要让写的cache尽量维持写的局部性

对于一个我们分的块行(blocksize行) `ABCD..` 转换为列

```
ABCD...Z      Z .... @
.             .
.        =>   .
.             C
.             B
#      @      A .... #
src           dst
```

根据内存机制`dst` 中A的行 和 B的行 可能被分配到同一个cache位置(因为这里测试的n的值都是32的倍数,不理解的见上一个lab的分块)

所以 我们的做法顺序是
 * dst的A的倒数第一行 `src[0~blocksize-1][0] -> dst[n-1][0~blocksize-1]`
 * dst的A的倒数第二行 `src[0~blocksize-1][1] -> dst[n-2][0~blocksize-1]`
 * ...
 * 一直到dst中Z的第一行

对于这里每一步 对应转换表达式

`dst[RIDX(n-1-j, i+stepi, n)] = src[RIDX(i+stepi, j, n)]`

然后用循环展开得到代码 具体实现见我的代码

测试`15.0/6.5 = 2.3076923076923075`

```
Rotate: Version = naive_rotate: Naive baseline implementation:
Dim           64     128     256     512     1024     Mean
Your CPEs     3.0    4.1     5.8     9.7     20.5
Baseline CPEs 14.7   40.1    46.4    65.9    94.5
Speedup       4.9    9.8     8.0     6.8     4.6      6.5

Rotate: Version = rotate: Current working version:
Dim           64     128     256     512     1024     Mean
Your CPEs     2.1    2.2     2.3     3.8     5.6
Baseline CPEs 14.7   40.1    46.4    65.9    94.5
Speedup       6.9    18.6    20.5    17.3    16.7     15.0
```

注:分数一直在变 **我的电脑上**测试发现block设为`16`和`32`的结果相差不大 代码中用的16

## smooth

 对每个点读一圈周围的和 再除个数

 首先 做分类(除4的(四个角) 除6的(边) 除9的(剩下)) 减少个数计算的过程 这样访问次数不变 每次操作减少

 然后 好像就已经 优化了3倍多了 见代码

 测试`37.7/11.9=3.1680672268907566`

```
Smooth: Version = smooth: Current working version:
Dim           32     64      128     256    512      Mean
Your CPEs     16.9   19.8    18.9    18.9   19.3
Baseline CPEs 695.0  698.0   702.0   717.0  722.0
Speedup       41.1   35.2    37.2    37.9   37.3     37.7

Smooth: Version = naive_smooth: Naive baseline implementation:
Dim           32     64      128     256    512      Mean
Your CPEs     59.9   59.9    60.0    57.2   58.9
Baseline CPEs 695.0  698.0   702.0   717.0  722.0
Speedup       11.6   11.7    11.7    12.5   12.3     11.9
```

据说除法也能用黑科技优化[博文](http://www.cppblog.com/huyutian/articles/124742.html)

## =。=

如果对最后的得分要求不高的话 这大概是个很水的lab 知道方法后基本以体力活为主

我们这届的lab里也没有`_(:з」∠)_`估计之后也不会有
