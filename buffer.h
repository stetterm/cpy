/**
 * Buffer implementation that is used
 * to pass data between the producer
 * and consumer threads.
 *
 * @author Matt Stetter
 * @file buffer.h
 */

#include "cpy.h"

#include <pthread.h>
#include <semaphore.h>

#ifndef BUFFER_H_
#define BUFFER_H_

// Struct used to represent each block
// of the buffer. This splits the
// full buffer into a series of blocks
// of a constant size. The buffer size
// should be a mulitple of the block size.
// The block contains the data stored in
// the block and a mutex used for
// reading and writing the block.
typedef struct block {
  pthread_mutex_t mutex; // Mutex used for reading and writing to a block
  char blk[BLOCK_SIZE];	 // Buffer block containing the buffered characters
} block_t;

// Buffer struct used for passing
// data between the producer and
// consumer threads.
typedef struct buffer {
  sem_t empty_spaces;   // Semaphore with value equal to the
                        // number of empty spaces in the buffer
  
  sem_t full_spaces;    // Semaphore with value equal to the
                        // number of full spaces in the buffer
  
  block_t *buf;		// Pointer to heap allocated buffer
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
