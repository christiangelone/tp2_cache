#include <stdlib.h>
#include <stdio.h>

typedef struct queue {
  int rear;
  unsigned int front, size;
  unsigned int capacity; 
  unsigned int * array; 
} queue;

struct queue init_queue(unsigned capacity);
void free_queue(struct queue * q);
void print_queue(struct queue q, char label);
int is_full(struct queue * q);
int is_empty(struct queue * q);
unsigned int peek(struct queue q);
void dequeue(struct queue * q);
void enqueue(struct queue * q, unsigned int item);