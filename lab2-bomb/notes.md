# Guide

## 知识储备

  1. C 到 assembly language 的转换关系(变量 堆栈 for/while switch rax/eax/ax/ah/al关系)
  2. 调试工具gdb
  3. 反编译工具objdump(`objdump -d bomb > dump`)
  4. 打印所有字符串strings(`strings bomb > strings.txt`)
  5. [以上知识储备参考资料](https://gist.github.com/ThaWeatherman/6912322)

## 题目

这是一个linux可执行的C语言"炸弹"，一共有六个关卡，每个关卡需要输入一个字符串，如果输入的是正确的字符串，那么这关将被破解，如果是错误的将会显示"BOOM!!!"(原训练会根据爆炸次数扣分,还有及时的分数板)

该训练主要练习理解汇编,使用调试器(如gdb/ddd),学会使用断点,学习反编译工具

注意有提供`bomb.c`可以知道整个框架结构

正确检验`./bomb student_solution.txt`

## Solution

`git diff default:lab2-bomb finish:lab2-bomb`

可以看到 所有的关卡都是

``` c
input = read_line();
phase_i(input);
phase_defused();
```

也就是说相对于题目描述并没有多出什么信息

下面使用gdb时记得每次加上`b explode_bomb`断点

先反汇编`objdump -d bomb > dump`

---

搜索定位到`phase_1`

```
0000000000400ee0 <phase_1>:
  400ee0: 48 83 ec 08           sub    $0x8,%rsp                    # 堆栈保存 以下序号表示阅读顺序
  400ee4: be 00 24 40 00        mov    $0x402400,%esi               # 4.某字符串地址0x402400
  400ee9: e8 4a 04 00 00        callq  401338 <strings_not_equal>   # 1.字符串不等函数
  400eee: 85 c0                 test   %eax,%eax                    # 2.测试eax是否为0(false)
  400ef0: 74 05                 je     400ef7 <phase_1+0x17>        # 3.是0则跳 说明需要上面的字符串相等
  400ef2: e8 43 05 00 00        callq  40143a <explode_bomb>
  400ef7: 48 83 c4 08           add    $0x8,%rsp
  400efb: c3                    retq
```

所以找到该地址的字符串就好 输入命令

```
gdb bomb
> x/s 0x402400
0x402400: "Border relations with Canada have never been better."
```
说明第一题的解就是"Border relations with Canada have never been better." 写入`student_solution.txt`的第一行

---

```
0000000000400efc <phase_2>:
  400efc: 55                    push   %rbp
  400efd: 53                    push   %rbx
  400efe: 48 83 ec 28           sub    $0x28,%rsp
  400f02: 48 89 e6              mov    %rsp,%rsi
  400f05: e8 52 05 00 00        callq  40145c <read_six_numbers> # 01.读六个数[lab不会骗你 说读几个就是几个 不信你看06步
  400f0a: 83 3c 24 01           cmpl   $0x1,(%rsp)               # 04.第一个数和1比较
  400f0e: 74 20                 je     400f30 <phase_2+0x34>     # 03.比较需要相等 跳到f30
  400f10: e8 25 05 00 00        callq  40143a <explode_bomb>     # 02.炸弹位置
  400f15: eb 19                 jmp    400f30 <phase_2+0x34>
  400f17: 8b 43 fc              mov    -0x4(%rbx),%eax           # 08.eax = (rbx - 4 )
  400f1a: 01 c0                 add    %eax,%eax                 # 12.eax = 2 eax 即这里的 2eax = (rbx) 综合这三句表明 每一个数是前一个数的一倍
  400f1c: 39 03                 cmp    %eax,(%rbx)               # 11.eax = (rbx)
  400f1e: 74 05                 je     400f25 <phase_2+0x29>     # 10.是个循环 且比较必须相等 跳到f25
  400f20: e8 15 05 00 00        callq  40143a <explode_bomb>     # 09.炸弹位置
  400f25: 48 83 c3 04           add    $0x4,%rbx                 # 13.rbx = 下一个数的地址
  400f29: 48 39 eb              cmp    %rbp,%rbx                 # 14.比较rbx 和rbp
  400f2c: 75 e9                 jne    400f17 <phase_2+0x1b>     # 15.不同则循环 比较内容参见06. 结合04 08 11 12 得出数列1 2 4 8 16 32
  400f2e: eb 0c                 jmp    400f3c <phase_2+0x40>     # 16.相同则f3c 结束循环
  400f30: 48 8d 5c 24 04        lea    0x4(%rsp),%rbx            # 05.rbx = rsp+0x4                  第二个数的地址
  400f35: 48 8d 6c 24 18        lea    0x18(%rsp),%rbp           # 06.rbp = rsp+0x18                 第"七"个数的地址
  400f3a: eb db                 jmp    400f17 <phase_2+0x1b>     # 07.调到f17
  400f3c: 48 83 c4 28           add    $0x28,%rsp
  400f40: 5b                    pop    %rbx
  400f41: 5d                    pop    %rbp
  400f42: c3                    retq
```

结合步骤 15 04 08 11 12 得出数列`1 2 4 8 16 32` 写入`student_solution.txt`的第二行

---

```
0000000000400f43 <phase_3>:
  400f43: 48 83 ec 18           sub    $0x18,%rsp
  400f47: 48 8d 4c 24 0c        lea    0xc(%rsp),%rcx
  400f4c: 48 8d 54 24 08        lea    0x8(%rsp),%rdx
  400f51: be cf 25 40 00        mov    $0x4025cf,%esi                   # 0. 像phase_1一样 用gdb命令 x/s 0x4025cf 得到 "%d %d"说明读了两个整数
  400f56: b8 00 00 00 00        mov    $0x0,%eax
  400f5b: e8 90 fc ff ff        callq  400bf0 <__isoc99_sscanf@plt>     # 1. 原来是调这个函数 那么 第一个数$rsp+8 第二个数$rsp+12 注意只能存16bits < 0xff
  400f60: 83 f8 01              cmp    $0x1,%eax                        # 2. 表示读的数的个数要大于1
  400f63: 7f 05                 jg     400f6a <phase_3+0x27>            #
  400f65: e8 d0 04 00 00        callq  40143a <explode_bomb>
  400f6a: 83 7c 24 08 07        cmpl   $0x7,0x8(%rsp)                   # 3. 7 compare 第一个数
  400f6f: 77 3c                 ja     400fad <phase_3+0x6a>            # 4. ja = jump if above , 目标地址是炸弹 所以 7 >= 第一个数
  400f71: 8b 44 24 08           mov    0x8(%rsp),%eax                   # 5. eax = 第一个数 注意 eax = rax[0..31]
  400f75: ff 24 c5 70 24 40 00  jmpq   *0x402470(,%rax,8)               # 6.  跳到 402470+8*第一个数 注意到下面全部都是调到fbe 明显这里是switch
  400f7c: b8 cf 00 00 00        mov    $0xcf,%eax                       # 10. 组合(0,207)
  400f81: eb 3b                 jmp    400fbe <phase_3+0x7b>
  400f83: b8 c3 02 00 00        mov    $0x2c3,%eax                      # 9. 大于0xff 排除
  400f88: eb 34                 jmp    400fbe <phase_3+0x7b>
  400f8a: b8 00 01 00 00        mov    $0x100,%eax                      # 9. 大于0xff 排除
  400f8f: eb 2d                 jmp    400fbe <phase_3+0x7b>
  400f91: b8 85 01 00 00        mov    $0x185,%eax                      # 9. 大于0xff 排除
  400f96: eb 26                 jmp    400fbe <phase_3+0x7b>
  400f98: b8 ce 00 00 00        mov    $0xce,%eax                       # 10.组合(5,206)
  400f9d: eb 1f                 jmp    400fbe <phase_3+0x7b>
  400f9f: b8 aa 02 00 00        mov    $0x2aa,%eax                      # 9. 大于0xff 排除
  400fa4: eb 18                 jmp    400fbe <phase_3+0x7b>
  400fa6: b8 47 01 00 00        mov    $0x147,%eax                      # 9. 大于0xff 排除
  400fab: eb 11                 jmp    400fbe <phase_3+0x7b>
  400fad: e8 88 04 00 00        callq  40143a <explode_bomb>
  400fb2: b8 00 00 00 00        mov    $0x0,%eax                        # 10.没有跳到这里的
  400fb7: eb 05                 jmp    400fbe <phase_3+0x7b>
  400fb9: b8 37 01 00 00        mov    $0x137,%eax                      # 9. 大于0xff 排除
  400fbe: 3b 44 24 0c           cmp    0xc(%rsp),%eax                   # 7. 比较第二个数和eax
  400fc2: 74 05                 je     400fc9 <phase_3+0x86>            # 8. 相等则成功
  400fc4: e8 71 04 00 00        callq  40143a <explode_bomb>
  400fc9: 48 83 c4 18           add    $0x18,%rsp
  400fcd: c3                    retq
```

第9+6步 查看跳转地址

```
gdb bomb
> r student_solution.txt
> x/8w 0x402470
0x402470: 0x00400f7c(0,207)  0x00000000  0x00400fb9(1已排除)  0x00000000
0x402480: 0x00400f83(2已排除)  0x00000000  0x00400f8a(3已排除)  0x00000000
0x402490: 0x00400f91(4已排除)  0x00000000  0x00400f98(5,206)  0x00000000
0x4024a0: 0x00400f9f(6已排除)  0x00000000  0x00400fa6(7已排除)  0x00000000
```

综上`0 207`或`5 206` 写入`student_solution.txt`的第三行

---

```
0000000000400fce <func4>:
  400fce: 48 83 ec 08           sub    $0x8,%rsp
  400fd2: 89 d0                 mov    %edx,%eax                   # eax  = edx      第一次 0xe
  400fd4: 29 f0                 sub    %esi,%eax                   # eax -= esi      第一次 0
  400fd6: 89 c1                 mov    %eax,%ecx                   # ecx  = eax
  400fd8: c1 e9 1f              shr    $0x1f,%ecx                  # 逻辑右移 31一位即 ecx 为eax的符号位(负1,非负0)
  400fdb: 01 c8                 add    %ecx,%eax                   # eax += ecx
  400fdd: d1 f8                 sar    %eax                        # eax算数右移动1位
  400fdf: 8d 0c 30              lea    (%rax,%rsi,1),%ecx          # ecx = rax + rsi
  400fe2: 39 f9                 cmp    %edi,%ecx                   # ecx<=edi 这里看出 edi是上界 在phase_4里看也就是 输入的第一个数
  400fe4: 7e 0c                 jle    400ff2 <func4+0x24>         # 跳到ff2
  400fe6: 8d 51 ff              lea    -0x1(%rcx),%edx              # edx = rcx -1
  400fe9: e8 e0 ff ff ff        callq  400fce <func4>               # 递归
  400fee: 01 c0                 add    %eax,%eax                    # eax=2eax 返回值乘2
  400ff0: eb 15                 jmp    401007 <func4+0x39>          # 结束返回
  400ff2: b8 00 00 00 00        mov    $0x0,%eax              ##### eax = 0  对应 只有这里可以返回0 所以 根据路径((0xe-0x0)>>1)+0x0=0x7
  400ff7: 39 f9                 cmp    %edi,%ecx                  # ecx>=edi 结束
  400ff9: 7d 0c                 jge    401007 <func4+0x39>
  400ffb: 8d 71 01              lea    0x1(%rcx),%esi              # esi = rcx-1
  400ffe: e8 cb ff ff ff        callq  400fce <func4>              # 递归
  401003: 8d 44 00 01           lea    0x1(%rax,%rax,1),%eax   ##### eax = 2 eax + 1 一旦进入递归 就会变为正数
  401007: 48 83 c4 08           add    $0x8,%rsp
  40100b: c3                    retq
000000000040100c <phase_4>:
  40100c: 48 83 ec 18           sub    $0x18,%rsp
  401010: 48 8d 4c 24 0c        lea    0xc(%rsp),%rcx
  401015: 48 8d 54 24 08        lea    0x8(%rsp),%rdx
  40101a: be cf 25 40 00        mov    $0x4025cf,%esi               # 1. 老规矩 x/s 0x4025cf 得到%d %d
  40101f: b8 00 00 00 00        mov    $0x0,%eax
  401024: e8 c7 fb ff ff        callq  400bf0 <__isoc99_sscanf@plt>
  401029: 83 f8 02              cmp    $0x2,%eax                    # 2. 需要刚好输入两个数？？？黑人问号
  40102c: 75 07                 jne    401035 <phase_4+0x29>
  40102e: 83 7c 24 08 0e        cmpl   $0xe,0x8(%rsp)               # 3. 0xe >= 第一个数
  401033: 76 05                 jbe    40103a <phase_4+0x2e>        # 3. jbe = below or equal
  401035: e8 00 04 00 00        callq  40143a <explode_bomb>
  40103a: ba 0e 00 00 00        mov    $0xe,%edx                    # 4.edx = 0xe
  40103f: be 00 00 00 00        mov    $0x0,%esi                    # 4.esi = 0
  401044: 8b 7c 24 08           mov    0x8(%rsp),%edi               # 4.edi = 第一个数字
  401048: e8 81 ff ff ff        callq  400fce <func4>               # 7.去看看函数
  40104d: 85 c0                 test   %eax,%eax                    # 5.需要返回0
  40104f: 75 07                 jne    401058 <phase_4+0x4c>
  401051: 83 7c 24 0c 00        cmpl   $0x0,0xc(%rsp)               # 6.需要0==第二个数???送分咯
  401056: 74 05                 je     40105d <phase_4+0x51>
  401058: e8 dd 03 00 00        callq  40143a <explode_bomb>
  40105d: 48 83 c4 18           add    $0x18,%rsp
  401061: c3                    retq


```

综上`7 0` 写入`student_solution.txt`的第四行

---

```
0000000000401062 <phase_5>:
  401062: 53                    push   %rbx
  401063: 48 83 ec 20           sub    $0x20,%rsp
  401067: 48 89 fb              mov    %rdi,%rbx
  40106a: 64 48 8b 04 25 28 00  mov    %fs:0x28,%rax
  401071: 00 00
  401073: 48 89 44 24 18        mov    %rax,0x18(%rsp)
  401078: 31 c0                 xor    %eax,%eax
  40107a: e8 9c 02 00 00        callq  40131b <string_length>      # 0.哎卧槽要读字符串长度
  40107f: 83 f8 06              cmp    $0x6,%eax                   # 1.还要等于6
  401082: 74 4e                 je     4010d2 <phase_5+0x70>
  401084: e8 b1 03 00 00        callq  40143a <explode_bomb>
  401089: eb 47                 jmp    4010d2 <phase_5+0x70>
  40108b: 0f b6 0c 03           movzbl (%rbx,%rax,1),%ecx          # 4.ecx = (rbx + rax) 第一次 rcx=(rbx+0)也就是读保存第一个字符
  40108f: 88 0c 24              mov    %cl,(%rsp)                  #
  401092: 48 8b 14 24           mov    (%rsp),%rdx                 # 5.rdx = cl
  401096: 83 e2 0f              and    $0xf,%edx                   # 6.edx 只取低4位edx = rbx+rax[0..3]
  401099: 0f b6 92 b0 24 40 00  movzbl 0x4024b0(%rdx),%edx         # 7.edx = (0x4024b0 + rdx) 第一次存入第一个字符??并不是 这里把字符的值拿来作为偏移量 x/s 0x4024b0 得到 maduiersnfotvbyl......
  4010a0: 88 54 04 10           mov    %dl,0x10(%rsp,%rax,1)       # 8.(rax+rsp+0x10)=dl=edx[0..7]
  4010a4: 48 83 c0 01           add    $0x1,%rax                   # 9.rax++
  4010a8: 48 83 f8 06           cmp    $0x6,%rax                   # 10.rax==6
  4010ac: 75 dd                 jne    40108b <phase_5+0x29>       # 11.循环到rax=6 , 12.见下
  4010ae: c6 44 24 16 00        movb   $0x0,0x16(%rsp)             # 13.表中再下一个地址的值为0 表示字符串结束符
  4010b3: be 5e 24 40 00        mov    $0x40245e,%esi              # 14.又是另一个地址 x/s 0x40245e 得到 "flyers"
  4010b8: 48 8d 7c 24 10        lea    0x10(%rsp),%rdi             # 15.rdi = (rsp+0x10) 也就是刚刚 奇妙操作得到的字符串
  4010bd: e8 76 02 00 00        callq  401338 <strings_not_equal>
  4010c2: 85 c0                 test   %eax,%eax                   # 16.字符串相等则成功 说明通过奇妙的技术造出`flyers`
  4010c4: 74 13                 je     4010d9 <phase_5+0x77>
  4010c6: e8 6f 03 00 00        callq  40143a <explode_bomb>
  4010cb: 0f 1f 44 00 00        nopl   0x0(%rax,%rax,1)
  4010d0: eb 07                 jmp    4010d9 <phase_5+0x77>       # 跳出
  4010d2: b8 00 00 00 00        mov    $0x0,%eax                   # 2.跳到这里 eax=0
  4010d7: eb b2                 jmp    40108b <phase_5+0x29>       # 3.又跳??
  4010d9: 48 8b 44 24 18        mov    0x18(%rsp),%rax
  4010de: 64 48 33 04 25 28 00  xor    %fs:0x28,%rax
  4010e5: 00 00
  4010e7: 74 05                 je     4010ee <phase_5+0x8c>
  4010e9: e8 42 fa ff ff        callq  400b30 <__stack_chk_fail@plt>
  4010ee: 48 83 c4 20           add    $0x20,%rsp
  4010f2: 5b                    pop    %rbx
  4010f3: c3                    retq
```

越来越长了 为了压压惊 先搜一下phase_5 发现只有内部跳转没有自己调用 不会递归 :-) 就送了一口气 恩？？？

