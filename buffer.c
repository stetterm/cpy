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

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Initializes the buffer by allocating
// heap memory for the internal buffer,
// initializing the mutex, and initializing
// all the internal variables.
int buffer_init(buffer_t *b) {

  // Initialize the mutex
  if (pthread_mutex_init(&b->mutex, NULL) != 0) {
    perror("Could not initialize the mutex\n");
    return 1;
  }

  // Allocate enough memory for the buffer
  char *temp = (char *)malloc(BUFFER_SIZE * sizeof(char));
  assert(temp);
  b->buf = temp;

  // Initialize the two semaphores in the buffer
  sem_init(&b->empty_spaces, 0, BUFFER_SIZE);
  sem_init(&b->full_spaces, 0, 0);

  // Initialize the index
  // variables to 0
  b->read = 0;
  b->write = 0;

  return 0;
}

// Destroy the buffer by destroying
// the inner mutex and freeing
// the memory pointed to by
// the buffer.
int buffer_destroy(buffer_t *b) {
  if (b == NULL) return 1;

  // Destroy the mutex
  pthread_mutex_destroy(&b->mutex);

  // Free the buffer
  free(b->buf);

  return 0;
}
