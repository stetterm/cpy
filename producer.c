/**
 * Source implementation of
 * the producer struct and its
 * functions to start, join on,
 * and destroy the struct after
 * the copy has concluded.
 *
 * @author Matt Stetter
 * @file producer.c
 */

#include "buffer.h"
#include "producer.h"

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

// Necessary synchronization primitives
// for the producer to copy the file
// to the buffer.
typedef struct producer {
  char *in_file;	// Name of the file to read from
  pthread_t thread;	// Producer's thread of execution
  buffer_t *buf;	// The buffer struct to write to
} producer_t;

// Thread target for the producer
// to read the input file and 
// write this to the buffer.
// The parameter is a casted
// to a pointer to a producer_t.
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

  // Initialize file reading variables and read
  // the first block of data from the input file
  ssize_t status;
  char temp_buf[TEMP_BUFFER_SIZE];
  status = read(fd, temp_buf, TEMP_BUFFER_SIZE);

  // While there are still bytes to be read from
  // the input file, push each character to
  // the shared buffer so the consumer can
  // save them to the output file
  while (status) {

    // Send each byte read to the consumer
    // using the shared buffer
    for (int i = 0; i < status; i++) {

      // Wait until there are empty spaces in
      // the buffer, and then acquire the mutex lock
      sem_wait(&p->buf->empty_spaces);
      pthread_mutex_lock(&p->buf->mutex);

      // Write a new byte in the buffer and
      // increment the field variables
      p->buf->buf[p->buf->write] = temp_buf[i];
      p->buf->write = (p->buf->write + 1) % BUFFER_SIZE;

      // Release the mutex lock and increment
      // the full spaces semaphore
      pthread_mutex_unlock(&p->buf->mutex);
      sem_post(&p->buf->full_spaces);
    }

    // Read the next block of data from the file
    status = read(fd, temp_buf, TEMP_BUFFER_SIZE);
  }

  if (close(fd) != 0) {
    fprintf(stderr, "Producer thread could not close file: %s\n", p->in_file);
  }

  return NULL;
}

// Function to initialize the producer
// struct before the file copy
// can begin.
int producer_init(producer_t *p, char *file_name, buffer_t *buf) {
  if (file_name == NULL) return 1;

  // Store the file name and
  // the buffer struct in the
  // producer struct
  p->in_file = file_name;
  p->buf = buf;

  // Spawn the producer thread
  if (pthread_create(&p->thread, NULL, &prod_target, p) != 0) {
    perror("Could not initialize producer thread\n");
    return 1;
  }

  return 0;
}

// Join on the producer's thread
// of execution.
int producer_join(producer_t *p) {

  // Join on the internal thread
  pthread_join(p->thread, NULL);

  return 0;
}

