//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"

//To create a queue and reserve the specified size as a parameter
queue* queue_init(int size){
  // Allocate memory for the queue structure
  queue * q = (queue *)malloc(sizeof(queue));
  if (q == NULL){
    perror("Memory allocation failed.\n");
    exit(EXIT_FAILURE);
  }

  // Allocate memory for the array to store the elements
  q->elements = malloc(size * sizeof(struct element));
  if (q->elements == NULL){
    perror("Memory allocation failed.\n");
    // We free the previously allocated memory, as there has been an error
    free(q);
    exit(EXIT_FAILURE);
  }

  // Initialize the rest of the queue structure properties
  q->capacity = size;   // Maximum capacity of the queue
  q->front = 0;         // Index of the front element
  q->tail = 0;         // Index of the tail element
  q->size = 0;          // Current number of elements on the queue

  // Initialize mutex and condition variable
  // THis is before the mutex and the condition variable
  // are used or locked.
  pthread_mutex_init(&q->mutex, NULL);
  pthread_cond_init(&q->not_empty, NULL);
  pthread_cond_init(&q->not_full, NULL);
  return q;
}

// To Enqueue an element, if there is no space available
// it must wait until the insertion can be done.
// The enqued element will be at the tail
int queue_put(queue *q, struct element* x){
  // Thread acquiring lock on the mutex 
  // before accessing the shared resource
  pthread_mutex_lock(&q->mutex);

  // If the queue is full, we will need to wait until
  // there is available space
  while(q->size == q->capacity){
    pthread_cond_wait(&q->not_empty, &q->mutex);
  }

  // Insert element in the queue when there is free space
  //q->elements[q->tail] = *x;
  // This is due to the circular schema of our buffer
  //q->tail = (q->tail + 1) % q->capacity;
  q->elements[(q->tail) % q->capacity] = *x;
  q->tail++;
  q->size++;

  // Each time an element is enqueued we signal the not_empty
  // condition variable, so that if the buffer was empty, and 
  // the dequeing had to wait, it can now go on
  pthread_cond_signal(&q->not_full);

  // When the thread has already enqueued an element (using
  // shared resource), we release the lock
  pthread_mutex_unlock(&q->mutex);

  return 0;
}

// To Dequeue an element. It returns the eliminated element.
// The dequeued element will be the one in the front.
struct element* queue_get(queue *q){
  

  // Thread acquiring lock on the mutex 
  // before accessing the shared resource
  pthread_mutex_lock(&q->mutex);

  // If the queue is empty we will need to wait
  // until an element is enqueued.
  while(q->size == 0){
    pthread_cond_wait(&q->not_full, &q->mutex);
  }

  struct element* element = malloc(sizeof(struct element));

  // Eliminate the element from the front of the queue
  *element = q->elements[q->front];
  // This is due the circular schema of our buffer
  q->front = (q->front + 1) % q->capacity;
  q->size--;
  
  // Each time an element is dequeued we signal the not_full
  // condition variable, so that if the buffer was full, and 
  // the enqueing had to wait, it can now go on
  pthread_cond_signal(&q->not_empty);

  // When the thread has already dequeued an element (using
  // shared resource), we release the lock
  pthread_mutex_unlock(&q->mutex);

  return element;
}

//To check queue state
int queue_empty(queue *q){
  if(q->size==0){
    // The queue is empty
    return 1;
  }
  else{
    // The queue is not empty
    return 0;
  }
}

// To check queue state
int queue_full(queue *q){
  if(q->size==q->capacity){
    // The queue is full
    return 1;
  }
  else{
    // The queue is not full
    return 0;
  }
}

//To destroy the queue and free the resources
int queue_destroy(queue *q){
  if(q==NULL){
    // Queue does not exist
    return -1;
  }

  // We free the memory allocated for the array of stored elements
  //free(q->elements);

  // Destroy mutex and condition variables
  pthread_mutex_destroy(&q->mutex);
  pthread_cond_destroy(&q->not_empty);
  pthread_cond_destroy(&q->not_full);

  // Free the memory allocated for the queue
  free(q);

  return 0;
}
