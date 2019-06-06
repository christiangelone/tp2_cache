#include "mem_hierarchy.h"

nways_cache * init_cache() {
  nways_cache * cache = malloc(sizeof(nways_cache));

  cache->fifo = malloc(NUM_OF_BLOCKS_PER_WAY * sizeof(queue));
  for(int f = 0; f < NUM_OF_BLOCKS_PER_WAY; f++) {
    queue q = init_queue(NUM_OF_WAYS);
    enqueue(&q, 0);
    enqueue(&q, 1);
    enqueue(&q, 2);
    enqueue(&q, 3);
    (* cache->fifo)[f] = q;
  }

  cache->ways = malloc(NUM_OF_WAYS * sizeof(way));
  for (int i = 0; i < NUM_OF_WAYS; i++) {
    way a_way;
    a_way.blocks = malloc(NUM_OF_BLOCKS_PER_WAY * sizeof(block));
    for (int j = 0; j < NUM_OF_BLOCKS_PER_WAY; j++) {
      block a_block;
      a_block.content = NULL;
      a_block.tag = 0;
      a_block.has_data = false;
      (* a_way.blocks)[j] = a_block;
    }
    (* cache->ways)[i] = a_way;
  }

  return cache;
};

main_memory init_ram() {
  main_memory ram;
  ram.content = malloc(RAM_SIZE * sizeof(char));

  srand(time(NULL));
  for(int i = 0; i < RAM_SIZE; i++) (* ram.content)[i] = '&';

  return ram;
};

memory_hierarchy * init() {
  memory_hierarchy * hierarchy = malloc(sizeof(memory_hierarchy));

  hierarchy->cache_accesses = malloc(sizeof(int));
  * hierarchy->cache_accesses = 0;
  
  hierarchy->cache_misses = malloc(sizeof(int));
  * hierarchy->cache_misses = 0;
  
  hierarchy->cache = init_cache();
  
  (* hierarchy).ram = init_ram();

  return hierarchy;
}

void destroy_cache(nways_cache * cache){
  for(int f = 0; f < NUM_OF_BLOCKS_PER_WAY; f++) free_queue(&(* cache->fifo)[f]);
  free(cache->fifo);
  
  for(int i = 0; i < NUM_OF_WAYS; i++) {
    way a_way = (* cache->ways)[i];
    free(a_way.blocks);
  }

  free(cache->ways);
  free(cache);
};

void destroy_ram(main_memory ram) {
  free(ram.content);
};

void destroy(memory_hierarchy * hierarchy) {
  destroy_cache(hierarchy->cache);
  destroy_ram(hierarchy->ram);
  free(hierarchy->cache_accesses);
  free(hierarchy->cache_misses);
  free(hierarchy);
};

void print_cache(nways_cache * cache) {
  puts("\nCACHE ====================================================================================================");
  printf("      ");
  for(int f = 0; f < NUM_OF_BLOCKS_PER_WAY; f++) printf("   SET%d:     ", f);
  printf("\n        \nFIFO:   ");
  for(int f = 0; f < NUM_OF_BLOCKS_PER_WAY; f++) {
    print_queue((* cache->fifo)[f], 'W');
    printf("        ");
  }
  printf("\n     \n     ");
  for(int f = 0; f < NUM_OF_BLOCKS_PER_WAY; f++)
    printf("  old: W%d    ", select_oldest(cache, f));
  puts("\n");

  for(int i = 0; i < NUM_OF_WAYS; i++) {
    way a_way = (* cache->ways)[i];
    printf("  W%d: ", i);
    for(int j = 0; j < NUM_OF_BLOCKS_PER_WAY; j++) {
      block a_block = (* a_way.blocks)[j];
      printf(" [B%d]: %s", j, (* a_block.content));
    }
    puts("\n");
  }
  puts("CACHE ====================================================================================================");
};

void print_ram(main_memory ram) {
  puts("RAM ======================================================================================================");
  for(int i = 0, j = 0, b = 0, w = 0; i < RAM_SIZE; i++) {
    if(i % BLOCKSIZE == 0) {
      if(i > 0) printf("\n\n");
      printf(" [B%d]:", j);
      j++;
    }
    if(i % 4 == 0) {
      printf("\n (word%d):", w);
      w++;
    }
    if(i % 1 == 0) {
      printf(" (byte%d): ", b);
      b++;
    }
    printf("%c", (* ram.content)[i]);
  }
  printf("\n");
  puts("RAM =======================================================================================================");
};

void print_hierarchy(memory_hierarchy * hierarchy, bool pcache, bool pram) {
  if(pcache){
    print_cache(hierarchy->cache);
    printf("\n");
  }
  if(pram) print_ram((* hierarchy).ram);
}

void flush_cache(memory_hierarchy * hierarchy){
  destroy_cache(hierarchy->cache);
  hierarchy->cache = init_cache();
  puts("[CACHE FLUSHED]");
};

