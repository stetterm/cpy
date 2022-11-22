/**
 * Main source file for the buffered
 * file copy program cpy. Takes command-line
 * input and starts the file transfer.
 *
 * @author Matt Stetter
 * @file cpy.c
 */

#include "buffer.h"
#include "consumer.h"
#include "producer.h"

/**
 * Main entry point for the cpy program.
 * Gets command line input to begin
 * the file transfer.
 * 
 * @param argc must be equal to 3
 * @param argv source file: argv[1]
 * 	  destination file: argv[2]
 * @return 0 if successful, 1 otherwise
 */
int main(int argc, char *argv[]) {
  if (argc != 3) return 1;

  // Initialize the shared buffer
  buffer_t buf;
  buffer_init(&buf);

  // Start the producer thread
  producer_t *prod;
  prod = producer_init(argv[1], &buf);

  // Start the consumer thread
  consumer_t *cons;
  cons = consumer_init(argv[2], &buf);

  // Join on producer and consumer
  producer_join(prod);
  consumer_join(cons);

  // Free the buffer memory and destroy
  // the mutex and semaphores
  buffer_destroy(&buf);

  return 0;
}
