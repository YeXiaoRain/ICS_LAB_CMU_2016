#include "cachelab.h"
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

/* addr = | tag |  index | offset |
 *        |     |  1<<s  |  1<<b  |
 * */
#define TAG(addr)    (addr >> (s+b))
#define INDEX(addr)  ((addr>>b) & ((1<<s)-1))
//#define OFFSET(addr) (addr & ((1<<b)-1))

#define INVALID      (1<<(sysbits-s-b))

typedef unsigned long long ull;

//64-bits system
int sysbits=64;
//config s-index bits, E-associativity,b-number of block bits
int s=-1,E=-1,b=-1;
int show_detail;// c do not have bool and true/false
//result
int hit_times,miss_times,eviction_times;
ull ** datasaver;

void clean_datasaver(){//{{{
  for(int i=0 ; i < (1<<s) ; i++)
    for(int j=0 ; j<E ; j++ )
      datasaver[i][j] = INVALID;
  miss_times=hit_times=eviction_times=0;
  return ;
}//}}}

void alloc_cache(){//{{{
  int i,maxi=1<<s;
  datasaver = (ull **) malloc(sizeof(ull *) * maxi);
  for(i=0;i<maxi;i++)
    datasaver[i] = (ull *)malloc(sizeof(ull) * E);

  clean_datasaver();
  return ;
}//}}}

void destroy_cache(){//{{{
  for(int i=0;i<(1<<s);i++)
    free(datasaver[i]);
  free(datasaver);
  return ;
}//}}}

/* 模拟访问index组
 * 对比tag来看访问的是否是我们需要的
 * 使用LRU在多路中重排和剔除
 * 返回 用二进制编码 低位1表示eviction 高位1表示miss
 * */
int do_code(ull tag,ull index){//{{{
  int i;
  ull temp;
  for(i=0;i<E;i++){
    temp=datasaver[index][i];
    if(temp==tag || temp==INVALID)
      break ;
  }
  //check hit/miss & eviction
  int return_code=0;
  if(i==E){//miss eviction
    return_code=3; // 11b
    i=0;
  }else if(temp==tag){//hit move
    return_code=0; // 00b
  }else{//miss
    return_code=2; // 10b
  }
  //LRU
  for(;i<E-1;i++){
    temp=datasaver[index][i]=datasaver[index][i+1];	
    if(temp==INVALID)
      break;
  }
  datasaver[index][i]=tag;
  return return_code;		

}//}}}

/* execute_code
 * 对于剩下三种 M S L 
 * 把M单独的hit提出 剩下交给do_code
 * 并判断是否需要输出
 * */
void execute_code(char op,ull addr,int length){//{{{
  ull tag    = TAG(   addr);
  ull index  = INDEX( addr);
  //ull offset = OFFSET(addr);
  int result=do_code(tag,index);
  //decode
  if(!result)
    hit_times++;
  if(result >> 1)
    miss_times++;
  if(result & 1)
    eviction_times++;
  
  if(op=='M')
    hit_times++;

  if(show_detail){
    printf("%c %llx,%d",op,addr,length);
    //decode
    if(!result)
      printf(" hit");
    if(result >> 1)
      printf(" miss");
    if(result & 1)
      printf(" eviction");
    if(op=='M')
      printf(" hit");
    printf("\n");
  }
  return ;

}//}}}

/* read_code
 * 读文件内容 并分离参数 op,val,size
 * 忽略I指令
 * 把剩余的指令交给execute(op,val,size)
 * */
void read_code(char * F_PATH){//{{{
  FILE * fin = NULL;
  if((fin=fopen(F_PATH,"r"))==NULL){
    printf("cannot open this file\n");
  }
  char op;
  ull addr;
  int length;
  while(fscanf(fin," %c %llx,%d",&op,&addr,&length)!=EOF){
    if(op=='I')
      continue;
    execute_code(op,addr,length);
  }
  fclose(fin);
  return ;
}//}}}

void show_help(char * argv0){//{{{
  printf("\
      Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n\
      Options:\n\
      \t-h         Print this help message.\n\
      \t-v         Optional verbose flag.\n\
      \t-s <num>   Number of set index bits.\n\
      \t-E <num>   Number of lines per set.\n\
      \t-b <num>   Number of block offset bits.\n\
      \t-t <file>  Trace file.\n\
      \n\
      Examples:\n\
      \tlinux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n\
      \tlinux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n",argv0,argv0);
}//}}}

int main(int argc,char *argv[]){
  int ch;
  //opterr=0;
  int input_index=-1;
  while((ch = getopt(argc,argv,"hvs:E:b:t:"))!=-1){
    switch(ch){
      case 'v':
        show_detail=1;
        break;
      case 's':
        s = atoi(optarg);
        break;
      case 'E':
        E = atoi(optarg);
        break;
      case 'b':
        b = atoi(optarg);
        break;
      case 't':
        input_index=optind-1;
        break;
      case 'h':
      default:
        show_help(argv[0]);
        return 0;
        break;
    }
    //printf("optind:%d\n",optind);
    //printf("optarg:%s\n",optarg);
    //printf("opterr:%d\n",opterr);
  }
  if((s | E | b | input_index) == -1 ){
    show_help(argv[0]);
    return 0;
  }

  alloc_cache();
  read_code(argv[input_index]);
  destroy_cache();

  printSummary(hit_times,miss_times,eviction_times);
  return 0;
}
