Guide
---

# 知识储备

  1. 网页访问的过程 (请求发送 解析 Cookie等)
  2. c的网络访问 看书看书！ 
  3. 并发编程 看出看书
  4. proxy的工作原理

# 题目

  [官方文档](http://csapp.cs.cmu.edu/3e/proxylab.pdf)

任务为 实现一个proxy

# Solution

`git diff default:lab9-proxy finish:lab9-proxy`

分为三个部分 
 1. 串行proxy 浏览器接受用户输入网址 浏览器把请求封装并传给我们的程序 我们的程序处理并转发给服务器 得到服务器结果 再发回给浏览器 浏览器解析显示给用户
 2. 并行proxy 可以同时处理上面描述的多个请求
 3. 带有LRU cache功能 在接受请求后 如果存在被cache的 就直接返回给用户

首先关于csapp.{c/h} **大家还请务必阅读这两份代码** 主要都是对原有函数套了一层处理掉错误状态 并且有一些其它实现，仔细看书上 的代码片段`#include<sys/socket.h>`的都是 c的库里的函数 而`#include "csapp.h"`的都是作者基于系统函数实现的函数

警告:该篇章讲了很多题外话

## Part I

首先我们知道(不知道的话说明你并不满足 知识储备1 哦)

HTTP请求分为三块
 * 请求方法URI协议/版本
 * 请求头(Request Header)
 * 请求正文

访问网页的 Request Header样子为(以下以baidu为例 可以打开chrome按Ctrl+Shift+C选中Network后 刷新网页,再查看某一个的Headers)

```
GET / HTTP/1.1
Host: www.baidu.com
Connection: keep-alive
Pragma: no-cache
Cache-Control: no-cache
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.87 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Encoding: gzip, deflate, sdch, br
Accept-Language: zh-CN,zh;q=0.8,en;q=0.6
Cookie: SOMETHINGHERE
```

Note that all lines in an HTTP request end with a carriage return, ‘\r’, followed by a newline, ‘\n’. Also important is that every HTTP request is terminated by an empty line: "\r\n".

Modern web browsers will generate HTTP/1.1 requests, but your proxy should handle them and forward them as HTTP/1.0 requests.

The important request headers for this lab are the `Host`, `User-Agent`, `Connection`, and `Proxy-Connection` headers:

我们要始终用 下面三个值我们要始终用作者说的
 * `Host: 最开始两个/之间的`
 * `User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3`(作者说已经在proxy.c里提供了)
 * `Connection: close`
 * `Proxy-Connection: close`

除了以上这些 如果header里还有其它东西那就把它们不改变的一起发送 可以看到也就是Accept Cookie等等

访问可能带有端口 如http://www.cmu.edu:8080/hub/index.html 要访问8080端口而不是默认端口(80)

对于浏览器传给我们的程序 就需要我们的程序在本地监听端口 pdf说`./proxy 12345`这样来指定端口号

还给了我们一个perl程序 [大概是助教拿来方便批量测试写的:-)] 并说如果需要额外的使用端口号 请用得到的端口号+1

```
./port-for-user.pl Cromarmot
Cromarmot: 62500
```

---

知道了接受到的 那我们作为proxy的角色 要做什么？
 * 开端口 让"浏览器"发给proxy
 * 接受到分析它应该发给哪个服务器
 * 发送给服务器并等待服务器返回
 * 返回给 "浏览器"

所以我们对于浏览器来说是一个server  对于目的服务器来说 是一个client

---

开始我的抄书之旅? 发现我以前的代码都不能用了！？手上也没有**第三版**的书.

找到一个影印的 11章在952页开始 之后以3e-pdf代指该书 [pdf指该lab的pdf]

<iframe width='100%' height='600' class='preview-iframe' scrolling='no' frameborder='0' src='http://download.csdn.net/source/preview/9234961/33dc73067d7512a7d970cec5fe8870db' ></iframe>