// We are going to use a (4 bytes or 32 bits) address [ 23 bits tag | 3 bits index | 6 bits offset]
unsigned int get_offset (unsigned int address) {
  return address & 0x0000003f;
};

unsigned int get_set(unsigned int address) {
  unsigned int idx = address & 0x000001c0;
  return (idx >> OFFSET_SIZE);
};

unsigned int get_tag(unsigned int address) {
  unsigned int tag = address & 0xfffffe00;
  return (tag >> (INDEX_SIZE + OFFSET_SIZE));
};

unsigned int select_oldest(nways_cache * cache, unsigned int setnum) {
  return peek((* cache->fifo)[setnum]);
};

hit hit_cache(nways_cache * cache, unsigned int set_idx, unsigned int tag) {
  hit has_hit;
  has_hit.is = false;
  has_hit.way_number = -1;

  for (int i = 0; i < NUM_OF_WAYS && !has_hit.is; i++) {
    block target_block = (*(* cache->ways)[i].blocks)[set_idx];
    //printf("has_data: %d, target_tag == tag ?: %d\n", target_block.has_data, target_block.tag == tag);
    if (target_block.has_data && target_block.tag == tag) {
      has_hit.is = true;
      has_hit.way_number = i;
    }
  }

  return has_hit;
};

block read_tocache(memory_hierarchy * hierarchy, unsigned int address, unsigned int waynum, unsigned int setnum) {
  unsigned char (* data)[BLOCKSIZE] = malloc(BLOCKSIZE * sizeof(char));

  for (int i = 0, j = address; (j < address + BLOCKSIZE) && j < RAM_SIZE ; i++, j++)
    (* data)[i] = (* hierarchy->ram.content)[j];
  
  block a_block;
  a_block.has_data = true;
  a_block.tag = get_tag(address);
  a_block.content = data;

  way a_way = (* hierarchy->cache->ways)[waynum];
  (* a_way.blocks)[setnum] = a_block;
  return a_block;
};

unsigned char read_byte(memory_hierarchy * hierarchy, unsigned int address) {
  const unsigned int tag = get_tag(address),
                     set_idx = get_set(address),
                     offset = get_offset(address);

  (* hierarchy->cache_accesses)++;
  hit cache_hit = hit_cache(hierarchy->cache, set_idx, tag);

  if (cache_hit.is) {
    const unsigned char byte_read = (*(*(* hierarchy->cache->ways)[cache_hit.way_number].blocks)[set_idx].content)[offset];
    printf("[HIT] read_byte: Read from cache (value: '%c', tag: %d, set: %d, offset: %d)\n", byte_read, tag, set_idx, offset);
    enqueue(&(* hierarchy->cache->fifo)[set_idx], cache_hit.way_number);
    return byte_read;
  } else {
    const unsigned int oldest_way = select_oldest(hierarchy->cache, set_idx);
    dequeue(&(* hierarchy->cache->fifo)[set_idx]);
    const unsigned char byte_read = (* read_tocache(hierarchy, address, oldest_way, set_idx).content)[offset];
    (* hierarchy->cache_misses)++;
    printf("[MISS] read_byte: Read from memory (address: 0x%08x, value: '%c') and block bringed to cache (tag: %d, set: %d, offset: %d)\n", address, byte_read, tag, set_idx, offset);
    return byte_read;
  }
};

// This function considers a WT/WA approach
void write_byte(memory_hierarchy * hierarchy, unsigned int address, unsigned char value) {
  const unsigned int tag = get_tag(address),
                     set_idx = get_set(address),
                     offset = get_offset(address);

  (* hierarchy->cache_accesses)++;
  hit cache_hit = hit_cache(hierarchy->cache, set_idx, tag);
  if (cache_hit.is) {
    (*(*(* hierarchy->cache->ways)[cache_hit.way_number].blocks)[set_idx].content)[offset] = value;
    printf("[HIT] write_byte: Wrote (value: '%c') in cache (tag: %d, set: %d, offset: %d)\n", value, tag, set_idx, offset);
    enqueue(&(* hierarchy->cache->fifo)[set_idx], cache_hit.way_number);
  } else {
    const unsigned int oldest_way = select_oldest(hierarchy->cache, set_idx);
    dequeue(&(* hierarchy->cache->fifo)[set_idx]);
    read_tocache(hierarchy, address, oldest_way, set_idx);
    (*(*(* hierarchy->cache->ways)[oldest_way].blocks)[set_idx].content)[offset] = value;
    printf("[MISS] write_byte: Block bringed to cache and wrote (value: '%c') in cache (tag: %d, set: %d, offset: %d)\n", value, tag, set_idx, offset);
    (* hierarchy->cache_misses)++;
  }

  (* (* hierarchy).ram.content)[address] = value;
  printf("  Wrote (value: '%c') in memory (address: 0x%08x)\n", value, address);
};

float get_miss_rate(memory_hierarchy * hierarchy) {
  if(* hierarchy->cache_accesses == 0) return 0;
  return ((float)(* hierarchy->cache_misses) / (float)(* hierarchy->cache_accesses));
};