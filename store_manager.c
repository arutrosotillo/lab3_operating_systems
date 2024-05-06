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

// Define a structure to hold each operation
typedef struct {
  int product_id;
  char operation_type[20];
  int units;
} Operation;

// Define arguments structure for the producer thread
typedef struct {
  Operation *operations;
  int start_index;
  int end_index;
} ProducerArgs;

// Function prototype for producer thread
void *producer(void *args);

int main (int argc, const char * argv[])
{
  int profits = 0;
  int product_stock [5] = {0};
  char buffer[2000]; // Buffer to hold each byte read from the file
  int num_operations;
  const char *file_name = argv[1];
  int num_producers = atoi(argv[2]);
  int num_consumers = atoi(argv[3]);
  int buffer_size = atoi(argv[4]);
  FILE *fd;
  int line = 0;
  int num_purchases = 0;

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
  Operation *operations = (Operation *)malloc(num_operations * sizeof(Operation));
  if (operations == NULL) {
    perror("Memory allocation failed.");
    return -1;
  }

  // Read the rest of the lines
  while (fgets(buffer, sizeof(buffer), fd) != NULL && line < num_operations) {
    int product_id, units;
    char operation_type[20];

    // Parse the line to extract product_id, operation_type, and units
    if (sscanf(buffer, "%d %s %d", &product_id, &operation_type, &units) != 3) {
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
        operations[line].product_id = product_id;
        strcpy(operations[line].operation_type, operation_type);
        operations[line].units = units;
      }
      
    }
    line++;
  }

  // Close the file
  fclose(fd);

  // Calculate number of operations per producer
  int ops_per_producer = num_operations / num_producers;
  int remainder = num_operations % num_producers;
  pthread_t producers[num_producers];
  ProducerArgs producer_args[num_producers];

  // Distribute operations among producers
  int start_index = 1;
  for (int i = 0; i < num_producers; i++) {
    producer_args[i].operations = operations;
    producer_args[i].start_index = start_index;
    producer_args[i].end_index = start_index + ops_per_producer + (i < remainder ? 1 : 0) - 1;
    start_index = producer_args[i].end_index + 1;

    // Create producer thread
    pthread_create(&producers[i], NULL, producer, (void *)&producer_args[i]);
  }

  // Wait for all producer threads to finish
  for (int i = 0; i < num_producers; i++) {
    pthread_join(producers[i], NULL);
  }

  // Free allocated memory
  free(operations);

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

void *producer(void *args) {
    // Código para el hilo productor
}
