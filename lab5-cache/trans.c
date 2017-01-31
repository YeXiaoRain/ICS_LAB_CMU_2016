/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
  int i,j;
  int q,k;
  int tmp;
  int tmp1,tmp2,tmp3,tmp4;
  if(M==32){//{{{
    //12 * 16 = 192miss
    // 287-192=95
    for(q=0;q<4;q++)
      for(k=0;k<4;k++)
        if(q==k){
          for(i=8*q;i<8*(q+1);i++){
            for(j=8*k;j<8*(k+1);j++){
              if(i==j)
                continue;
              else
                B[j][i]=A[i][j];
            }
            B[i][i]=A[i][i];
          }
        }else{
          for(i=8*q;i<8*(q+1);i++)
            for(j=8*k;j<8*(k+1);j++)
              B[j][i]=A[i][j];
        }//}}}
  }else if(M==64){//{{{
    for(q=0;q<8;q++)
      for(k=0;k<8;k++)
        if(q==k){
          for(i=8*q;i<8*q+4;i++){
            for(j=8*k;j<8*k+4;j++)
              if(!((i-j+4)%4)){
                tmp=j;
                continue;
              }
              else
                B[j][i]=A[i][j];
            B[tmp][i]=A[i][tmp];
          }
          for(i=8*q+4;i<8*q+8;i++){
            for(j=8*k;j<8*k+4;j++)
              if(!((i-j+4)%4)){
                tmp=j;
                continue;
              }
              else
                B[j][i]=A[i][j];
            B[tmp][i]=A[i][tmp];
          }
          for(i=8*q+4;i<8*q+8;i++){
            for(j=8*k+4;j<8*k+8;j++)
              if(!((i-j+4)%4)){
                tmp=j;
                continue;
              }
              else
                B[j][i]=A[i][j];
            B[tmp][i]=A[i][tmp];
          }
          for(i=8*q;i<8*q+4;i++){
            for(j=8*k+4;j<8*k+8;j++)
              if(!((i-j+4)%4)){
                tmp=j;
                continue;
              }
              else
                B[j][i]=A[i][j];
            B[tmp][i]=A[i][tmp];
          }
          /*
             for(i=8*q;i<8*(q+1);i++){
             for(j=8*k;j<8*(k+1);j++){
             if(i==j)
             continue;
             else
             B[j][i]=A[i][j];
             }
             B[i][i]=A[i][i];
             }*/
        }else{
          for(i=8*q;i<8*q+4;i++)
            for(j=8*k;j<8*k+4;j++)
              B[j][i]=A[i][j];
          for(i=8*q;i<8*q+4;i++)
            for(j=8*k+4;j<8*k+8;j++)
              B[j-4][i+4]=A[i][j];
          for(j=8*k;j<8*k+4;j++){
            tmp1=B[j][8*q+4];
            tmp2=B[j][8*q+5];
            tmp3=B[j][8*q+6];
            tmp4=B[j][8*q+7];
            for(i=8*q+4;i<8*q+8;i++)
              B[j][i]=A[i][j];
            B[j+4][8*q]=tmp1;
            B[j+4][8*q+1]=tmp2;
            B[j+4][8*q+2]=tmp3;
            B[j+4][8*q+3]=tmp4;
            for(i=8*q+4;i<8*q+8;i++)
              B[j+4][i]=A[i][j+4];
          }
        }
  }//}}}
  else if(M==61){//{{{
    for(q=0;q<4;q++)
      for(k=0;k<4;k++)
        for(i=17*q;i<17*(q+1) && i<67;i++)
          for(j=17*k;j<17*(k+1) && j<61;j++)
            B[j][i]=A[i][j];
  }//}}}
  return ;
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

