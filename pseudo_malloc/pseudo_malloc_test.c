#include "pseudo_malloc.h"
#include <stdio.h>

int main(){
    void* p1=pseudo_malloc(100);
    void* p2=pseudo_malloc(1000);
    void* p3=pseudo_malloc(10000);

    printf("%p,%p,%p\n",p1,p2,p3);
    pseudo_free(p1);
    pseudo_free(p2);
    pseudo_free(p3);

    p1=pseudo_malloc(100000000);
    p3=pseudo_malloc(1);
    p2=pseudo_malloc(1019);

    pseudo_free(p1);
    //pseudo_free(p1+1);

    p1=pseudo_malloc(510);

    pseudo_free(p1);
    //pseudo_free(p1+1);
    pseudo_free(p2);
    pseudo_free(p3);
    /*
    for (int i=0;i<=(1024);i++) {
      p1=pseudo_malloc(509);
      printf("%p\n",p1);
    }
    */
      
}
