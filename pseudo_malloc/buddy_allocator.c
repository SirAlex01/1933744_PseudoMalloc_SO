#include <stdio.h>
#include <assert.h>
#include <math.h> // for floor and log2
#include <sys/mman.h>
#include "buddy_allocator.h"

void BuddyAllocator_init(BuddyAllocator* alloc,
                         unsigned char* memory,
                         int num_levels, 
                         int max_bucket_size,
                         int min_bucket_size){

  // we need room also for level 0
  alloc->num_levels=num_levels;
  alloc->memory=memory;
  alloc->min_bucket_size=min_bucket_size;
  alloc->max_bucket_size=max_bucket_size;

  // size of the managed memory
  int mem_size=((1<<(num_levels)))*min_bucket_size;

  // number of trees with depth for each tree
  int roots = mem_size/max_bucket_size;
  int nodes_for_each_tree=(max_bucket_size/min_bucket_size)*2-1;

  // we need enough bits to store #roots trees with blocks of sizes from max to min
  int bits_needed=roots*nodes_for_each_tree;

  // we use the mmap to get enough bytes to store the bitmap
  //uint8_t bit_map_buffer[BitMap_getBytes(bits_needed)];
  
  uint8_t* bit_map_buffer = (uint8_t*) mmap(NULL,
				                               BitMap_getBytes(bits_needed),
				                               PROT_READ|PROT_WRITE,
                                       MAP_PRIVATE | MAP_ANONYMOUS,
				                               -1,
				                               0);
  assert(bit_map_buffer != MAP_FAILED);
  

  // creating the bitmap
  BitMap_init(&alloc->bit_map,bits_needed,bit_map_buffer);

  // initializing the bitmap: all blocks free :) (not for a long time though)
  for (int bit_num=0; bit_num<bits_needed; bit_num++)
    BitMap_setBit(&alloc->bit_map,bit_num,0);


  printf("BUDDY INITIALIZING\n");
  printf("\tlevels: %d", num_levels);
  printf("\tmax bucket size %d bytes\n", max_bucket_size);
  printf("\tbucket size:%d\n", min_bucket_size);
  printf("\tmanaged memory %d bytes\n", (1<<num_levels)*min_bucket_size);
  
};


int BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level){

  // calcolo dell'area di memoria gestita
  int mem_size=((1<<(alloc->num_levels)))*alloc->min_bucket_size;

  // number of roots with #nodes for each tree
  int roots = mem_size/(alloc->max_bucket_size);
  int nodes_for_each_tree=(alloc->max_bucket_size/alloc->min_bucket_size)*2-1;
  
  // numero di buddies al livello richiesto
  int level_buddies=(1<<level);

  //calcolo l'indice all'interno dell'albero del primo blocco
  int tree_index=level_buddies-1;
  int offset=0;
  //calcolo il numero di livelli di ogni albero
  int tree_levels=alloc->num_levels-log2(mem_size/alloc->max_bucket_size);

  // ciclo tra gli alberi con radice di max_bucket_size
  // l'albero è tagliato a partire dalla dimensione massima allocabile
  // cioè page_size/4
  for (int i=0;i<roots;i++) {
    // controllo in sequenza i #level_buddies buddies del livello #level
    for (int j=0;j<level_buddies;j++) {
      int buddy_idx=offset+tree_index+j;
      if (!BitMap_bit(&alloc->bit_map,buddy_idx)) {

        //il buddy è libero per l'allocazione
        BitMap_setBit(&alloc->bit_map,buddy_idx,1);

        //ora bisogna notificare l'allocazione di tutti i nodi padre
        int father_idx=(tree_index+j-1)/2;

        //comincio dal padre, se uno degli antenati è già allocato, assumo
        //quelli superiori già segnati come allocati
        while (!BitMap_bit(&alloc->bit_map,offset + father_idx) && father_idx>=0) {
          BitMap_setBit(&alloc->bit_map,offset + father_idx,0x01);
          father_idx=(father_idx-1)/2;
        }
        //per evitare la ricorsione:
        //potrei utilizzare una pila esplicita, come quelle viste durante il corso,
        //con un pool allocator, ma secondo me la seguente soluzione è più originale

        /*OLTRETUTTO RISPARMIAMO SPAZIO
        la pila contiene al più tutti i nodi dell'ultimo livello (allocando il primo)
        che per un albero profondo n è pari a 2^n.

        l'array contiene al più tutto l'albero cioè 2^(n+1)-1 elementi
        ma senza puntatori e senza l'overhead della struttura dati 

        in definitiva, la soluzione è a mio modo di vedere preferibile
        */

        //conoscendo il numero dei successori
        int num_successors=(1<<(tree_levels-level+1))-1;
        //inizializzo un array in grado di contenerne gli indici
        int successors[num_successors];
        //in cui inserisco come primo elemento l'elemento stesso
        successors[0]=tree_index+j;
        int i=0,next=1;
        while(i<num_successors) {
          //l'array viene visitato cella per cella e di ogni cella
          //vengono calcolati gli indici dei figli sinistro e destro
          int sx=2*successors[i]+1;
          int dx=sx+1;

          //se l'indice appartiene effettivamente a un figlio dell'albero corrente
          if (sx<nodes_for_each_tree) {
            successors[next]=sx;
            next++;
            successors[next]=dx;
            next++;
            //andiamo a settare i bit dei figli come occupati
            BitMap_setBit(&alloc->bit_map,offset+sx,1);
            BitMap_setBit(&alloc->bit_map,offset+dx,1);
          }
          i++;
        }
        //e di quelli figli, con l'ausilio della ricorsione
        //BuddyAllocator_setSuccessorBits(&alloc->bit_map,1,offset,tree_index+j,offset+nodes_for_each_tree);

        return buddy_idx;
      }
    }    
    offset+=nodes_for_each_tree;
  }

  return -1;
}

