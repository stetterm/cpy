/**
 * Source implementation of the functions
 * necessary to create the buffer used
 * by the producer and consumer, and also
 * to destroy the buffer and free the memory.
 *
 * @author Matt Stetter
 * @file buffer.c
 */

#include "buffer.h"
#include "cpy.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Initializes the buffer by allocating
// heap memory for the internal buffer,
// initializing the mutex, and initializing
// all the internal variables.
int buffer_init(buffer_t *b) {

  // Allocate enough memory for the shared
  // inner buffer
  b->buf = (block_t *)calloc(NUM_BLOCKS, sizeof(block_t));
  assert(b->buf);

  log("Successfully allocated memory for the internal buffer\n");

  // Initialize all the mutexes in each block
  for (int i = 0; i < NUM_BLOCKS; i++) {
    pthread_mutex_init(&b->buf[i].mutex, NULL);
  }

  log("Successfully initialized mutexes for each block in the buffer\n");

  // Initialize the two semaphores in the buffer
  sem_init(&b->empty_spaces, 0, BUFFER_SIZE);
  sem_init(&b->full_spaces, 0, 0);

  log("Successfully initialized the semaphores for the buffer\n");
  
  return 0;
}

// Destroy the buffer by destroying
// the inner mutex and freeing
// the memory pointed to by
// the buffer.
int buffer_destroy(buffer_t *b) {
  if (b == NULL) return 1;

  // Destroy each mutex in the buffer
  for (int i = 0; i < NUM_BLOCKS; i++) {
    pthread_mutex_destroy(&b->buf[i].mutex);
  }

  log("Main thread destroyed the mutexes in the buffer\n");

  // Free the buffer memory
  free(b->buf);

  log("Main thread freed the memory used for the buffer\n");

  // Destroy the semaphores
  sem_destroy(&b->empty_spaces);
  sem_destroy(&b->full_spaces);

  log("Main thread destroyed the semaphores in the buffer\n");

  return 0;
}
