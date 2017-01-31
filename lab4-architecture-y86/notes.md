# Guide

## 知识储备

  1. 之前lab所学到的c,汇编,栈
  2. Y86 指令集
  3. 单周期 流水线 (结构 冲突 转发)
  4. ~~提供的 y86模拟器使用方法(sim)~~

## 题目

官方文档 [simguide.pdf](/simguide.pdf) [archlab.pdf](/archlab.pdf)

 * Part A 手工把C代码翻译为Y86代码
 * Part B SEQ中新增两条指令
 * Part C 通过优化流水线pipe-full.hcl和指令ncopy.ys来提高CPE

实现一个y86流水线的处理器,利用流水线知识优化程序性能

目的是为了理解代码和硬件之间的关系

## Solution

`git diff default:lab4-architecture-y86 finish:lab4-architecture-y86`

因为缺少相关tk 把`sim/Makefile`中的

    TKLIBS=-L/usr/lib -ltk -ltcl

    TKINC=-isystem /usr/include

`pipe/Makefile`中的

    GUIMODE=-DHAS_GUI

    TKLIBS=-L/usr/lib -ltk -ltcl

    TKINC=-isystem /usr/include

`seq/Makefile`中的

    GUIMODE=-DHAS_GUI

    TKLIBS=-L/usr/lib -ltk -ltcl

    TKINC=-isystem /usr/include

前面加上`#` 即注释掉

cannot find `-lfl`,cannot find `bison`

    sudo apt-get install flex,bison

执行`make clean;make`没有报错的话,说明相关依赖已经安装好,就可以开始做这个lab了

### Part A

在`sim/misc`里,依照IA32的栈框架和寄存器调用,用Y86指令实现`example.c`里的函数

写一个`sum.ys`即`example.c`里的`sum_list`,测试数据段在pdf上

代码被顺序执行先将栈设定好传出对应地址 并传入数据段起始位置 调用`sum_list`函数 执行完以后halt

```
.pos 0
init:
  irmovl  Stack,%esp
  irmovl  Stack,%ebp
  irmovl  ele1,%eax
  pushl   %eax
  call    sum_list
  halt

# Sample linked list
.align 4
ele1:
  .long  0x00a
  .long  ele2
ele2:
  .long  0x0b0
  .long  ele3
ele3:
  .long  0xc00
  .long  0

sum_list:

.pos  0x100
Stack:
```

然后翻译`sum_list`的代码即可

验证:`./yas sum.ys ;./yis sum.yo`

如果输出中函数返回值`%eax`是`0xcba`则说明正确

```
Stopped in 33 steps at PC = 0x19.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%eax:eax0x000000000x00000cba
%esp:esp0x000000000x000000fc
%ebp:ebp0x000000000x00000100
%esi:esi0x000000000x00000c00

Changes to memory:
0x00f4:0x00f40x000000000x00000100
0x00f8:0x00f80x000000000x00000019
0x00fc:0x00fc0x000000000x0000001c
```

---

写`rsum.ys` 栈初始化和第一次传参和`sum`一样,这里是递归需要注意利用`pushl,popl`保存局部变量

验证 `./yas rsum.ys ;./yis rsum.yo`关注`%eax`的值是否是`0xcba`

---

写`copy.ys` 栈初始化和之前一样

这里首先要用新的数据段,第二初次传参是三个参数,根据栈的压栈顺序应为`push len, push &dest, push &src`,当其它顺序的也能跑,但还请按照lab最开始的模仿IA32的要求

验证 `./yas copy.ys ;./yis copy.yo`

关注`%eax`的值是否是`0xcba` 通过异或得到的

关注是否有memory从`dst`的数据变为`src`的数据
```
Changes to memory:
0x0038:0x00380x00000111 0x0000000a
0x003c:0x003c0x00000222 0x000000b0
0x0040:0x00400x00000333 0x00000c00
```

以上均有说明指令正确

这个Part首先对之前lab的c代码和汇编(之前lab都是x86和x86-64的)的关系进行回顾，栈上的变化进行回顾，和对y86的指令熟悉

### Part B

在`sim/seq`里,扩展SEQ处理器支持两个新的指令

`iaddl` : |C,0|F,rB|V| ,把常数值V加到寄存器rB里

`leave` : |D,0| ,实现 `rrmoval %ebp,%esp;popl %ebp`

先做`iaddl`

分析指令各个阶段

