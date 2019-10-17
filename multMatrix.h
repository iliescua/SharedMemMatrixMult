#ifndef MULTMATRIX_H
#define MULTMATRIX_H

/**
 * This is a struct that was designed to have all of the componenets of
 * a matrix where it has int values for the row and col and an int pointer
 * to locate the data of the matrix
 */
typedef struct {
  int row;
  int col;
  int *data;
} matrix;

/**
 * This is a struct that was designed to be used so that values can be passed
 * between the child and parent process when completing the multi-processor
 * function
 */
typedef struct {
  int val;
  int loc;
} childExec;

/**
 * This is the method which runs the matrix multiplication algorithim
 * as a singular process and only uses a single procssor for execution
 *
 * Parameters (fOne, fTwo): These are the two string
 * pointers which are used to pass in the names of the files that store
 * the matricies. Make sure that the files are in the same dir as the
 * program files otherwise the file path needs to be passed in instead
 * of just the name
 */
void singProcess(char *fOne, char *fTwo);

/**
 * This is the method which runs the matrix multiplication algorithim
 * as multiple processes and uses multiple procssors for execution
 *
 * Parameters (filenameOne, filenameTwo): These are the two string
 * pointers which are used to pass in the names of the files that store
 * the matricies. Make sure that the files are in the same dir as the
 * program files otherwise the file path needs to be passed in instead
 * of just the name
 */
void multProcess(char *fOne, char *fTwo);

/**
 * This is the method which runs the matrix multiplication algorithim
 * using named shared memory
 *
 * Parameters (filenameOne, filenameTwo): These are the two string
 * pointers which are used to pass in the names of the files that store
 * the matricies. Make sure that the files are in the same dir as the
 * program files otherwise the file path needs to be passed in instead
 * of just the name
 */
void namedProcess(char *fOne, char *fTwo);

/**
 * This is the method which runs the matrix multiplication algorithim
 * using anonymous share memory
 *
 * Parameters (filenameOne, filenameTwo): These are the two string
 * pointers which are used to pass in the names of the files that store
 * the matricies. Make sure that the files are in the same dir as the
 * program files otherwise the file path needs to be passed in instead
 * of just the name
 */
void anonProcess(char *fOne, char *fTwo);

#endif