// funzione che effettua una visita in profondità dei nodi di un albero successori all'indice
// index, l'albero inizio al bit start e finisce al bit end
// a ogni nodo attraversato segnala come occupato il buddy
int BuddyAllocator_setSuccessorBits(BitMap* bitmap,int bit_value, int start,int index, int end) {
  //caso base: siamo arrivati all'inizio del buddy successivo, abbiamo finito
  if (start+(2*index+1)>=end)
    return 0;
  //settiamo come occupati i bit dei figli
  BitMap_setBit(bitmap, start+(index*2+1), bit_value);
  BitMap_setBit(bitmap, start+(index*2+2), bit_value);
  //poiché l'albero è binario e completo, se il figlio sinistro è fuori dai limiti, lo è anche il destro
  //se non lo è il sinistro, neppure il destro.
  if (BuddyAllocator_setSuccessorBits(bitmap,bit_value, start, index*2+1, end))
    BuddyAllocator_setSuccessorBits(bitmap,bit_value, start, index*2+2, end);
  return 1;
}

void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, int block_idx){
  int mem_size=((1<<(alloc->num_levels)))*alloc->min_bucket_size;

  // number of roots with depth for each tree
  int nodes_for_each_tree=(alloc->max_bucket_size/alloc->min_bucket_size)*2-1;

  //indice del blocco all'interno del suo albero
  int tree_idx=block_idx%nodes_for_each_tree;
  //indice di inizio dell'albero
  int offset=((int)(block_idx/nodes_for_each_tree))*nodes_for_each_tree;
  
  BitMap_setBit(&alloc->bit_map, block_idx,0);

  //indice del padre e del buddy nell'albero
  int father_idx = tree_idx>0? (tree_idx-1)/2 : -1 ;
  int buddy_idx = (tree_idx % 2)? tree_idx+1 : tree_idx-1;

  //risalgo l'albero fino alla radice se posso unire i buddies
  while (father_idx>=0 && !BitMap_bit(&alloc->bit_map,offset+buddy_idx)) {
    BitMap_setBit(&alloc->bit_map,offset+father_idx,0);
    buddy_idx=(father_idx % 2)? father_idx+1 : father_idx-1;
    father_idx= father_idx>0? (father_idx-1)/2 : -1;
  }
  //segno tutti i figli come allocabili
  //conoscendo il numero dei successori
  int tree_levels=alloc->num_levels-log2(mem_size/alloc->max_bucket_size);
  int level=log2(tree_idx+1);

  int num_successors=(1<<(tree_levels-level+1))-1;
  //inizializzo un array in grado di contenerne gli indici
  int successors[num_successors];
  //in cui inserisco come primo elemento l'elemento stesso
  successors[0]=tree_idx;
  int i=0,next=1;
  while(i<num_successors) {
    //l'array viene visitato cella per cella e di ogni cella
    //vengono calcolati gli indici dei figli sinistro e destro
    int sx=2*successors[i]+1;
    int dx=sx+1;

    //se l'indice appartiene effettivamente a un figlio dell'albero corrente
    if (sx<nodes_for_each_tree) {
      successors[next]=sx;
      next++;
      successors[next]=dx;
      next++;
       //andiamo a settare i bit dei figli come occupati
      BitMap_setBit(&alloc->bit_map,offset+sx,0);
      BitMap_setBit(&alloc->bit_map,offset+dx,0);
      }
    i++;
  }
  //segno tutti i figli come allocabili
  //BuddyAllocator_setSuccessorBits(&alloc->bit_map,0,offset,tree_idx,offset+nodes_for_each_tree);
  
}