```
Fetch
  icode:ifun<-M1[PC]  读入[C,0]
  rA:rB<-M1[PC+1]     读入[F,rB]
  valC<-M4[PC+2]      读入[V]
  valP<-PC+6          根据读入指令长度自动+6
Decode
  valB<-R[rB]         读取rB的值
Execute
  valE<-valC+valB     计算加法结果
  R[rB]<-valE         写回rB 这里没有访问内存 只有cpu和寄存器的事，以及看到`IIRMOVL`这种写入寄存器的都在Execute Stage,所以写回rB在Execute
Memory
  无内存操作
Program Counter Update
  PC<-valP            更新PC
```

下面开始看`seq-full.hcl`

首先看`Symbolic representation of Y86 Instruction Codes`这一块发现作者已经帮我们写好了

```
# Instruction code for iaddl instruction
intsig IIADDL'I_IADDL'
# Instruction code for leave instruction
intsig ILEAVE'I_LEAVE'
```

`Fetch Stage`根据上面分析 `IIADDL`是 合法指令 需要从指令读取寄存器号 需要立即数

 * 在`instr_valid,need_regids,need_valC` 中加上`IIADDL`

`Decode Stage`根据上面分析 `IIADDL` 不用`rA`  从`rB`读入,虽然这是decode的阶段,但这里还有dstE,dstM(因为目的未知在decode就能判断),根据上面的分析写回寄存器`rB`在Excute阶段

 * 在`srcB`的`rB` 中加上`IIADDL`
 * 在`dstE`的`rB` 中加上`IIADDL`

`Execute Stage`根据上面分析 我们将`valC`和`valB`相加,`alufun`这里因为`IIADDL`不属于`IOPL`所以直接是`1:ALUADD`不用改,这里计算会影响符号标志

 * `aluA`的`valC`中添加`IIADDL`
 * `aluB`的`valB`中添加`IIADDL`
 * `set_cc`中添加`IIADDL`

`Memory Stage` 虽然没事还是看一看 注意到除了mem操作还有 机器状态判断 `Stat`的改变 当然IIADDL不会影响

`Program Counter Update`正常加6也就是加valP即可

---

`leave`考虑替代后执行前后的差别也就是 esp=old ebp +4,ebp = M[old ebp]

```
Fetch
  icode:ifun<-M1[PC]  读入[C,0]
  valP<-PC+1          根据读入指令长度自动+1
Decode
  valA<-R[%ebp]       读取old ebp
  valB<-R[%ebp]       读取old ebp
Execute
  valE<-valB+4        计算将要的esp       这两步骤不需要内存
  R[%esp]<-valE       写入esp
Memory
  valM<-M4[valA]      valM=M[old ebp]
  R[%ebp]<-valM       ebp=valM=M[old ebp]
Program Counter Update
  PC<-valP            更新PC
```

`Fetch Stage`根据上面分析 `ILEAVE`是 合法指令 不需要从指令读入寄存器号(我们虽然用了ebp和esp寄存器但是这不是从指令读如的) 步需要立即数

 * 在`instr_valid` 中加上`ILEAVE`

`Decode Stage`根据上面分析 `ILEAVE` val=ebp,valB=ebp,根据上面的分析在Execute阶段写esp 在Memory阶段写ebp

 * 在`srcA`和`srcB`中加上 `icode in { ILEAVE } : REBP;`
 * 在`dstE`的`ESP` 中加上`ILEAVE`
 * 在`dstM`的`EBP` 中加上`ILEAVE`

`Execute Stage`根据上面分析 我们将`4`和`valB`相加

 * `aluA`的`4`中添加`ILEAVE`
 * `aluB`的`valB`中添加`ILEAVE`

`Memory Stage` 读以`ebp`为地址的memory 并把值放入ebp,写的是寄存器不是内存所以`mem_write`不用添加,我们没有读立即数 也就没有`mem_data`的事情

 * `mem_read`中添加`ILEAVE`
 * `mem_addr`的valA中添加`ILEAVE`

`Program Counter Update`正常加1也就是加valP即可

测试 注：如果出现文件未找到，可能是之前没有在sim目录下make

```
lab4-architecture-y86/sim/seq > make clean;make VERSION=full;cd ../ptest; make SIM=../seq/ssim TFLAGS=-il;cd ../seq;
./optest.pl -s ../seq/ssim -il
Simulating with ../seq/ssim
  All 59 ISA Checks Succeed
./jtest.pl -s ../seq/ssim -il
Simulating with ../seq/ssim
  All 96 ISA Checks Succeed
./ctest.pl -s ../seq/ssim -il
Simulating with ../seq/ssim
  All 22 ISA Checks Succeed
./htest.pl -s ../seq/ssim -il
Simulating with ../seq/ssim
  All 870 ISA Checks Succeed
```

