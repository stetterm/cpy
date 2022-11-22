/**
 * The main header file containing some
 * useful constants and definitions for
 * customizing the cpy program.
 *
 * @author Matt Stetter
 * @file cpy.h
 */

#ifndef CPY_H_
#define CPY_H_

// Change to non-zero value
// to enable debug messages
// to standard error.
// Warning: This will log every character
// that is transmitted.
#define ENABLE_LOGGER 1

// Logger used to print out debug
// information.
#if ENABLE_LOGGER != 0
#define log(...) (fprintf(stderr, __VA_ARGS__))
#else
#define log(...)
#endif

// IMPORTANT: ENSURE THAT BUFFER_SIZE = NUM_BLOCKS * BLOCK_SIZE

// The number of bytes that can be stored
// in the buffer.
#define BUFFER_SIZE 2048

// Constants used for the size and number
// of individual blocks in the buffer.
#define NUM_BLOCKS 64
#define BLOCK_SIZE 32

#endif
