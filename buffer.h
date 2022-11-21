/**
 * Buffer implementation that is used
 * to pass data between the producer
 * and consumer threads.
 *
 * @author Matt Stetter
 * @file buffer.h
 */

#include <pthread.h>
#include <semaphore.h>

#ifndef BUFFER_H_
#define BUFFER_H_

// The number of bytes that can be stored
// in the buffer. 
#define BUFFER_SIZE 8

// Buffer struct used for passing
// data between the producer and
// consumer threads.
typedef struct buffer {
  pthread_mutex_t mutex;// Mutex used for reading and writing to the buffer
  
  sem_t empty_spaces;   // Semaphore with value equal to the
                        // number of empty spaces in the buffer
  
  sem_t full_spaces;    // Semaphore with value equal to the
                        // number of full spaces in the buffer
  
  char *buf;		// Pointer to heap allocated buffer
  unsigned int read;	// Index to read starting from (by consumer)
  unsigned int write;	// Index to write starting from (by producer)
} buffer_t;

/**
 * Initialize the buffer to contain
 * heap-allocated empty buffer.
 *
 * @param b buffer_t struct
 * @return 0 if successful, errno otherwise
 */
int buffer_init(buffer_t *b);

/**
 * Destroy the buffer and free
 * the memory that the buffer
 * points to.
 *
 * @param b buffer_t struct (must be initialized)
 * @return 0 if successful, errno otherwise
 */
int buffer_destroy(buffer_t *b);

#endif
