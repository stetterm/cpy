/**
 * Source implementation of
 * the producer struct and its
 * functions to start and join on
 * the producer's thread to
 * perform the buffered file copy.
 *
 * @author Matt Stetter
 * @file producer.c
 */

#include "buffer.h"
#include "cpy.h"
#include "producer.h"

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Necessary synchronization primitives
// for the producer to copy the file
// to the buffer.
typedef struct producer {
  char *in_file;	// Name of the file to read from
  pthread_t *thread;	// Producer's thread of execution
  buffer_t *buf;	// The buffer struct to write to
  unsigned int write;	// The index to write to the shared buffer at
} producer_t;

/**
 * Function to write data to
 * the buffer, and alert the consumer that
 * there is new data to be read.
 * This is the producer's critical section.
 *
 * @param p the producer_t struct
 * @param d the data to write
 * @param length the number of bytes to write
 */
void send_data(producer_t *p, char *d, int length) {
  
  // Get the block number and mutex
  // for the block the producer wants
  // to write to
  unsigned int cur_block = p->write / BLOCK_SIZE;
  pthread_mutex_t *cur_mutex = &p->buf->buf[cur_block].mutex;
  unsigned int end_of_block = p->write - (p->write % BLOCK_SIZE) + BLOCK_SIZE;

  // Wait until there are empty spaces in
  // the buffer, and then acquire the mutex lock
  sem_wait(&p->buf->empty_spaces);
  pthread_mutex_lock(cur_mutex);

  log("Producer got the mutex lock on block %u\n", cur_block);
  
  for (int i = 0; i < length; i++) {

    // Write a new byte in the buffer and
    // increment the field variables
    p->buf->buf[cur_block].blk[p->write % BLOCK_SIZE] = d[i];

    // Increment the index for writing to the buffer
    p->write = (p->write + 1) % BUFFER_SIZE;
  
    //log("Producer wrote character %c to buffer\n", d[i]);
    
    // Alert the consumer that there is a new character
    // available to be read in the buffer
    sem_post(&p->buf->full_spaces);

    // If there are no spaces available, give up
    // the mutex lock and wait until there are
    // spaces available. Once at least one space
    // is available, get the mutex lock again.
    if (sem_trywait(&p->buf->empty_spaces) == -1) {
      pthread_mutex_unlock(cur_mutex);
      log("Producer ran out of buffer space\n");
      sem_wait(&p->buf->empty_spaces);
      pthread_mutex_lock(cur_mutex);
    }

    // If the loop has entered a new block,
    // change the mutex and cur_block
    if (p->write == end_of_block) {
      pthread_mutex_unlock(cur_mutex);
      cur_block = (cur_block + 1) % NUM_BLOCKS;
      cur_mutex = &p->buf->buf[cur_block].mutex;
      end_of_block = (end_of_block + BLOCK_SIZE) % BUFFER_SIZE;
      log("Producer reached end of block, new end of block is %u\n", end_of_block);
      pthread_mutex_lock(cur_mutex);
    }

  }

  // Release the lock on the last block
  // being written to
  pthread_mutex_unlock(cur_mutex);

  log("Producer release mutex lock\n");
}

/** 
 * Thread target for the producer
 * to read the input file and 
 * write this to the buffer.
 * The parameter is a casted
 * to a pointer to a producer_t.
 *
 * @param args producer struct
 * @return NULL
 */
void *prod_target(void *args) {
  producer_t *p = (producer_t *)args;

  // Producer attempts to open the input
  // file. An error message is printed
  // if this could not be completed.
  int fd;
  if ((fd = open(p->in_file, O_RDONLY)) == -1) {
    fprintf(stderr, "Producer thread could not open file: %s\n", p->in_file);
    return NULL;
  }

  log("Producer successfully opened file %s\n", p->in_file);

  // Initialize file reading variables and read
  // the first block of data from the input file
  ssize_t status;
  char temp_buf[PROD_BUFFER_SIZE];
  status = read(fd, temp_buf, PROD_BUFFER_SIZE);

  log("Producer read %ld bytes from file %s\n", status, p->in_file);

  // While there are still bytes to be read from
  // the input file, push each character to
  // the shared buffer so the consumer can
  // save them to the output file
  while (status) {
   
    // Write each character from the file
    // buffer to the shared buffer
    send_data(p, temp_buf, status);

    // Read the next block of data from the file
    status = read(fd, temp_buf, PROD_BUFFER_SIZE);

    log("Producer read %ld bytes from file %s\n", status, p->in_file);
  }

  // Send the null byte to terminate
  // the buffered file copy
  temp_buf[0] = '\0';
  send_data(p, temp_buf, 1);

  // Try to close the input file
  if (close(fd) != 0) {
    fprintf(stderr, "Producer thread could not close file: %s\n", p->in_file);
  }

  log("Producer successfully closed file %s\n", p->in_file);

  return NULL;
}

// Function to initialize the producer
// struct before the file copy
// can begin.
producer_t *producer_init(char *file_name, buffer_t *buf) {
  if (file_name == NULL) return NULL;

  // Allocate enough heap memory for the
  // producer struct itself
  producer_t *ptmp;
  ptmp = (producer_t *)malloc(sizeof(producer_t));
  if (ptmp == NULL) {
    perror("Could not allocate memory for producer struct\n");
    return NULL;
  }
  producer_t *p = ptmp;

  log("Successfully allocated memory for the producer struct\n");

  // Store the file name and
  // the buffer struct in the
  // producer struct
  p->in_file = file_name;
  p->buf = buf;
  p->write = 0;

  // Spawn the producer thread
  pthread_t *temp;
  temp = (pthread_t *)malloc(sizeof(pthread_t));
  if (temp == NULL) {
    perror("Could not allocate memory for the thread\n");
    return NULL;
  }
  p->thread = temp;

  log("Successfully allocated memory for the producer thread\n");

  if (pthread_create(p->thread, NULL, &prod_target, p) != 0) {
    perror("Could not initialize producer thread\n");
    return NULL;
  }

  log("Successfully started producer thread\n");

  return p;
}

// Join on the producer's thread
// of execution.
int producer_join(producer_t *p) {

  log("Main thread joining on producer thread\n");

  // Join on the internal thread
  pthread_join(*p->thread, NULL);

  // When the producer is finished,
  // free the thread's memory
  free(p->thread);

  // Free the producer struct
  free(p);

  log("Main thread freed producer memory\n");

  return 0;
}

