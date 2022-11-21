/**
 * API for the producer side
 * of the buffered file IO.
 * Permits the ability to read
 * files and send them to the
 * consumer for saving them.
 *
 * @author Matt Stetter
 * @file producer.h
 */

#include "buffer.h"

#ifndef PRODUCER_H_
#define PRODUCER_H_

#define TEMP_BUFFER_SIZE 16

// Main struct of this producer
// implementation. Contains a pthread
// and the information needed to
// read from a file and send the
// data to the consumer.
typedef struct producer producer_t;

/**
 * Function to initialize the producer
 * struct before the file copy
 * can begin.
 *
 * @param p producer_t struct to initialize
 * @param file_name the name of the file to read from
 * @param buf the buffer to write to
 * @return 0 if successful, errno otherwise
 */
int producer_init(producer_t *p, char *file_name, buffer_t *buf);

/**
 * Joins on the specified
 * producer's thread and returns
 * an error code depending on
 * the success or failure.
 *
 * @param p producer_t struct (must be initialized)
 * @return 0 if successful, errno otherwise
 */
int producer_join(producer_t *p);

#endif
