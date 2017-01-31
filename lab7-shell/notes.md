Guide
---

# 知识储备

  1. linux signal
  2. 线程,程序运行状态
  3. 书的第八章

# 题目

  [官方文档](http://csapp.cs.cmu.edu/3e/shlab.pdf)

任务为实现下面的函数
 * eval: Main routine that parses and interprets the command line. [70 lines]
 * builtin cmd: Recognizes and interprets the built-in commands: quit, fg, bg, and jobs. [25 lines]
 * do bgfg: Implements the bg and fg built-in commands. [50 lines]
 * waitfg: Waits for a foreground job to complete. [20 lines]
 * sigchld handler: Catches SIGCHILD signals. 80 lines]
 * sigint handler: Catches SIGINT (ctrl-c) signals. [15 lines]
 * sigtstp handler: Catches SIGTSTP (ctrl-z) signals. [15 lines]

# Solution

`git diff default:lab7-shell finish:lab7-shell`

进程管理和信号

pdf 讲基本法
 * argc 和 argv
 * 以&结束 表示后台运行
 * ctrl-c 发送SIGINT信号
 * ctrl-z 发送SIGTSTP信号

讲内置命令
 * jobs: List the running and stopped background jobs.
 * bg <job>: Change a stopped background job to a running background job.
 * fg <job>: Change a stopped or running background job to a running in the foreground.
 * kill <job>: Terminate a job.(我们不用支持)

讲我们要做的tsh应该 (也就是上面的总和)
 1. The prompt should be the string “tsh> ”
 2. 如果命令是内置的要立即处理并等待下一个命令,否则看做一个指向可执行文件的路径,并用子进程运行它
 3. 不需要支持 管道 | 以及I/O重定向 `<`和`>`
 4. ctrl-c/ctrl-z 要发相应的信号给前台运行着的程序
 5. 以&结束 表示后台运行
 6. 支持上面所讲的3个命令 jobs bg fg + quit
 7. tsh should reap all of its zombie children. If any job terminates because it receives a signal that it didn’t catch, then tsh should recognize this event and print a message with the job’s PID and a description of the offending signal.

已经读完pdf 但pdf不再一步步具体的指导我们了,下一步怎么做？因为文件较少我采取的方法如下

先阅读README 虽然是README 但毕竟吸引力不如pdf 前面几个lab都没怎么看README,它告诉我们
 * `my*.c`这些程序是在我们tsh上运行的trace的调用的程序 所以也就不用看了
 * `tshref` 是作者提供的tsh的demo
 * `tsh.c` 是我们要改的
 * 其余是和测试相关

我们目前也就只用管心`tsh.c` 接下来阅读之

发现顶部宏主要是最大值的限制 和关于状态的定义以及变换注释

再看看全局变量 和 已提供的帮助函数 以下只写最直接影响本lab的操作 例如main 里 dup2(1,2) 这样的就不解释了

 * `main` 分别设置了SIGINT,SIGTSTP,SIGCHLD,SIGQUIT的对应处理函数 不断读入一整行 并把读入的字符串作为参数传给eval()
 * `eval` 尚未实现 应该的样子见描述
 * `parseline(传入行,将被写的argv)` 如注释描述 将传入行解析到argv里 并返回是否为后台运行
 * `builtin_cmd` 尚未实现 应该的样子见描述
 * `do_bgfg` 尚未实现 应该的样子见描述
 * `waitfg` 尚未实现 应该的样子见描述

以下是信号处理
 * `sigchld_handler` 尚未实现 应该的样子见描述
 * `sigint_handler` 尚未实现 应该的样子见描述
 * `sigtstp_handler` 尚未实现 应该的样子见描述

辅助函数 个人建议看看实现 以至于一会写代码不会太茫然
 1. `clearjob(struct job_t *job)`  清除
 2. `initjobs(struct job_t *jobs)` 清除列表
 3. `maxjid(struct job_t *jobs)` 最大的jid
 4. `addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline)` 添加 成功返回1
 5. `deletejob(struct job_t *jobs, pid_t pid) ` 删除 成功返回1
 6. `pid_t fgpid(struct job_t *jobs)` 返回当前前台工作的pid
 7. `struct job_t *getjobpid(struct job_t *jobs, pid_t pid)` 通过pid找job
 8. `struct job_t *getjobjid(struct job_t *jobs, int jid)` 通过jid找job
 9. `int pid2jid(pid_t pid)`
 10. `void listjobs(struct job_t *jobs)`

# 然后根据pdf的Hint