//allocates memory
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size) {
  // we determine the level of the page

  //si verifica che la dimensione richiesta sia allocabile
  assert(size+4<=alloc->max_bucket_size && "impossibile allocare la quantità di memoria richiesta");

  //si calcolano la dimensione della memoria e il livello richiesto
  int mem_size=(1<<alloc->num_levels)*alloc->min_bucket_size;
  int  level=floor(log2(mem_size/(size+4)));

  // if the level is too small, we pad it to max
  if (level>alloc->num_levels)
    level=alloc->num_levels;

  //si sottrae il numero di livelli assenti nela bitmap: infatti, il buddy allocator, pur gestendo 1MB
  //può allocare fino a page_size/4 B. I livelli corrispondenti a blocchi più grandi sono assenti nella bitmap
  level-= floor(log2(mem_size/(alloc->max_bucket_size)));

  int level_size=alloc->max_bucket_size/(1<<(level));

  printf("requested: %d bytes, level %d \n",
         size, level);

  // we get a buddy of that size;
  int buddy=BuddyAllocator_getBuddy(alloc, level);
  if (buddy==-1)
    return 0;
  //numero di nodi dell'albero
  int nodes_for_each_tree=(alloc->max_bucket_size/alloc->min_bucket_size)*2-1;

  //indice all'interno dell'albero 
  int tree_idx=buddy%nodes_for_each_tree;
  //indice della radice dell'albero
  int offset=buddy/nodes_for_each_tree;

  //index of the first block of the level #level
  int start_idx=(1<<level)-1;

  unsigned char* res = alloc->memory+(offset*alloc->max_bucket_size+level_size*(tree_idx-start_idx));

  int* buddy_idx_saver = (int*)res;
  *buddy_idx_saver=buddy;
  
  //printf("buddy:%d, offset:%d, %d\n",buddy,(offset*alloc->max_bucket_size+level_size*(tree_idx-start_idx)),*(int*)res);
  //printf("%p\n",res+4);

  return res+4;
}
//releases allocated memory
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem) {
  
  printf("freeing %p\n", mem);

  //comportamento della vera free:
  if (!mem)
    return;

  // we retrieve the buddy from the system

  unsigned char* p=(unsigned char*) mem;
  p=p-4;

  int buddy=*(int*)p;

  // size of the managed memory
  int mem_size=(1<<(alloc->num_levels))*alloc->min_bucket_size;

  // number of trees with depth for each tree
  int roots = mem_size/alloc->max_bucket_size;
  int nodes_for_each_tree=(alloc->max_bucket_size/alloc->min_bucket_size)*2-1;

  // we need enough bits to store #roots trees with blocks of sizes from max to min
  int bits_needed=roots*nodes_for_each_tree;


  //indice all'interno dell'albero 
  int tree_idx=buddy%nodes_for_each_tree;
  //indice della radice dell'albero
  int offset=buddy/nodes_for_each_tree;
  //calcolo il livello del blocco restituito e la sua dimensione
  int level=floor(log2(tree_idx+1));
  int level_size = alloc->max_bucket_size/(1<<(level));
  //calcolo l'indice del blocco all'inizio del livello
  int start_idx=(1<<level)-1;

  //verifica della correttezza del puntatore passato:

  //validità del bit
  assert(buddy>=0 && buddy<bits_needed && "buddy non presente nella bitmap: puntatore errato!");
  //validità del puntatore
  assert(p>=alloc->memory && p < alloc->memory+mem_size && "l'indirizzo non è all'interno della memoria gestita dal buddy allocator!");
  //alineamento dell'offset puntatore
  assert(!((p-alloc->memory)%alloc->min_bucket_size) && "indirizzo di memoria non allineato!");
  //corrispondenza del puntatore a quello associato all'indice della bitmap
  assert(alloc->memory+(offset*alloc->max_bucket_size+level_size*(tree_idx-start_idx))==p && "indirizzo di memoria errato!");
  //alla bitmap deve risultare che il blocco corrispondente al puntatore sia allocato
  //altrimenti si verifica una doppia free
  assert(BitMap_bit(&alloc->bit_map,buddy) && "double free!");
  
  BuddyAllocator_releaseBuddy(alloc, buddy);
  
}

int BuddyAllocator_isMyBlock(BuddyAllocator* alloc, void* mem) {
  int mem_size=(1<<(alloc->num_levels))*alloc->min_bucket_size;
  return mem>=(void*)alloc->memory && mem < (void*)alloc->memory+mem_size;
}