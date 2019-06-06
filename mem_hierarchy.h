#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include "queue.h"

#define ADDRESS_SIZE 32 // bits

#define RAM_SIZE 16 * 1024 // Bytes

#define CACHE_SIZE 2 * 1024 // Bytes
#define NUM_OF_WAYS 4
#define BLOCKSIZE 64 // Bytes
#define NUM_OF_BLOCKS_PER_WAY CACHE_SIZE / (NUM_OF_WAYS * BLOCKSIZE)
#define OFFSET_SIZE 6 // bits
#define INDEX_SIZE 3
#define TAG_SIZE ADDRESS_SIZE - (INDEX_SIZE + OFFSET_SIZE) // bits

typedef struct block {
  bool has_data;
  unsigned int tag;
  unsigned char (* content)[BLOCKSIZE]; // Here is where data is stored
} block;

typedef struct hit {
  bool is;
  int way_number;
} hit;

typedef struct way {
  block (* blocks)[NUM_OF_BLOCKS_PER_WAY];
} way;

typedef struct nways_cache {
  way (* ways)[NUM_OF_WAYS];
  queue (* fifo)[NUM_OF_BLOCKS_PER_WAY];
} nways_cache;

typedef struct main_memory {
  unsigned char (* content)[RAM_SIZE];
} main_memory;

typedef struct memory_hierarchy {
  unsigned int * cache_accesses;
  unsigned int * cache_misses;
  nways_cache * cache;
  main_memory ram;
} memory_hierarchy;

memory_hierarchy * init();
unsigned int get_offset (unsigned int address);
unsigned int get_set(unsigned int address);
unsigned int get_tag(unsigned int address);
unsigned int select_oldest(nways_cache * cache, unsigned int setnum);
block read_tocache(memory_hierarchy * hierarchy, unsigned int address, unsigned int waynum, unsigned int setnum);
unsigned char read_byte(memory_hierarchy * hierarchy, unsigned int address);
void write_byte(memory_hierarchy * hierarchy, unsigned int address, unsigned char value);
float get_miss_rate(memory_hierarchy * hierarchy);

// Extra
void flush_cache(memory_hierarchy * hierarchy);
void print_cache(nways_cache * cache);
void print_ram(main_memory ram);
void print_hierarchy(memory_hierarchy * hierarchy, bool pcache, bool pram);
hit hit_cache(nways_cache * cache, unsigned int set_idx, unsigned int tag);
void destroy(memory_hierarchy * hierarchy);