12步 看循环内部rbx,rsp没变

|Address|value|
|---|---|
|rsp+0x10|(0x4024b0 + rbx[0..3] )|
|rsp+0x11|(0x4024b1 + rbx[0..3] )|
|rsp+0x12|(0x4024b2 + rbx[0..3] )|
|rsp+0x13|(0x4024b3 + rbx[0..3] )|
|rsp+0x14|(0x4024b4 + rbx[0..3] )|
|rsp+0x15|(0x4024b5 + rbx[0..3] )|
|rsp+0x16|'\0'|

17步

`flyers` 在 `maduiersnfotvbyl` 依次是 (9,f,e,5,6,7)

注意到0的ASCII=48刚好是16的倍数,建立对应字符串"9?>567" 当然你还可以使用 其它ASCII模运算=(9fe567)mod(0x10)的字符

```
maduiersnfotvbyl
0123456789ABCDEF
0123456789:;<=>?
```

综上`9?>567` 写入`student_solution.txt`的第五行

---

```
00000000004010f4 <phase_6>:
  4010f4: 41 56                 push   %r14
  4010f6: 41 55                 push   %r13
  4010f8: 41 54                 push   %r12
  4010fa: 55                    push   %rbp
  4010fb: 53                    push   %rbx
  4010fc: 48 83 ec 50           sub    $0x50,%rsp
  401100: 49 89 e5              mov    %rsp,%r13
  401103: 48 89 e6              mov    %rsp,%rsi
  401106: e8 51 03 00 00        callq  40145c <read_six_numbers>    # 又读六个数.....举报了
  40110b: 49 89 e6              mov    %rsp,%r14
  40110e: 41 bc 00 00 00 00     mov    $0x0,%r12d
  401114: 4c 89 ed              mov    %r13,%rbp                    #<----
  401117: 41 8b 45 00           mov    0x0(%r13),%eax                     |
  40111b: 83 e8 01              sub    $0x1,%eax                          |
  40111e: 83 f8 05              cmp    $0x5,%eax                          |
  401121: 76 05                 jbe    401128 <phase_6+0x34>        #     | 数-1 <= 5  |
  401123: e8 12 03 00 00        callq  40143a <explode_bomb>              |
  401128: 41 83 c4 01           add    $0x1,%r12d                         |
  40112c: 41 83 fc 06           cmp    $0x6,%r12d                   #     | r12d为循环量 循环6次跳出 及检查每一个数小于6
  401130: 74 21                 je     401153 <phase_6+0x5f>     ---#     |
  401132: 44 89 e3              mov    %r12d,%ebx               |         |
  401135: 48 63 c3              movslq %ebx,%rax                |   #<--  | ebx从r12d开始循环
  401138: 8b 04 84              mov    (%rsp,%rax,4),%eax       |       | |
  40113b: 39 45 00              cmp    %eax,0x0(%rbp)           |       | | 和当前r12d对应index值比较 要求两两不等
  40113e: 75 05                 jne    401145 <phase_6+0x51>    |   #   | |
  401140: e8 f5 02 00 00        callq  40143a <explode_bomb>    |       | |
  401145: 83 c3 01              add    $0x1,%ebx                |       | |
  401148: 83 fb 05              cmp    $0x5,%ebx                |       | |
  40114b: 7e e8                 jle    401135 <phase_6+0x41>    |   # --  |
  40114d: 49 83 c5 04           add    $0x4,%r13                |         |
  401151: eb c1                 jmp    401114 <phase_6+0x20>    |   # ----
  401153: 48 8d 74 24 18        lea    0x18(%rsp),%rsi           -->#      ###########################################
  401158: 4c 89 f0              mov    %r14,%rax
  40115b: b9 07 00 00 00        mov    $0x7,%ecx
  401160: 89 ca                 mov    %ecx,%edx                    #<--
  401162: 2b 10                 sub    (%rax),%edx                      |
  401164: 89 10                 mov    %edx,(%rax)                      |
  401166: 48 83 c0 04           add    $0x4,%rax                        |
  40116a: 48 39 f0              cmp    %rsi,%rax                        |
  40116d: 75 f1                 jne    401160 <phase_6+0x6c>        #---   a[i]=7-a[i]
  40116f: be 00 00 00 00        mov    $0x0,%esi                           ###########################################
  401174: eb 21                 jmp    401197 <phase_6+0xa3>        #-----
  401176: 48 8b 52 08           mov    0x8(%rdx),%rdx               #<-   | #<------  edx+=8
  40117a: 83 c0 01              add    $0x1,%eax                       |  |         |
  40117d: 39 c8                 cmp    %ecx,%eax                       |  |         |
  40117f: 75 f5                 jne    401176 <phase_6+0x82>        #--   |         | eax = ecx
  401181: eb 05                 jmp    401188 <phase_6+0x94>        #--   |         |
  401183: ba d0 32 60 00        mov    $0x6032d0,%edx                  |  | #<--    |
  401188: 48 89 54 74 20        mov    %rdx,0x20(%rsp,%rsi,2)       #<-   |     |   | (rsp+2*rsi)=edx
  40118d: 48 83 c6 04           add    $0x4,%rsi                          |     |   |
  401191: 48 83 fe 18           cmp    $0x18,%rsi                         |     |   |
  401195: 74 14                 je     4011ab <phase_6+0xb7>     ---#     |     |   |
  401197: 8b 0c 34              mov    (%rsp,%rsi,1),%ecx       |   #<----      |   | ecx = rsp + esi{0~0x14 step 4}
  40119a: 83 f9 01              cmp    $0x1,%ecx                |               |   |
  40119d: 7e e4                 jle    401183 <phase_6+0x8f>    |   #-----------    |
  40119f: b8 01 00 00 00        mov    $0x1,%eax                |                   |
  4011a4: ba d0 32 60 00        mov    $0x6032d0,%edx           |                   |
  4011a9: eb cb                 jmp    401176 <phase_6+0x82>    |   #---------------
  4011ab: 48 8b 5c 24 20        mov    0x20(%rsp),%rbx           -->#      #### (%rsp+0x20+8*i{0~5})=0x6032d0 + (a[i]-1)*0x10;通过x/12w  $rsp+0x20可以看到储存结构
  4011b0: 48 8d 44 24 28        lea    0x28(%rsp),%rax
  4011b5: 48 8d 74 24 50        lea    0x50(%rsp),%rsi              #结尾地址
  4011ba: 48 89 d9              mov    %rbx,%rcx                              # 通过x/24x 0x006032d0 看出结构是链表状有指针 有value
  4011bd: 48 8b 10              mov    (%rax),%rdx                  #<--
  4011c0: 48 89 51 08           mov    %rdx,0x8(%rcx)                   |
  4011c4: 48 83 c0 08           add    $0x8,%rax                        |
  4011c8: 48 39 f0              cmp    %rsi,%rax                        |
  4011cb: 74 05                 je     4011d2 <phase_6+0xde>     ---#   |
  4011cd: 48 89 d1              mov    %rdx,%rcx                |       |
  4011d0: eb eb                 jmp    4011bd <phase_6+0xc9>    |   #---
  4011d2: 48 c7 42 08 00 00 00  movq   $0x0,0x8(%rdx)            -->#         # 根据提供的 a[i]的顺序 把他们串起来 最后一个指向空
  4011d9: 00
  4011da: bd 05 00 00 00        mov    $0x5,%ebp
  4011df: 48 8b 43 08           mov    0x8(%rbx),%rax               #<--
  4011e3: 8b 00                 mov    (%rax),%eax                      |
  4011e5: 39 03                 cmp    %eax,(%rbx)                      |     # 链表访问比较 要按value降序排列
  4011e7: 7d 05                 jge    4011ee <phase_6+0xfa>        #   |
  4011e9: e8 4c 02 00 00        callq  40143a <explode_bomb>            |
  4011ee: 48 8b 5b 08           mov    0x8(%rbx),%rbx                   |
  4011f2: 83 ed 01              sub    $0x1,%ebp                        |
  4011f5: 75 e8                 jne    4011df <phase_6+0xeb>        #---
  4011f7: 48 83 c4 50           add    $0x50,%rsp
  4011fb: 5b                    pop    %rbx
  4011fc: 5d                    pop    %rbp
  4011fd: 41 5c                 pop    %r12
  4011ff: 41 5d                 pop    %r13
  401201: 41 5e                 pop    %r14
  401203: c3                    retq
```

 1. 画一下 跳转结构
 2. 按一块一块 分析

