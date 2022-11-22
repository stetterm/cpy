/**
 * API for the consumer side
 * of the buffered file IO.
 * Reads from the buffer using
 * a separate execution thread
 * and writes to a destination file.
 *
 * @author Matt Stetter
 * @file consumer.h
 */

#include "buffer.h"

#ifndef CONSUMER_H_
#define CONSUMER_H_

#define CONS_BUFFER_SIZE 32

// Main struct of this consumer
// implementation. Contains a pthread
// and the information needed to save
// the data in the shared buffer to
// an output file.
typedef struct consumer consumer_t;

/**
 * Function to allocate and initialize 
 * the consumer struct before the 
 * output file saving can begin. 
 * This function will begin the 
 * process of asynchronously copying the file.
 *
 * @param file_name the name of the file to write to
 * @param buf the buffer to read from
 * @return 0 if successful, errno otherwise
 */
consumer_t *consumer_init(char *file_name, buffer_t *buf);

/**
 * Joins on the specified
 * consumer's main thread
 * of execution.
 *
 * @param c consumer_t struct to join on (must be initialized)
 * @return 0 if successful, errno otherwise
 */
int consumer_join(consumer_t *c);

#endif
