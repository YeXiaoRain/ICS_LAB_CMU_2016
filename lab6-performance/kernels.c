/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following team struct 
 */
team_t team = {
    "bovik",              /* Team name */

    "Harry Q. Bovik",     /* First member full name */
    "bovik@nowhere.edu",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
  int i,j;
  const int block = 16;
  int srcder = 1 - dim*(block-1) ; //move from the end of one column to the start of next in src
  int dstder = 1 - block - dim ;   //move from the end of one line   to the start of next in dst
  // after move block*dim 
  int srcblockder = 1 - srcder ;
  int dstblockder = dim*dim + block;

  dst += dim*(dim-1);
  for(i = 0; i < dim; i += block ){
    for(j = 0; j < dim; j++ ){
      *dst++ = *src; src += dim;
      *dst++ = *src; src += dim;
      *dst++ = *src; src += dim;
      *dst++ = *src; src += dim;
      
      *dst++ = *src; src += dim;
      *dst++ = *src; src += dim;
      *dst++ = *src; src += dim;
      *dst++ = *src; src += dim;
      
      *dst++ = *src; src += dim;
      *dst++ = *src; src += dim;
      *dst++ = *src; src += dim;
      *dst++ = *src; src += dim;
      
      *dst++ = *src; src += dim;
      *dst++ = *src; src += dim;
      *dst++ = *src; src += dim;
      *dst = *src;// 
      
      src += srcder ;
      dst += dstder ;
    }
    src += srcblockder;
    dst += dstblockder;
  }
}

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);   
    /* ... Register additional test functions here */
}


/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
    int red;
    int green;
    int blue;
    int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
