//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

// Define arguments structure for the producer thread
typedef struct {
  element *elements;
  int start_index;
  int end_index;
  queue *q;  // Including queue pointer in the ProducerArgs
} ProducerArgs;

// Function prototype for producer thread
void *producer(void *args);
// Function prototype for consumer thread
void *consumer(void *args);

int main (int argc, const char * argv[])
{ 
  int profits = 0;
  int product_stock [5] = {0};
  char buffer[2000] = ""; // Buffer to hold each byte read from the file
  int num_operations = 0;
  int num_producers = atoi(argv[2]);
  int num_consumers = atoi(argv[3]);
  int buffer_size = atoi(argv[4]);
  FILE *fd;
  int line = 0;

  //Check if there are less arguments or more than should be
  if (argc != 5){
    return -1;
  }
  
  // Open the file
  fd = fopen(argv[1], "r");
  if (fd == NULL) {
    perror("Error opening file.");
    return -1;
  }
  
  // Read the first line
  if (fgets(buffer, 100000, fd) != NULL) {
    num_operations = atoi(buffer);
  } else {
    perror("Empty file.");
  }

  // Allocate memory for operations array
  element *elements = (element *)malloc(num_operations * sizeof(element));
  if (elements == NULL) {
    perror("Memory allocation failed.");
    return -1;
  }

  // Read the rest of the lines
  while (fgets(buffer, sizeof(buffer), fd) != NULL && line < num_operations) {
    int product_id, units;
    char operation_type[20];

    // Parse the line to extract product_id, operation_type, and units
    if (sscanf(buffer, "%d %s %d", &product_id, operation_type, &units) != 3) {
      printf("This line has a incorrect format: %s\n", buffer);
      continue; // Skip this line and move to the next one
    }
    else
    {
      // Check constraints on product_id and operation_type
      int valid_product_id = (product_id >= 1 && product_id <= 5);
      int valid_operation_type = (strcmp(operation_type, "PURCHASE") == 0 || strcmp(operation_type, "SALE") == 0);

      if (!valid_product_id || !valid_operation_type) {
        printf("Invalid product_id or operation_type on line: %d\n", line + 2); // +2 because line index starts from 0 and we've already read one line
        continue; // Skip this line and move to the next one
      }
      else
      {
        // Store the operation in the operations array
        elements[line].product_id = product_id;
        elements[line].op = (strcmp(operation_type, "PURCHASE") == 0) ? 0 : 1; // Set operation type: 0 for PURCHASE, 1 for SALE
        elements[line].units = units;
        
      }
      
    }
    line++;
  }
  
  // Close the file
  fclose(fd);

  // Initialize the queue
  queue *q = queue_init(buffer_size);
  if (q == NULL) {
    perror("Failed to initialize the queue.");
    free(elements);
    return -1;
  }

  // Calculate number of operations per producer
  int ops_per_producer = num_operations / num_producers;
  int remainder1 = num_operations % num_producers;
  pthread_t producers[num_producers];
  ProducerArgs producer_args[num_producers];


  // Calculate number of operations per consumer
  int ops_per_consumer = num_operations / num_consumers;
  int remainder2 = num_operations % num_consumers;
  pthread_t consumers[num_consumers];
  ProducerArgs consumer_args[num_consumers];
  
  // Distribute operations among producers
  int start_index = 0;
  for (int i = 0; i < num_producers; i++) {
    producer_args[i].elements = elements;
    producer_args[i].start_index = start_index;
    producer_args[i].end_index = start_index + ops_per_producer + (i < remainder1 ? 1 : 0) - 1;
    producer_args[i].q = q;  // Set the queue here
    start_index = producer_args[i].end_index + 1;

    // Create producer thread
    pthread_create(&producers[i], NULL, producer, (void *)&producer_args[i]);
  }

  start_index = 0;
  // Create consumer threads
  for (int i = 0; i < num_consumers; i++) {
    consumer_args[i].start_index = start_index;
    consumer_args[i].end_index = start_index + ops_per_consumer + (i < remainder2 ? 1 : 0) - 1;
    consumer_args[i].q = q;  // Set the queue here
    start_index = consumer_args[i].end_index + 1;
    pthread_create(&consumers[i], NULL, consumer, (void *)&consumer_args[i]);
  }

  // Wait for all producer threads to finish
  for (int i = 0; i < num_producers; i++) {
    pthread_join(producers[i], NULL);
  }

  // Join consumer threads
  for (int i = 0; i < num_consumers; i++) {
    //int data [6];
    int *data;
    pthread_join(consumers[i], (void **)&data);
    if(data != NULL){
      profits += data[0];
      for (int j = 1; j < 6; j++) {
        product_stock[j - 1] += data[j];

      }
    }   
    free(data);
  }

  queue_destroy(q);

  // Output
  printf("Total: %d euros\n", profits);
  printf("Stock:\n");
  printf("  Product 1: %d\n", product_stock[0]);
  printf("  Product 2: %d\n", product_stock[1]);
  printf("  Product 3: %d\n", product_stock[2]);
  printf("  Product 4: %d\n", product_stock[3]);
  printf("  Product 5: %d\n", product_stock[4]);

  return 0;
}

// Producer thread function
void *producer(void *args) {
  ProducerArgs *pargs = (ProducerArgs *)args;
  queue *q = pargs->q;  // Extract queue from the passed ProducerArgs
  element *elements = pargs->elements;
  
  // Iterate over the assigned range of operations
  for (int i = pargs->start_index; i <= pargs->end_index; i++) {
    // Create an element for the queue from the operation data
    element *new_element = malloc(sizeof(element));
    new_element->product_id = elements[i].product_id;  // Set product ID
    new_element->op = elements[i].op; // Set operation type
    new_element->units = elements[i].units; // Set units involved in the operation

    // Enqueue the element into the shared queue
    queue_put(q, new_element);  // Assuming queue_put is adapted to handle 'element'
  }
  pthread_exit(NULL);
  return NULL;
}

// Consumer thread function
void *consumer(void *args) {
  ProducerArgs *cargs = (ProducerArgs *)args;
  queue *q = cargs->q;  // Extract queue from the passed ProducerArgs
  
  int *data = malloc(6 * sizeof(int)); // Dynamically allocate memory for data
  if (data == NULL) {
    perror("Memory allocation failed for consumer data.");
    pthread_exit(NULL);
  }
  int cost [5] = {2,5,15,25,100};
  int price [5] = {3,10,20,40,125};

  for (int i = cargs->start_index; i <= cargs->end_index; i++) {
    // Extract element from the queue
    element *elements = queue_get(q);
    // Check if it's the termination signal
    if (elements == NULL) {
      printf("finish\n");
      break;  // Terminate the thread
    }
    
    // Process the operation
    // Calculate profit and update product stock based on the operation
    int product_id = elements->product_id;
    int operation_type = elements->op;
    int units = elements->units;

    // Update product stock and profit
    if (operation_type == 0) {  // PURCHASE
      data[product_id] += units;
      data[0] -= cost[product_id - 1] * units;
    } else {  // SALE
      data[product_id] -= units;
      data[0] += price[product_id - 1] * units;
    }
    
    // Free the memory allocated for the element
    free(elements);
  }
  pthread_exit((void *)data);
}
