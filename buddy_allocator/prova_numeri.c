#include "math.h"
#include "stdio.h"

int main() {

    int min_bucket_size=8;
    int max_bucket_size=1024;
    int num_levels = log(1024*1024/8)/log(2);
    int mem_size=((1<<(num_levels)))*min_bucket_size;

  // number of trees with depth for each tree
     int roots = mem_size/max_bucket_size;
     int nodes_for_each_tree=(max_bucket_size/min_bucket_size)*2-1;
     int depth_for_each_tree=log(max_bucket_size/min_bucket_size)/log(2);

     // we need enough bits to store #roots trees with blocks of sizes from max to min
     int bits_needed=roots*nodes_for_each_tree;

     printf("%d,%d,%d,%d,%d,%d\n",num_levels,mem_size,roots,nodes_for_each_tree,depth_for_each_tree,bits_needed);


}