static void initialize_pixel_sum(pixel_sum *sum) 
{
    sum->red = sum->green = sum->blue = 0;
    sum->num = 0;
    return;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
static void accumulate_sum(pixel_sum *sum, pixel p) 
{
    sum->red += (int) p.red;
    sum->green += (int) p.green;
    sum->blue += (int) p.blue;
    sum->num++;
    return;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) 
{
    current_pixel->red = (unsigned short) (sum.red/sum.num);
    current_pixel->green = (unsigned short) (sum.green/sum.num);
    current_pixel->blue = (unsigned short) (sum.blue/sum.num);
    return;
}

/* 
 * avg - Returns averaged pixel value at (i,j) 
 */
static pixel avg(int dim, int i, int j, pixel *src) 
{
    int ii, jj;
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for(ii = max(i-1, 0); ii <= min(i+1, dim-1); ii++) 
	for(jj = max(j-1, 0); jj <= min(j+1, dim-1); jj++) 
	    accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

    assign_sum_to_pixel(&current_pixel, sum);
    return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/

/*
 * naive_smooth - The naive baseline version of smooth 
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(i, j, dim)] = avg(dim, i, j, src);
}

/*
 * smooth - Your current working version of smooth. 
 * IMPORTANT: This is the version you will be graded on
 */
#define R(exp) exp.red 
#define B(exp) exp.blue 
#define G(exp) exp.green
char smooth_descr[] = "smooth: Current working version";
void smooth(int dim, pixel *src, pixel *dst) 
{
    //UpLeft
    R(dst[0])           = (R(src[0])             +R(src[1])              +R(src[dim])         +R(src[dim +1])         ) >> 2;
    G(dst[0])           = (G(src[0])             +G(src[1])              +G(src[dim])         +G(src[dim +1])         ) >> 2;
    B(dst[0])           = (B(src[0])             +B(src[1])              +B(src[dim])         +B(src[dim +1])         ) >> 2;
    //UpRight
    R(dst[dim-1])       = (R(src[dim-2])         +R(src[dim-1])          +R(src[dim*2-2])     +R(src[dim*2-1])        ) >> 2;
    G(dst[dim-1])       = (G(src[dim-2])         +G(src[dim-1])          +G(src[dim*2-2])     +G(src[dim*2-1])        ) >> 2;
    B(dst[dim-1])       = (B(src[dim-2])         +B(src[dim-1])          +B(src[dim*2-2])     +B(src[dim*2-1])        ) >> 2;
    //DownLeft
    R(dst[dim*(dim-1)]) = (R(src[dim*(dim-2)])   +R(src[dim*(dim-2) +1]) +R(src[dim*(dim-1)]) +R(src[dim*(dim-1) +1]) ) >> 2;
    G(dst[dim*(dim-1)]) = (G(src[dim*(dim-2)])   +G(src[dim*(dim-2) +1]) +G(src[dim*(dim-1)]) +G(src[dim*(dim-1) +1]) ) >> 2;
    B(dst[dim*(dim-1)]) = (B(src[dim*(dim-2)])   +B(src[dim*(dim-2) +1]) +B(src[dim*(dim-1)]) +B(src[dim*(dim-1) +1]) ) >> 2;
    //DownRight
    R(dst[dim*dim-1])   = (R(src[dim*(dim-1)-2]) +R(src[dim*(dim-1)-1])  +R(src[dim*dim-2])   +R(src[dim*dim-1])      ) >> 2;
    G(dst[dim*dim-1])   = (G(src[dim*(dim-1)-2]) +G(src[dim*(dim-1)-1])  +G(src[dim*dim-2])   +G(src[dim*dim-1])      ) >> 2;
    B(dst[dim*dim-1])   = (B(src[dim*(dim-1)-2]) +B(src[dim*(dim-1)-1])  +B(src[dim*dim-2])   +B(src[dim*dim-1])      ) >> 2;

    //UpEdge
    for (int j =1; j < dim-1; j++){
        R(dst[j]) =(R(src[j]) +R(src[j-1]) +R(src[j+1]) +R(src[j+dim]) +R(src[j+1+dim]) +R(src[j-1+dim])) / 6;
        G(dst[j]) =(G(src[j]) +G(src[j-1]) +G(src[j+1]) +G(src[j+dim]) +G(src[j+1+dim]) +G(src[j-1+dim])) / 6;
        B(dst[j]) =(B(src[j]) +B(src[j-1]) +B(src[j+1]) +B(src[j+dim]) +B(src[j+1+dim]) +B(src[j-1+dim])) / 6;
    }
    //DownEdge
    for (int j =dim * (dim-1)+1; j < dim * dim-1; j++){
        R(dst[j]) =(R(src[j]) +R(src[j-1]) +R(src[j+1]) +R(src[j-dim]) +R(src[j+1-dim]) +R(src[j-1-dim])) / 6;
        G(dst[j]) =(G(src[j]) +G(src[j-1]) +G(src[j+1]) +G(src[j-dim]) +G(src[j+1-dim]) +G(src[j-1-dim])) / 6;
        B(dst[j]) =(B(src[j]) +B(src[j-1]) +B(src[j+1]) +B(src[j-dim]) +B(src[j+1-dim]) +B(src[j-1-dim])) / 6;
    }
    //LeftEdge
    for (int j =dim; j < dim * (dim-1); j+=dim){
        R(dst[j]) =(R(src[j]) +R(src[j-dim]) +R(src[j+1]) +R(src[j+dim]) +R(src[j+1+dim]) +R(src[j-dim+1])) / 6;
        G(dst[j]) =(G(src[j]) +G(src[j-dim]) +G(src[j+1]) +G(src[j+dim]) +G(src[j+1+dim]) +G(src[j-dim+1])) / 6;
        B(dst[j]) =(B(src[j]) +B(src[j-dim]) +B(src[j+1]) +B(src[j+dim]) +B(src[j+1+dim]) +B(src[j-dim+1])) / 6;
    }
    //RightEdge
    for (int j =dim+dim-1; j < dim * dim-1; j+=dim){
        R(dst[j]) =(R(src[j]) +R(src[j-1]) +R(src[j-dim]) +R(src[j+dim]) +R(src[j-dim-1]) +R(src[j-1+dim])) / 6;
        G(dst[j]) =(G(src[j]) +G(src[j-1]) +G(src[j-dim]) +G(src[j+dim]) +G(src[j-dim-1]) +G(src[j-1+dim])) / 6;
        B(dst[j]) =(B(src[j]) +B(src[j-1]) +B(src[j-dim]) +B(src[j+dim]) +B(src[j-dim-1]) +B(src[j-1+dim])) / 6;
    }

    //Center
    int tmpi = dim;
    for (int i = 1; i < dim - 1; i++){
        for (int j = 1; j < dim - 1; j++){
            int tmp = tmpi + j;
            R(dst[tmp]) = (R(src[tmp - dim - 1]) +R(src[tmp - dim]) +R(src[tmp - dim + 1]) +
                           R(src[tmp - 1])       +R(src[tmp])       +R(src[tmp + 1])       +
                           R(src[tmp + dim - 1]) +R(src[tmp + dim]) +R(src[tmp + dim + 1]) ) / 9;
            G(dst[tmp]) = (G(src[tmp - dim - 1]) +G(src[tmp - dim]) +G(src[tmp - dim + 1]) +
                           G(src[tmp - 1])       +G(src[tmp])       +G(src[tmp + 1])       +
                           G(src[tmp + dim - 1]) +G(src[tmp + dim]) +G(src[tmp + dim + 1]) ) / 9;
            B(dst[tmp]) = (B(src[tmp - dim - 1]) +B(src[tmp - dim]) +B(src[tmp - dim + 1]) +
                           B(src[tmp - 1])       +B(src[tmp])       +B(src[tmp + 1])       +
                           B(src[tmp + dim - 1]) +B(src[tmp + dim]) +B(src[tmp + dim + 1]) ) / 9;
        }
        tmpi += dim;
    }
}


/********************************************************************* 
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_smooth_functions() {
    add_smooth_function(&smooth, smooth_descr);
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    /* ... Register additional test functions here */
}