说明正确

### Part C

在`sim/pipe`里,修改pipeline 文件支持前面的IIADDL和ILEAVE,优化pipeline和y86指令,需要有流水线冲突转发知识

~~其实直接跑correctness.pl就知道并没有测试IIADDL和ILEAVE的~~

`FDEM`四阶段流水 其中`W`有阶段寄存器但是被融在了各个过程中

翻到`simguide.pdf`的pipeline或这书上的pipeline图也行

以下讲解区别与之前SEQ的部分

`Fetch Stage` 即从F(包括)到D(不包括)的部分

寄存器(在不同的stage之间都有硬件寄存器用来暂存,这里用大写首字母`F_`表示,也就是在pipeline图上的横条部分)和线路部分

因此所有从上一个阶段读入的都应该写作**大写首字母**,**输出**到线路的为**小写首字母**

```
##### Pipeline Register F ##########################################

intsig  F_predPC    'pc_curr->pc'           # Predicted value of PC

##### Intermediate Values in Fetch Stage ###########################

intsig  imem_icode  'imem_icode'            # icode field from instruction memory
intsig  imem_ifun   'imem_ifun'             # ifun  field from instruction memory
intsig  f_icode     'if_id_next->icode'     # (Possibly modified) instruction code
intsig  f_ifun      'if_id_next->ifun'      # Fetched instruction function
intsig  f_valC      'if_id_next->valc'      # Constant data of fetched instruction
intsig  f_valP      'if_id_next->valp'      # Address of following instruction
boolsig imem_error  'imem_error'            # Error signal from instruction memory
boolsig instr_valid 'instr_valid'           # Is fetched instruction valid?
```

`f_pc`控制下一条指令的pc位置的选择 图中`Select PC`

`f_predPC`为预测PC的值,图中`Predict PC`,不一定会被选,最后的选择还是由`f_pc`来决定

`f_stat`是fetch阶段的stat,图中`stat`,会被传入D的寄存器再往后传

注意到原有的`need_regids`等没有`f_`前缀 是因为这只是fetch阶段的中间`变量` 不会被往后传递

图中的rA rB valC等等 和SEQ一样 是作者已经帮我们写好了不需要我们自己解析.广告:如果对这个感兴趣,要知道具体的可以来选**软件与系统**方向的**数字部件**课~ :-)

---

`Decode Stage`

储存和线路内容见` Pipeline Register D ` 和` Intermediate Values in Decode Stage ` 这两部分

`d_srcA`和`d_srcB`为 蓝色的`Register file` 这里少了从`icode`,`rA`和`rB`到它的线 用于控制输出的`A`和`B` [表示不是很懂图上最右侧的两个寄存器有什么用]

`d_dstE`和`d_dstM`是之前就有的在第二阶段推断出的将来的目的储存位置,将会往后传递

`d_valA`和`d_valB`分别为灰色的`Sel+Fwd A`和`Fwd B`模块,功能是利用后面流水阶段的转发来解决部分hazad [细节在下面M阶段以后讲]

---

`Execute Stage`

储存和线路内容见` Pipeline Register E ` 和` Intermediate Values in Execute Stage ` 这两部分

构造同上的原理`aluA`,`aluB`,`alufun`,`set_cc`,`e_dstE`和图中模块相对应

看图上有一些直接传递的线并没有表示在代码中,如stat icode,可见作者应该是默认自动传递了,所以我也不是很懂为什么会有`int e_valA = E_valA;    # Pass valA through stage`这段，讲道理的话可以去掉这句,它会被“自动”传递

---

`Memory Stage`

储存和线路内容见` Pipeline Register M ` 和` Intermediate Values in Memory Stage ` 这两部分

`mem_addr`对应`Addr`

`mem_read`和`mem_write`对应`Mem control`

`m_stat`对应`stat`

同理我也不知道为什么下面的四个传递要干嘛,删掉试试也的确能跑

`Stat`最后再更新下Stat

---

首先把刚刚的D部分的`d_valA`和`d_valB`部分的细节讲一讲

前面已经降到这里是利用转发,那当同时`DEM`三个阶段都有会引起hazad都有可能转发的时候,越早的stage执行的代码越靠后,所以从最早相同的位置选择

那就完成了整个的结构?

并不hazad并没有有被完全解决 当相邻指令

`Pipeline Register Control`

|Condition|F|D|E|M|W|
|---|---|---|---|---|---|
|加载 / 使用冒险|stall|stall|bubble|||
|预测错误的分支||bubble|bubble|||
|处理ret|stall|bubble||||