```
(gdb) x/24w  0x6032d0
0x6032d0 <node1>: 0x0000014c
0x6032e0 <node2>: 0x000000a8
0x6032f0 <node3>: 0x0000039c
0x603300 <node4>: 0x000002b3
0x603310 <node5>: 0x000001dd
0x603320 <node6>: 0x000001bb
```

降序排序为 `3 4 5 6 1 2`

注意过程中有 `a[i]=7-a[i]`的操作 所以原序列为`4 3 2 1 6 5` 写入`student_solution.txt`的第六行

---

还有一个`secret_phase`!!!!怎么和说好的不一样

通过搜索发现来自`phase_defused`

首先 `num_input_strings`告诉我们 首先要完成前面六关 **这里有一些疑问 加断点0x202181+$rip的值一直是0 ?** 虽然 只有第6次没有往外`0x40163f`跳

发现里面几个和字符串有关的函数`__isoc99_sscanf` `strings_not_equal`

注意到`sscanf`不是`scanf`所以是从前面某一个字符串读取来的
```
x/s 0x402619
0x402619: "%d %d %s"

x/s 0x603870
0x603870 <input_strings+240>: "7 0" 也就是第四行

x/s 0x402622
0x402622: "DrEvil"
```

说明 该字符串是两个数和一个字串`DrEvil` sscanf上面3个lea没有看懂 应该是sscanf的参数 但没弄懂具体意思