**抄书！** 我说我怎么对这个lab没什么印象了 原来大部分是抄的。。。

找到书上的 `eval builtin_command`等 先抄一波再说 下面是书上的 大家放心抄 不会被判重的

然而并没有结束[具体见我的代码]

关于eval修改的一些个人见解
 * 阅读parseline 发现不需要copy [虽然我并没有改]
 * 根据pdf的hint第6条 eval的父 需要对SIGCHLD进行 block和unblock来避免竞争 实现同样抄第二版书上的8-37图 用sigprocmask来同步进程 但这里我们的函数要小写首字母
 * Here is the workaround: After the fork, but before the execve, the child process should call setpgid(0, 0), which puts the child in a new process group whose group ID is identical to the child’s PID. This ensures that there will be only one process, your shell, in the foreground process group. When you type ctrl-c, the shell should catch the resulting SIGINT and then forward it to the appropriate foreground job (or more precisely, the process group that contains the foreground job).
 * 然后加上根据当前的bg值来调用addjob函数(辅助函数5)

至此 我们tsh上面写的应该做的还未完成的有
 * 2.eval只是交给了`builtin_cmd`判断执行内置命令 还未实现(包括 步骤6) 
 * 4.作者提供了接受 信号的代码 未做相应控制实现
 * 7.zombie child 的控制

关于`builtin_cmd`的个人见解 总之就是执行掉内置命令 上述4个 并返回1 否则返回0
 * 加上jobs bg fg 的判断 并分发给listjobs 和`do_bgfg`即可

关于`do_bgfg`的个人见解 总之就是管理已经执行的那些程序
 * Each job can be identified by either a process ID (PID) or a job ID (JID), which is a positive integer assigned by tsh. JIDs should be denoted on the command line by the prefix ’%’. For example, “%5” denotes JID 5, and “5” denotes PID 5. (We have provided you with all of the routines you need for manipulating the job list.) 也就是用%来区分是PID还是JID
 * 得到以后就用上面两个辅助函数(7,8)来获得 我们管理的具体job
 * 唤醒进程用kill() 但要注意When you implement your signal handlers, be sure to send SIGINT and SIGTSTP signals to the entire foreground process group, using ”-pid” instead of ”pid” in the argument to the kill function. The sdriver.pl program tests for this error
 * 然后修改它的state标识

关于waitfg 的个人简介 本质就是循环等待
 * 用sleep+提供的辅助 函数6 [这样现在看上去 每次fgpid搜一遍有点耗时 有大腿的写法是通过pid找到 job然后检测job的state]

关于`sigchld_handler`个人见解 在8-17 8-33 等找到了代码
 * The waitpid, kill, fork, execve, setpgid, and sigprocmask functions will come in very handy. The WUNTRACED and WNOHANG options to waitpid will also be useful 
 * 见书上8.4.3 回收子进程部分

关于`sigint_handler` 和`sigtstp_handler` 我们的shell接受到SIGINT 我们要做的是把这个sig“转发”给fg的程序
 * 和上面一样 注意加负号

不是很理解 [fg进程最多只有一个，所以一旦产生就必须调用waitfg，`do_bgfg`函数里也是。]

有一个测试程序 如果`./checktsh.pl`除了Checking..没有额外输出 表明正确[也就是个和demo的对拍程序] 这也就是你看到我的代码中的printf格式的来源

```
./checktsh.pl 
Checking trace01.txt...
Checking trace02.txt...
Checking trace03.txt...
Checking trace04.txt...
Checking trace05.txt...
Checking trace06.txt...
Checking trace07.txt...
Checking trace08.txt...
Checking trace09.txt...
Checking trace10.txt...
Checking trace11.txt...
Checking trace12.txt...
Checking trace13.txt...
Checking trace14.txt...
Checking trace15.txt...
Checking trace16.txt...
```

# 总结

我们做一个没什么卵用的tsh
 * 你会发现前台程序除了收收 SIGINT SIGSTP 也没什么卵用 你可以试着写一个a+b的程序 然后用tsh运行它 你会发现 连输入都无法输入,waitfg 这里就真的只是干等
 * 你大概学会一个shell的实现? 但这个lab真实的目的在于 1.程序的三种状态 2.线程与信号
 * 回看最顶部 你会发现 作者实现的行数都更多 哇我用更少的代码实现了 好棒棒哦！其实正如总结的第一点 我们实现的只是针对于这个lab离最最基本的shell都还有差距 不过我认为以学习(总结第二点)的想法来说足够了 重要还是把第二点理解到
