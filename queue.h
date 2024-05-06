//SSOO-P3 23/24
#include <pthread.h>

#ifndef HEADER_FILE
#define HEADER_FILE

// Structure of the element to be stored in the queue
typedef struct element {
  int product_id; //Product identifier
  int op;         //Operation
  int units;      //Product units
}element;

// Structure of the queue
typedef struct queue {
  // Array to store the elements
  element *elements;
  // Maximum capacity of the queue
  int capacity;
  // Index of the front element
  int front;
  // Index of the tail element
  int tail;
  // Current number of elements in the queue
  int size;
  // Mutex
  pthread_mutex_t mutex;
  // Condition variable to check that the queue is not full
  pthread_cond_t not_full;
  // Condition variable to check that the queue is not empty
  pthread_cond_t not_empty;
}queue;

queue* queue_init (int size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct element* elem);
struct element * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);

#endif
