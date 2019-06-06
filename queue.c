#include "queue.h"

struct queue init_queue(unsigned capacity) { 
  struct queue q; 
  q.capacity = capacity; 
  q.front = q.size = 0;  
  q.rear = -1;
  q.array = (unsigned int *) malloc(q.capacity * sizeof(int)); 
  return q;
}

void free_queue(struct queue * q) {
  free(q->array);
};

void print_queue(struct queue q, char label) {
  printf("<");
  for(int i = q.front; i < q.capacity; i++) printf("%c%d,", label, q.array[i]);
  printf("]");
}

// Queue is full when size becomes equal to the capacity  
int is_full(struct queue * q) {
  return (q->size == q->capacity);
} 

int is_empty(struct queue * q) {
  return (q->size == 0);
}

unsigned int peek(struct queue q) {
  if (is_empty(&q)) return 0;
  return q.array[q.front];
}

void dequeue(struct queue * q) {
  if (!is_empty(q)) {
    q->front++;
    if(q->front == q->capacity) q->front = 0;
    q->size = q->size - 1;
  }
}

void enqueue(struct queue * q, unsigned int item) { 
  if (!is_full(q)){
    if(q->rear + 1 == q->capacity) q->rear = -1;
    q->array[++q->rear] = item; 
    q->size = q->size + 1;
  }
} 