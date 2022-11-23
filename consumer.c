/**
 * Source implementation of the
 * consumer struct and its functions
 * to start and join on the consumer
 * to perform buffered file copies.
 *
 * @author Matt Stetter
 * @file consumer.c
 */

#include "buffer.h"
#include "consumer.h"
#include "cpy.h"

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Control struct used for
// completing the file save
// portion of the buffered
// file copy.
typedef struct consumer {
  char *out_file;	// Name of the file to write to
  pthread_t *thread;	// Consumer's thread of execution
  buffer_t *buf;	// Buffer to read from
  unsigned int read;	// Index to read from the shared buffer at
} consumer_t;

/**
 * Main thread target for the
 * consumer thread to execute.
 * Reads from the shared buffer
 * into the output file until
 * an EOF character is read.
 *
 * @param args consumer_t struct pointer
 * @return NULL
 */
void *cons_target(void *args) {
  
  // Cast parameter to consumer_t struct
  consumer_t *c = (consumer_t *)args;

  // Try to open the output file to write to
  int fd;
  if ((fd = open(c->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0600)) == -1) {
    fprintf(stderr, "Consumer could not open/create target file %s for writing\n", c->out_file);
    return NULL;
  }

  log("Consumer successfully opened file %s\n", c->out_file);

  unsigned int cur_block = 0;
  pthread_mutex_t *cur_mutex = &c->buf->buf[0].mutex;

  // Wait until there is at least one
  // character available in the shared
  // buffer.
  sem_wait(&c->buf->full_spaces);
  pthread_mutex_lock(cur_mutex);

  log("Consumer got the mutex lock on block %u\n", cur_block);

  // Get the first character from the buffer
  char ch = c->buf->buf[0].blk[0];

  // Increment the index for reading from the buffer
  c->read = (c->read + 1) % BUFFER_SIZE;
  
  sem_post(&c->buf->empty_spaces);

  if (sem_trywait(&c->buf->full_spaces) == -1) {
    pthread_mutex_unlock(cur_mutex);
    log("Consumer ran out of buffer space to read\n");
    sem_wait(&c->buf->full_spaces);
    pthread_mutex_lock(cur_mutex);
  }

  //log("Consumer received character: %c\n", ch);

  // Initialize temporary buffer for writing
  // to the file
  char temp_buf[CONS_BUFFER_SIZE];
  int buf_index = 0;
  ssize_t nbytes;
  unsigned int end_of_block = c->read - (c->read % BLOCK_SIZE) + BLOCK_SIZE;
  
  // Increment the block number and mutex if
  // the consumer reads to a new block
  if (c->read == end_of_block) {
    pthread_mutex_unlock(cur_mutex);
    cur_block = (cur_block + 1) % NUM_BLOCKS;
    cur_mutex = &c->buf->buf[cur_block].mutex;
    end_of_block = (end_of_block + BLOCK_SIZE) % BUFFER_SIZE;
    log("Consumer reached end of block, new end of block is %u\n", end_of_block);
    pthread_mutex_lock(cur_mutex);
  }

  // Keep reading bytes until a null
  // byte is read, which terminates
  // the file copy
  while (ch != '\0') {

    // Flush the buffer to the output
    // file if it is full
    if (buf_index >= CONS_BUFFER_SIZE) {
      nbytes = write(fd, temp_buf, buf_index);

      // If not all the bytes in the buffer
      // could be written to the file,
      // print an error message and return
      if (nbytes != buf_index) {
        fprintf(stderr, "Could not write all %d bytes to file %s\n", buf_index, c->out_file);
	return NULL;
      }
      buf_index = 0;

      log("Consumer wrote %ld bytes to file %s\n", nbytes, c->out_file);
    }

    
    // Write this character to the
    // temporary buffer
    temp_buf[buf_index++] = ch;

    // Read the character and increment
    // the read index of the buffer
    ch = c->buf->buf[cur_block].blk[c->read % BLOCK_SIZE];

    // Increment the index for reading from the buffer
    c->read = (c->read + 1) % BUFFER_SIZE;
    
    // Increment the semaphore containing the number
    // of empty positions in the buffer
    sem_post(&c->buf->empty_spaces);

    // Determine if there are any spaces left to read,
    // and if not, release the lock and wait until
    // there are spaces and finally reacquire the lock
    if (sem_trywait(&c->buf->full_spaces) == -1) {
      pthread_mutex_unlock(cur_mutex);
      log("Consumer ran out of buffer space to read\n");
      if (ch == '\0') break;
      sem_wait(&c->buf->full_spaces);
      pthread_mutex_lock(cur_mutex);
    }

    // Increment the block and mutex being used
    // to read from
    if (c->read == end_of_block) {
      pthread_mutex_unlock(cur_mutex);
      cur_block = (cur_block + 1) % NUM_BLOCKS;
      cur_mutex = &c->buf->buf[cur_block].mutex;
      end_of_block = (end_of_block + BLOCK_SIZE) % BUFFER_SIZE;
      log("Consumer reached end of block, new end of block is %u\n", end_of_block);
      pthread_mutex_lock(cur_mutex);
    }

    
    //log("Consumer received character: %c\n", ch);
  }

  // Release the mutex lock
  pthread_mutex_unlock(cur_mutex);

  // If the buffer still has characters in it,
  // flush them out to the output file
  if (buf_index != 0) {
    nbytes = write(fd, temp_buf, buf_index);
    if (nbytes != buf_index) {
      fprintf(stderr, "Could not write all %d bytes to file %s\n", buf_index, c->out_file);
      return NULL;
    }

    log("Consumer wrote %ld bytes to file %s\n", nbytes, c->out_file);
  }

  // Try to close the output file
  if (close(fd) != 0) {
    fprintf(stderr, "Consumer could not close target file %s\n", c->out_file);
    return NULL;
  }

  log("Consumer closed file %s\n", c->out_file);

  return NULL;
}

// Function to initialize the consumer
// to begin the process of reading
// from the shared buffer and writing
// the data to an output file.
consumer_t *consumer_init(char *file_name, buffer_t *buf) {
  if (file_name == NULL) return NULL;

  // Allocate enough heap memory for the
  // consumer struct
  consumer_t *ctmp;
  ctmp = (consumer_t *)malloc(sizeof(consumer_t));
  if (ctmp == NULL) {
    perror("Failed to allocate memory for the consumer struct\n");
    return NULL;
  }
  consumer_t *c = ctmp;

  log("Successfully allocated memory for the consumer struct\n");

  // Set the output file name and the
  // shared buffer in the consumer struct
  c->out_file = file_name;
  c->buf = buf;
  c->read = 0;

  // Try to initialize the main thread
  // and return 1 if it fails
  pthread_t *temp;
  temp = (pthread_t *)malloc(sizeof(pthread_t));
  if (temp == NULL) {
    perror("Consumer failed to allocate memory for thread\n");
    return NULL;
  }

  log("Successfully allocated memory for the consumer thread\n");

  c->thread = temp;
  if (pthread_create(c->thread, NULL, &cons_target, c) != 0) {
    fprintf(stderr, "Consumer failed to initialize thread\n");
    return NULL;
  }

  log("Successfully started consumer thread\n");
  
  return c;
}

// Function to join on the main
// thread of the consumer.
int consumer_join(consumer_t *c) {

  log("Main thread joining on consumer thread\n");

  // Join on the internal thread
  pthread_join(*c->thread, NULL);

  // Free the memory used for the thread
  free(c->thread);

  // Free the memory used for the consumer struct
  free(c);

  log("Main thread freed consumer memory\n");

  return 0;
}