所以在 第四行 最后加上DrEvil

```
0000000000401204 <fun7>:
  401204: 48 83 ec 08           sub    $0x8,%rsp
  401208: 48 85 ff              test   %rdi,%rdi
  40120b: 74 2b                 je     401238 <fun7+0x34>
  40120d: 8b 17                 mov    (%rdi),%edx
  40120f: 39 f2                 cmp    %esi,%edx
  401211: 7e 0d                 jle    401220 <fun7+0x1c>
  401213: 48 8b 7f 08           mov    0x8(%rdi),%rdi
  401217: e8 e8 ff ff ff        callq  401204 <fun7>
  40121c: 01 c0                 add    %eax,%eax
  40121e: eb 1d                 jmp    40123d <fun7+0x39>
  401220: b8 00 00 00 00        mov    $0x0,%eax
  401225: 39 f2                 cmp    %esi,%edx
  401227: 74 14                 je     40123d <fun7+0x39>
  401229: 48 8b 7f 10           mov    0x10(%rdi),%rdi
  40122d: e8 d2 ff ff ff        callq  401204 <fun7>
  401232: 8d 44 00 01           lea    0x1(%rax,%rax,1),%eax
  401236: eb 05                 jmp    40123d <fun7+0x39>
  401238: b8 ff ff ff ff        mov    $0xffffffff,%eax
  40123d: 48 83 c4 08           add    $0x8,%rsp
  401241: c3                    retq
0000000000401242 <secret_phase>:
  401242: 53                    push   %rbx
  401243: e8 56 02 00 00        callq  40149e <read_line>
  401248: ba 0a 00 00 00        mov    $0xa,%edx
  40124d: be 00 00 00 00        mov    $0x0,%esi
  401252: 48 89 c7              mov    %rax,%rdi
  401255: e8 76 f9 ff ff        callq  400bd0 <strtol@plt>          # 1.字符转
  40125a: 48 89 c3              mov    %rax,%rbx
  40125d: 8d 40 ff              lea    -0x1(%rax),%eax
  401260: 3d e8 03 00 00        cmp    $0x3e8,%eax                  # 2.输入-1<=1000
  401265: 76 05                 jbe    40126c <secret_phase+0x2a>
  401267: e8 ce 01 00 00        callq  40143a <explode_bomb>
  40126c: 89 de                 mov    %ebx,%esi
  40126e: bf f0 30 60 00        mov    $0x6030f0,%edi
  401273: e8 8c ff ff ff        callq  401204 <fun7>                # 4.函数
  401278: 83 f8 02              cmp    $0x2,%eax
  40127b: 74 05                 je     401282 <secret_phase+0x40>   # 3.返回值等于2
  40127d: e8 b8 01 00 00        callq  40143a <explode_bomb>
  401282: bf 38 24 40 00        mov    $0x402438,%edi
  401287: e8 84 f8 ff ff        callq  400b10 <puts@plt>
  40128c: e8 33 03 00 00        callq  4015c4 <phase_defused>
  401291: 5b                    pop    %rbx
```