找到一个[代码](https://github.com/Davon-Feng/CSAPP-Labs/tree/master/yzf-proxylab-handout-3e)
 * 本lab的代码为在代码上 加上 辅助我自己理解 的注释,以及部分代码修改,和修复了一些虽然能哪满分但依然存在的bug,见总结
 * 这个大腿的[关于此lab的博客](http://blog.csdn.net/u012336567/article/details/52056089)
 * [TODO 为什么我测的他的上一个lab的mm.c爆了?]


据说第三版有新的函数`getaddrinfo` `getnameinfo` 当然csapp.c也做了一个封装成首字母大写的函数
 * `getaddrinfo`能够将传入的用String变量代表的 hostnames、 host addresses、 service names和port number等转换成为socket address structure。它代替了原来的geihostbyname和getservbyname。**该函数是线程安全的，并且是可重入的函数，而且该函数与协议无关，适用IPV4 IPV6 。**
 * `getnameinfo`函数是getaddrinfo函数的逆函数，功能恰好相反

发现和以前的一些不同之处,除了`csapp.c`文件新增了很多函数,还有:

用的`sockaddr_storage`而不是`sockaddr_in` 关于区别 [查到的](http://pubs.opengroup.org/onlinepubs/009696699/basedefs/sys/socket.h.html)是

```/*
 *  Desired design of maximum size and alignment.
 */
#define _SS_MAXSIZE 128
    /* Implementation-defined maximum size. */
#define _SS_ALIGNSIZE (sizeof(int64_t))
    /* Implementation-defined desired alignment. */


/*
 *  Definitions used for sockaddr_storage structure paddings design.
 */
#define _SS_PAD1SIZE (_SS_ALIGNSIZE - sizeof(sa_family_t))
#define _SS_PAD2SIZE (_SS_MAXSIZE - (sizeof(sa_family_t)+ \
                      _SS_PAD1SIZE + _SS_ALIGNSIZE))
struct sockaddr_storage {
    sa_family_t  ss_family;  /* Address family. */
/*
 *  Following fields are implementation-defined.
 */
    char _ss_pad1[_SS_PAD1SIZE];
        /* 6-byte pad; this is to make implementation-defined
           pad up to alignment field that follows explicit in
           the data structure. */
    int64_t _ss_align;  /* Field to force desired structure
                           storage alignment. */
    char _ss_pad2[_SS_PAD2SIZE];
        /* 112-byte pad to achieve desired size,
           _SS_MAXSIZE value minus size of ss_family
           __ss_pad1, __ss_align fields is 112. */
};
```

[以及](http://stackoverflow.com/questions/16010622/reasoning-behind-c-sockets-sockaddr-and-sockaddr-storage)

```
struct sockaddr {
   unsigned short    sa_family;    // address family, AF_xxx
   char              sa_data[14];  // 14 bytes of protocol address
};


struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET, AF_INET6
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};


struct sockaddr_in6 {
    u_int16_t       sin6_family;   // address family, AF_INET6
    u_int16_t       sin6_port;     // port number, Network Byte Order
    u_int32_t       sin6_flowinfo; // IPv6 flow information
    struct in6_addr sin6_addr;     // IPv6 address
    u_int32_t       sin6_scope_id; // Scope ID
};

struct sockaddr_storage {
    sa_family_t  ss_family;     // address family

    // all this is padding, implementation specific, ignore it:
    char      __ss_pad1[_SS_PAD1SIZE];
    int64_t   __ss_align;
    char      __ss_pad2[_SS_PAD2SIZE];
};
```

虽然我通过grep找到的`/usr/include/bits/socket.h`长这样

```
struct sockaddr_storage
  {
    __SOCKADDR_COMMON (ss_);	/* Address family, etc.  */
    char __ss_padding[_SS_PADSIZE];
    __ss_aligntype __ss_align;	/* Force desired alignment.  */
  };
```

说了这么多废话 我们也不会去操作它的内部 总之就是 `_storage`比`_in`的要更有兼容性

具体实现见main函数 各个函数输入见注释 基本流程为

`argv----Open_listenfd()--->listenfd----Accept()--->connfd,clientaddr`

`clientaddr----Getnameinfo()--->hostname,port--->printf()`

`connfd--->doit()--->Close()`

写到这里我发现第三版的书上有main的代码 见 3e-pdf 992页 不过我决定还是讲一讲(其实主要还是写给我自己看的`:-)` )

接下来实现的是doit函数 它接受一个链接描述符号 connfd(connection file descriptor)参数

书上实现的是一个server (你可以在tiny/tiny.c里找到代码) 
 * 阅读tiny/tiny.c或者 3e-pdf 上的代码 它做的是 接受 在本地找文件 返回
 * 而我们 要接受它(同) 并发送给服务器(额要做的 再从服务器接受(要做的) 再返回(同)

`doit`函数
 * 首先 分析处理方法 和URI 即(GET http://www.baidu.com HTTP/1.1)部分 获取需要连接的服务器的hostname，port。这里用了`parse_uri`函数
 * 再对于客户端请求的HTTP Header进行处理，修改客户端的HTTP Header，让proxy充当客户端将信息转发给正确的服务器。这里用了`build_http_header`函数
 * 接受服务器的返回并转发给正真的请求客户端。

接受完成了

`parse_uri`函数实现
 * 没有难度 也就是字符串处理相信各位都会 这里我的代码用了[正则匹配](http://pubs.opengroup.org/onlinepubs/7908799/xsh/regex.h.html)
 * 我的正则表达式也比较简单只针对该lab

`build_http_header`根据pdf的要求要进行改动 见上面或见pdf
 * 同样是字符串处理 没有难度
 * 这里用key来比较当前的前strlen(key)个字符来判断是 是什么参数
 * 用直接的字符串 或者 sprintf+参数的形式来生成我们要的

下面再回到doit函数的发送
 * 也就用csapp.c里封装好的`Open_clientfd`
 * 发送
 * 接受
 * 返回

这样 我们第一部分也就完成了 完整的代码见proxy.I.c

测试

``` 
> make && ./driver.sh 
*** Basic ***
Starting tiny on 3161
Starting proxy on 21093
1: home.html
   Fetching ./tiny/home.html into ./.proxy using the proxy
   Fetching ./tiny/home.html into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
2: csapp.c
   Fetching ./tiny/csapp.c into ./.proxy using the proxy
   Fetching ./tiny/csapp.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
3: tiny.c
   Fetching ./tiny/tiny.c into ./.proxy using the proxy
   Fetching ./tiny/tiny.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
4: godzilla.jpg
   Fetching ./tiny/godzilla.jpg into ./.proxy using the proxy
   Fetching ./tiny/godzilla.jpg into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
5: tiny
   Fetching ./tiny/tiny into ./.proxy using the proxy
   Fetching ./tiny/tiny into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
Killing tiny and proxy
basicScore: 40/40
```

## Part II

说 书上Section 12.5.5有
 * 注意内存溢出
 * 使用 POSIX 线程，最好在线程一开始执行 `pthread_detach(pthread_self());` 这样就不用自己负责清理线程了。
 * 注意线程安全 书上的基于getaddrinfo函数的 `open_clientfd`和`open_listenfd`是线程安全的

运用了多线程的方法，通过main监听到客户端连接后，调用`Pthread_create`来创建线程，并且将connfd参数通过传值传递给线程thread函数。

注意：如果是connfd是传地址的，会出现竞争（race）问题。当然通过一定的方法可以避免，一种是我上面用到的传值，另外就是每个connfd在堆中申请一个区域存储，这样主线程和子线程就出现竞争。

下面是 基于Part I的所有改动 [在此再次推荐`vimdiff proxy.I.c proxy.II.c`]

```
//首部加上声明
void *thread(void *vargp); 

//main里 这个注释/*sequential handle the client transaction*/以后的doit和Close改为下面的
/*concurrent request*/
pthread_t tid;
Pthread_create(&tid,NULL,thread,(void *)(intptr_t)connfd);

//实现
void *thread(void *vargp){
    int connfd = (intptr_t)vargp;
    Pthread_detach(pthread_self());
    doit(connfd);
    Close(connfd);
    return NULL;
}
```

然后测试

```
> make;./driver.sh 
*** Concurrency ***
Starting tiny on port 26402
Starting proxy on port 4322
Starting the blocking NOP server on port 29160
Trying to fetch a file from the blocking nop-server
Fetching ./tiny/home.html into ./.noproxy directly from Tiny
Fetching ./tiny/home.html into ./.proxy using the proxy
Checking whether the proxy fetch succeeded
Success: Was able to fetch tiny/home.html from the proxy.
Killing tiny, proxy, and nop-server
concurrencyScore: 15/15
```

完整代码自己改 或 见我的proxy.II.c

这种实现方式也就是每次被连接 就分离出一个线程来处理 并可以同时等待下一个连接

## Part III
 * 在内存里做个LRU cache 我们只做简易的,也就是,相同请求,就不再访问,而是直接返回cache里的
 * 设置cache上限`MAX_CACHE_SIZE = 1 MiB` 避免超过限制
 * 设置每个元素上限`MAX_OBJECT_SIZE = 100 KiB`

后两步已经有define了 我们再定义个宏`CACHE_OBJS_COUNT`

顶部加上 函数声明和 结构定义

```
/*cache function*/
void cache_init();
int cache_find(char *uri);
int cache_eviction_index();
void cache_LRU(int index);
void cache_add(char *uri,char *buf);
void readerPre(int i);
void readerAfter(int i);

typedef struct {
    char cache_obj[MAX_OBJECT_SIZE];
    char cache_uri[MAXLINE];
    int LRU;
    int isEmpty;

    int readCnt;            /*count of readers*/
    sem_t wmutex;           /*protects accesses to cache*/
    sem_t rdcntmutex;       /*protects accesses to readcnt*/

}cache_block;

typedef struct {
    cache_block cacheobjs[CACHE_OBJS_COUNT];  /*ten cache blocks*/
    int cache_num;
}Cache;

Cache cache;
```

在main中加上初始化和信号忽略(注:要说正确性的话 改信号忽略在Part I 就应该加上 只是测试的时候Part I 和Part II都没有终止Tiny)

```
cache_init();
Signal(SIGPIPE,SIG_IGN);
```

顺便讲一下测试方式 
 * 启动tiny服务器
 * 启动我们的proxy
 * 通过我们的proxy去访问Tiny和直接访问Tiny对比 这其中就有不同对Tiny的操作来检测PartII和PartIII 
 * sjtu的lab测试是用的网络真实资源`_(:з」∠)_` 详细见他们的`grade.sh`

doit函数里在处理接受前 检测cache里是否有 如果有就返回

```
int cache_index;
if((cache_index=cache_find(url_store))!=-1){/*in cache then return the cache content*/
  readerPre(cache_index);
  Rio_writen(connfd,cache.cacheobjs[cache_index].cache_obj,strlen(cache.cacheobjs[cache_index].cache_obj));
  readerAfter(cache_index);
  return;
}
```

在doit函数里的访问服务器部分 同时写入cache

```
/*receive message from end server and send to the client*/
char cachebuf[MAX_OBJECT_SIZE];
int sizebuf = 0;
size_t n;
while((n=Rio_readlineb(&server_rio,buf,MAXLINE))!=0)
{
  sizebuf+=n;
  if(sizebuf < MAX_OBJECT_SIZE)  strcat(cachebuf,buf);
  Rio_writen(connfd,buf,n);
}

Close(end_serverfd);

/*store it*/
if(sizebuf < MAX_OBJECT_SIZE){
  cache_add(url_store,cachebuf);
}
```

最后实现cache的各个函数细节

这里用了读写锁 以读者优先的模式

测试

```
> make && ./driver.sh
*** Cache ***
Starting tiny on port 2544
Starting proxy on port 29983
Fetching ./tiny/tiny.c into ./.proxy using the proxy
Fetching ./tiny/home.html into ./.proxy using the proxy
Fetching ./tiny/csapp.c into ./.proxy using the proxy
Killing tiny
Fetching a cached copy of ./tiny/home.html into ./.noproxy
Success: Was able to fetch tiny/home.html from the cache.
Killing proxy
cacheScore: 15/15

totalScore: 70/70
```

关于所有区别请用vimdiff proxy.II.c proxy.III.c 查看

## 测试工具telnet curl netcat

Telnet: 不安全的 ssh，需要手动构造 HTTP 请求，如果想要测试非法的 header，这个功能就很有用
 * man telnet
 * telnet www.wdxtub.com
 * GET http://www.wdxtub.com HTTP/1.0
cURL: 会自动构建 HTTP 请求
 * curl http://www.wdxtub.com
 * 代理模式 curl --proxy lemonshark.ics.cs.cmu.edu:3092 http://www.wdxtub.com
netcat: 多用途网络工具，用法与 telnet 类似
 * nc catshark.ics.cs.cmu.edu 12345
 * GET http://www.cmu.edu/hub/index.html HTTP/1.0

As discussed in the Aside on page 964 of the CS:APP3e text, your proxy must ignore SIGPIPE signals and should deal gracefully with write operations that return EPIPE errors.

我用的chrome的 Proxy SwitchyOmega插件来 访问我的proxy端口`_(:з」∠)_`

我曾经做的还有log要求...

#其它参考

书上的`Tiny Web` 见文件`tiny/tiny.c`

#总结

首先我们的这个lab拿到了满分,我找到的代码(上面的链接)有部分的bug,都因为测试比较水而没有被检测出来，我修正了一部分变成了proxy.I/II/III.c
 * `cache_find`找到后没有做readerAfter [也就是读计数永远为正 未被检测出是因为 测试程序在找到后只有读操作没有写 所以只是读 不会发现写的锁解不开了 ]
 * `cache_find`在命中后没有做LRU 测试程序....根本就没有测LRU好吧 就测了有没有暂存
 * `cache_find`原来虽然能保证找到以后 不会被再写见上面第一个bug 因此"保证"了读的连续性 即找到以后读的就是我们要找的,但现在我改掉第一个bug以后 中间可能产生: 找到了 [被其它的覆盖了 当放开了所有读写锁的时候] 读取 ,的不连续问题 当然测试程序也测试不到这里, 但我暂时还未想好如何去改[TODO]
 * 信号SIGPIPE屏蔽在Part I就应该加上 目前我的代码也没有加

总的来说这个lab要理解的是
 * HTTP访问时候 作为client 和server分别要干什么
 * 如果用线程
 * 锁的使用(关于锁代码尚有可能存在的bug) 希望热心观众能给一个 关于LRU 实现的好的建议或代码[TODO]