stall即是让该位置的再存一个stage 即暂停效果

bubble让当前发出nop即空指令 即插入利用执行空指令来"等待"

IIADDL对后继指令产生的hazad在于计算结果 完成在E阶段 和后一阶段最多有一个stage差距 可以并已经由转发解决

ILEAVE对ebp的重写在M阶段 当下一条指令读ebp则无法利用转发解决 需要stall 属于加载/使用冒险

注意这里的代码用了 `不执行不影响`的思想(即只要对所有功能存储原件的值都是对的 那么线路中就算传递不正确的值也不会影响系统整体的正确性 因为只是传递而已 在写入时仍会被抛弃) 因此这里有了`D_bubble`和`E_bubble`的写法,注释也有讲

总共只用在意`F_stall`,`D_stall`,`D_bubble`,`E_bubble`

以上 在流水线里加了IADDL和LEAVE指令

---

`ncopy`即是 从src拷贝n个到dst 已经有可以运行的代码

```
# You can modify this portion
	# Loop header
	xorl %eax,%eax		  # count = 0;
	andl %edx,%edx		  # len <= 0?
	jle Done	      	  # if so, goto Done:
Loop:	
  mrmovl (%ebx), %esi	# read val from src...
	rmmovl %esi, (%ecx)	# ...and store it to dst
	andl %esi, %esi		  # val <= 0?
	jle Npos		        # if so, goto Npos:
	irmovl $1, %edi
	addl %edi, %eax		  # count++
Npos:	
  irmovl $1, %edi
	subl %edi, %edx		  # len--
	irmovl $4, %edi
	addl %edi, %ebx		  # src++
	addl %edi, %ecx		  # dst++
	andl %edx,%edx		  # len > 0?
	jg Loop			        # if so, goto Loop:
```

回顾我们刚刚做的流水线会产生bubble/stall的有(直接翻到最后)MRMOVL, POPL, LEAVE, RET, JXX

看这个给的代码可以优化的有

 1. RET
 2. JXX
 3. MRMOVL后的指令

RET 就一次 据说也有效果不过要改`hcl` 懒如我`_(:з」∠)_`并没有改

JXX 每次的循环都是会用到
 1. 思路1.因为是有针对性的,那么可否默认都跳转,讲道理有实现的可能,但这即要改hcl每次遇到JXX,就把原来PC+和valC对换,对于中间的Npos为50%期望可以消除大量for的bubble ,这改的难度也较大,未实施.广告:有兴趣跳转预测甚至乱序指令的欢迎选课**软件与系统**方向的**计算机体系结构**课.
 2. 思路2.利用循环展开把16个(我这里用的16个,可以自己试试多少个即不会超过长度也可以达到好的CPE)作为一个循环节,每16个判断一次,最后剩下的用switch的方式(lab2做过的都知道)进行跳转,比如43个,当两次16个执行完后,发现剩下11个则通过跳转表调到剩11个的位置执行剩下11个,这种方法减少了了`n*15/16`个bubble,我的代码也就用了这种方法

MRMOVL 也是大量的

使用的方法ics上好像是有讲过,反正**体系结构**讲了,这里也使用了

观察已经展开的连续两次操作

```
mrmovl 0(%ebx), %esi  # read val from (src + 0) 产生bubble
rmmovl %esi, 0(%ecx)  # store it to (dst + 0)
...
mrmovl 4(%ebx), %edi  # read val from (src + 4) 产生bubble
rmmovl %edi, 4(%ecx)  # store it to (dst + 4)
```

注意到mr和rm之间有bubble 于是有了一个叫`指令流水`还是什么的想法,这个想法和pipeline如出一辙,都想办法让代码同时执行。变成了

```
mrmovl 0(%ebx), %esi  # read val from (src + 0) 可转发解决
mrmovl 4(%ebx), %edi  # read val from (src + 4) 可转发解决
rmmovl %esi, 0(%ecx)  # store it to (dst + 0)
rmmovl %edi, 4(%ecx)  # store it to (dst + 4)
```

以上我的所有优化,具体的y86代码见`ncopy.ys`,建议写个c代码去生成吧,反正我是这样干的

记得当时测的是8点几的CPE,怎么现在测出来3.几 为什么感觉还有零点几的CPE 感觉评测的坏了？一脸懵逼???

`./correctness.pl`检测正确性

`./benchmark.pl` 测CPE

---

有大腿们还做了hcl优化 表示并没有学会orz