5.先直译fun7为c代码 再整理得到

整理方法 搜索后缀相同的进行替换 去掉不会发生的分支

```c
fun7( rdi ){
  if(输入 < *rdi){
    return 2 * fun7(*(rdi + 0x8) );
  }else{
    if(输入 == *rdi)
      return 0;
    return 1 + 2*fun7( *(rdi + 0x10) );
  }
}
fun7(0x6030f0)
```
要得到2 看return的类型有

```
返回两倍
返回0
返回1+两倍
```

所以只有 `2 * ( 1 + 2 * 0)`可以得到2

从上到下 `esi < *rdi` -> `esi > *rdi` -> `esi = *rdi`

```
(gdb) x/32x 0x6030f0
0x6030f0 <n1>:  0x00000024  0x00000000  0x00603110  0x00000000
0x603100 <n1+16>: 0x00603130  0x00000000  0x00000000  0x00000000
0x603110 <n21>: 0x00000008  0x00000000  0x00603190  0x00000000
0x603120 <n21+16>:  0x00603150  0x00000000  0x00000000  0x00000000
0x603130 <n22>: 0x00000032  0x00000000  0x00603170  0x00000000
0x603140 <n22+16>:  0x006031b0  0x00000000  0x00000000  0x00000000
0x603150 <n32>: 0x00000016  0x00000000  0x00603270  0x00000000
0x603160 <n32+16>:  0x00603230  0x00000000  0x00000000  0x00000000
```

输入 < 0x24 跳到 0x00603110

输入 > 0x8 跳到 0x00603150

输入 = 0x16

所以 也就是`0x16=22`将`22` 写入`student_solution.txt`的第七行

---

至此`student_solution.txt` 里为

```
Border relations with Canada have never been better.
1 2 4 8 16 32
0 207
7 0 DrEvil
9?>567
4 3 2 1 6 5
22
```

运行 `./bomb student_solution.txt` 验证答案

