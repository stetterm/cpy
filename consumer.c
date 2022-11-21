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

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Control struct used for
// completing the file save
// portion of the buffered
// file copy.
typedef struct consumer {
  char *out_file;	// Name of the file to write to
  pthread_t thread;	// Consumer's thread of execution
  buffer_t *buf;	// Buffer to read from
} consumer_t;

void out_flush(int fd, char *buf, int length) {
  ssize_t nbytes;
  write(fd, buf, length);
}

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
  if ((fd = open(c->out_file, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
    fprintf(stderr, "Consumer could not open/create target file %s for writing\n", c->out_file);
    return NULL;
  }

  // Wait until there is at least one
  // character available in the shared
  // buffer.
  sem_wait(&c->buf->full_spaces);
  pthread_mutex_lock(&c->buf->mutex);

  // Get the first character from the buffer
  char ch = c->buf->buf[c->buf->read];
  c->buf->read = (c->buf->read + 1) % BUFFER_SIZE;

  // Alert the producer that there is
  // a new empty space in the buffer
  pthread_mutex_unlock(&c->buf->mutex);
  sem_post(&c->buf->empty_spaces);

  // Initialize temporary buffer for writing
  // to the file
  char temp_buf[CONS_TEMP_BUFFER];
  int buf_index = 0;
  ssize_t nbytes;

  while (ch != '\0') {

    // Flush the buffer to the output
    // file if it is full
    if (buf_index >= CONS_TEMP_BUFFER) {
      nbytes = write(fd, temp_buf, buf_index);

      // If not all the bytes in the buffer
      // could be written to the file,
      // print an error message and return
      if (nbytes != buf_index) {
        fprintf(stderr, "Could not write all %d bytes to file %s\n", buf_index, c->out_file);
	return NULL;
      }
      buf_index = 0;
    }

    // Write this character to the
    // temporary buffer
    temp_buf[buf_index] = ch;

    // Wait until at least one new character
    // is available to read in the buffer
    sem_wait(&c->buf->full_spaces);
    pthread_mutex_lock(&c->buf->mutex);

    // Read the character and increment
    // the read index of the buffer
    ch = c->buf->buf[c->buf->read];
    c->buf->read = (c->buf->read + 1) % BUFFER_SIZE;

    // Release the lock and alert the
    // producer that there is a new
    // empty space in the buffer
    pthread_mutex_unlock(&c->buf->mutex);
    sem_post(&c->buf->empty_spaces);
  }

  // If the buffer still has characters in it,
  // flush them out to the output file
  if (buf_index != 0) {
    nbytes = write(fd, temp_buf, buf_index);
    if (nbytes != buf_index) {
      fprintf(stderr, "Could not write all %d bytes to file %s\n", buf_index, c->out_file);
      return NULL;
    }
  }

  // Try to close the output file
  if (close(fd) != 0) {
    fprintf(stderr, "Consumer could not close target file %s\n", c->out_file);
    return NULL;
  }

  return NULL;
}

// Function to initialize the consumer
// to begin the process of reading
// from the shared buffer and writing
// the data to an output file.
int consumer_init(consumer_t *c, char *file_name, buffer_t *buf) {
  if (file_name == NULL) return 1;

  // Set the output file name and the
  // shared buffer in the consumer struct
  c->out_file = file_name;
  c->buf = buf;

  // Try to initialize the main thread
  // and return 1 if it fails
  if (pthread_create(&c->thread, NULL, &cons_target, c) != 0) {
    fprintf(stderr, "Consumer failed to initialize thread\n");
    return 1;
  }
  
  return 0;
}

// Function to join on the main
// thread of the consumer.
int consumer_join(consumer_t *c) {

  // Join on the internal thread
  pthread_join(c->thread, NULL);

  return 0;
}
