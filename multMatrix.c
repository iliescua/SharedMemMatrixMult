#include "multMatrix.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/**
 * This is a helper method created which is used to read in the files and
 * populate the the matricies
 * This method exists only in the .c file because it is a helper method
 * that is called on by other methods to read the files and populate
 * the matricies and only exists in the .c file because a user doesn't
 * need to use it and therefore shouldn't have access to it outside of here
 *
 * Parameters (fOne, fTwo, mOne, mTwo): The parameters here are the
 * files that store the matricies which are passed in from the user in
 * main. The matricies are also passed in so that when the data is read
 * in and stored, it can be used by the calling method
 */
void readFiles(char *fOne, char *fTwo, matrix *mOne, matrix *mTwo);

void readFiles(char *fOne, char *fTwo, matrix *mOne, matrix *mTwo) {
  FILE *fileOne = fopen(fOne, "r"); // Loads file one into file variable

  // Checks to make sure a valid file is loaded
  if (fileOne == NULL) {
    printf("Can't Load File\n");
    exit(0);
  }

  // The first two values in the matrix file are row,col and get loaded here
  fscanf(fileOne, "%d", &mOne->row);
  fscanf(fileOne, "%d", &mOne->col);
  // Allocates memory for the size of the matrix and creates pointer to it
  mOne->data = malloc(sizeof(int) * mOne->row * mOne->col);

  // These for loops are used to iterate through the file and store the values
  for (int i = 0; i < mOne->row; i++) {
    for (int j = 0; j < mOne->col; j++) {
      fscanf(fileOne, "%d", &mOne->data[i * mOne->col + j]);
    }
  }
  fclose(fileOne);

  FILE *fileTwo = fopen(fTwo, "r");
  if (fileTwo == NULL) {
    printf("Can't Load File\n");
    exit(0);
  }
  fscanf(fileTwo, "%d", &mTwo->row);
  fscanf(fileTwo, "%d", &mTwo->col);

  // This checks to make sure that the matricies can be muiltiplied together
  if (mOne->col != mTwo->row) {
    printf("Matricies can't be multiplied do to size\n");
    exit(0);
  }

  mTwo->data = malloc(sizeof(int) * mTwo->row * mTwo->col);
  for (int i = 0; i < mTwo->row; i++) {
    for (int j = 0; j < mTwo->col; j++) {
      fscanf(fileTwo, "%d", &mTwo->data[i * mTwo->col + j]);
    }
  }
  fclose(fileTwo);
}

/**
 * This is a helper method created which is used to display the results
 * This method exists only in the .c file because it is a helper method
 * that is called on by other methods to display the product matrix in a
 * clean and understandble format and only exists in the .c file because a user
 * doesn't need to use it and therefore shouldn't have access to it outside of
 * here
 *
 * Parameters (mOne, mTwo, mProduct): The parameters here are the
 * matricies so that the product matrix can be iterated through
 * and printed to the console
 */
void displayResults(matrix *mOne, matrix *mTwo, matrix *mProduct);

void displayResults(matrix *mOne, matrix *mTwo, matrix *mProduct) {
  // This is used to display the reulting matrix in a nice and readable format
  for (int i = 0; i < mOne->row; i++) {
    for (int j = 0; j < mTwo->col; j++) {
      printf("%d\t", mProduct->data[i * mTwo->col + j]);
    }
    printf("\n");
  }
}

/**
 * This is a helper method created which is used to free the mallocs
 * This method exists only in the .c file because it is a helper method
 * that is called on by other methods to perform the matrix to clear the
 * mallocs and it only exists in the .c file because a user doesn't need
 * to use it and therefore shouldn't have access to it outside of here
 *
 * Parameters (mOne, mTwo, mProduct): The parameters here are the
 * matricies so that they can be freed
 */
void freeMalloc(matrix *mOne, matrix *mTwo, matrix *mProduct);

void freeMalloc(matrix *mOne, matrix *mTwo, matrix *mProduct) {
  // Got to free the malloc
  free(mOne->data);
  free(mTwo->data);
  free(mProduct->data);
  free(mOne);
  free(mTwo);
  free(mProduct);
}

/**
 * This is a helper method created which is used to perform the multiplication
 * function as a multi-processor operation
 * This method exists only in the .c file because it is a helper method
 * that is called on by the multProcess method to perform the matrix
 * multiplication and it only exists in the .c file because a user doesn't need
 * to use it and therefore shouldn't have access to it outside of here
 *
 * Parameters (toggle, mOne, mTwo, loc, row, col): The parameters here are the
 * toggle function which controls if it is a read or write instruction, the two
 * matricies that were loaded, loc is a count to keep track of the cell that is
 * supposed to be calculated, and row and col are the current row and col so
 * that the calculation can be exectuted
 */
void child(int *toggle, matrix *mOne, matrix *mTwo, int loc, int row, int col);

void child(int *toggle, matrix *mOne, matrix *mTwo, int loc, int row, int col) {
  childExec result;

  result.val = 0;
  result.loc = loc;

  for (int i = 0; i < mTwo->row; i++) {
    result.val +=
        mOne->data[row * mOne->col + i] * mTwo->data[mTwo->col * i + col];
  }

  write(toggle[1], &result, sizeof(result));

  close(toggle[1]); // Close Write
}

/**
 * This is a helper method created which is used to perform the multiplication
 * function as a multi-processor operation
 * This method exists only in the .c file because it is a helper method
 * that is called on by other method to perform the matrix multiplication
 * and it only exists in the .c file because a user doesn't need to use it and
 * therefore shouldn't have access to it outside of here
 *
 * Parameters (mOne, mTwo, mProduct, loc, row, col): The parameters here are the
 * matricies that contain the passed in values as well as the product matrix,
 * loc is a count to keep track of the cell that is supposed to be calculated,
 * and row and col are the current row and col so that the calculation can be
 * exectuted
 */
void sharedMemChild(matrix *mOne, matrix *mTwo, matrix *mProduct, int loc,
                    int row, int col);

void sharedMemChild(matrix *mOne, matrix *mTwo, matrix *mProduct, int loc,
                    int row, int col) {
  childExec result;
  result.val = 0;
  result.loc = loc;

  for (int i = 0; i < mTwo->row; i++) {
    result.val +=
        mOne->data[row * mOne->col + i] * mTwo->data[mTwo->col * i + col];
  }
  mProduct->data[result.loc] = result.val;
}

// Single process implementation
void singProcess(char *fOne, char *fTwo) {
  int temp = 0;
  int count = 0;
  matrix *mOne = malloc(sizeof(matrix)); // Space is set aside for matrix one
  matrix *mTwo = malloc(sizeof(matrix)); // Space is set aside for matrix two

  // Called to read in the files
  readFiles(fOne, fTwo, mOne, mTwo);

  // Using time.h library to calculate execution time for multiplication func
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  matrix *mProduct = malloc(sizeof(matrix)); // Space is set aside for matrix
  // Pointer is created that will store the product matrix
  mProduct->data = malloc(sizeof(int) * mOne->row * mTwo->col);

  // These loops are used to multiply the matrix together and store the result
  for (int i = 0; i < mOne->row; i++) {
    for (int j = 0; j < mTwo->col; j++) {
      for (int k = 0; k < mTwo->row; k++) {
        temp += mOne->data[i * mOne->col + k] * mTwo->data[k * mTwo->col + j];
      }
      mProduct->data[count] = temp;
      count++;
      temp = 0;
    }
  }

  // This ends the clock time and computes the durationof the multiply func
  clock_gettime(CLOCK_REALTIME, &end);
  double totalTime = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1000000000.0;

  // Called to display product matrix
  displayResults(mOne, mTwo, mProduct);

  // This prints the execution time duration
  printf("\t\t\tExecution time: %fs\n\n", totalTime);

  // Got to free the malloc
  freeMalloc(mOne, mTwo, mProduct);
}

// Malloc multi-proceess implementation
void multProcess(char *fOne, char *fTwo) {
  int count = 0;
  int complete = 0;
  childExec result;
  matrix *mOne = malloc(sizeof(matrix)); // Space is set aside for matrix one
  matrix *mTwo = malloc(sizeof(matrix)); // Space is set aside for matrix two

  // Calls the read method to populate the two passed on matricies
  readFiles(fOne, fTwo, mOne, mTwo);

  // Using the time.h library to calculate execution time
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  matrix *mProduct = malloc(sizeof(matrix)); // Space is set aside for matrix
  // Pointer is created that will store the product matrix
  mProduct->data = malloc(sizeof(int) * mOne->row * mTwo->col);

  // This is used to distinguish between read and write
  int toggleRW[2];

  // Pipe failed
  if (pipe(toggleRW) == -1) {
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < mOne->row; i++) {
    for (int j = 0; j < mTwo->col; j++) {
      pid_t pid = fork(); // Fork into 2 processes

      if (pid < 0) { // Error
        printf("Fork failed, terminating\n");
        exit(EXIT_FAILURE);
      } else if (pid == 0) { // Child PID is 0
        close(toggleRW[0]);  // Close Read
        child(toggleRW, mOne, mTwo, count, i, j);

        // Freeing child malloc
        freeMalloc(mOne, mTwo, mProduct);
        exit(EXIT_SUCCESS);
      }
      count++;
    }
  }

  close(toggleRW[1]); // Close Write

  // Used to load in the values from the child process into the product matrix
  while ((complete = read(toggleRW[0], &result, sizeof(childExec)))) {
    mProduct->data[result.loc] = result.val;
  }

  // Busy wait, required for proper execution
  for (int i = 0; i < (mOne->row * mTwo->col); i++) {
    wait(0);
  }

  // This ends the clock time and computes the duration
  clock_gettime(CLOCK_REALTIME, &end);
  double totalTime = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1000000000.0;

  // Called to display product matrix
  displayResults(mOne, mTwo, mProduct);

  // This prints the execution time duration
  printf("\t\t\tExecution time: %fs\n", totalTime);

  // Freeing parent malloc
  freeMalloc(mOne, mTwo, mProduct);
}

// Anonymous shared memory process implementation
void anonProcess(char *fOne, char *fTwo) {
  int count = 0;
  FILE *fileOne = fopen(fOne, "r"); // Loads file one into file variable
  FILE *fileTwo = fopen(fTwo, "r"); // Loads file one into file variable
  matrix *mOne = mmap(NULL, sizeof(matrix), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1,
                      0); // Space is set aside for matrix one
  matrix *mTwo = mmap(NULL, sizeof(matrix), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1,
                      0); // Space is set aside for matrix two

  // Checks to make sure a valid file is loaded
  if (fileOne == NULL) {
    printf("Can't Load File\n");
    exit(0);
  }

  // The first two values in the matrix file are row,col and get loaded here
  fscanf(fileOne, "%d", &mOne->row);
  fscanf(fileOne, "%d", &mOne->col);

  // Pointer is created that will store matrix one
  mOne->data = mmap(NULL, (sizeof(int) * mOne->row * mOne->col),
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,
                    0); // Space is set aside for matrix one data

  // These for loops are used to iterate through the file and store the values
  for (int i = 0; i < mOne->row; i++) {
    for (int j = 0; j < mOne->col; j++) {
      fscanf(fileOne, "%d", &mOne->data[i * mOne->col + j]);
    }
  }
  fclose(fileOne);

  // The first two values in the matrix file are row,col and get loaded here
  fscanf(fileTwo, "%d", &mTwo->row);
  fscanf(fileTwo, "%d", &mTwo->col);

  // Pointer is created that will store matrix two
  mTwo->data = mmap(NULL, (sizeof(int) * mTwo->row * mTwo->col),
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,
                    0); // Space is set aside for matrix two data

  // These for loops are used to iterate through the file and store the values
  for (int i = 0; i < mTwo->row; i++) {
    for (int j = 0; j < mTwo->col; j++) {
      fscanf(fileTwo, "%d", &mTwo->data[i * mTwo->col + j]);
    }
  }
  fclose(fileTwo);

  // Using the time.h library to calculate execution time
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  matrix *mProduct = mmap(NULL, sizeof(matrix), PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1,
                          0); // Space is set aside for product matrix
  mProduct->data = mmap(NULL, (sizeof(int) * mOne->row * mTwo->col),
                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,
                        0); // Space is set aside for product matrix data

  for (int i = 0; i < mOne->row; i++) {
    for (int j = 0; j < mTwo->col; j++) {
      pid_t pid = fork(); // Fork into 2 processes

      if (pid < 0) { // Error
        printf("Fork failed, terminating\n");
        exit(EXIT_FAILURE);
      } else if (pid == 0) { // Child PID is 0
        sharedMemChild(mOne, mTwo, mProduct, count, i, j);
        exit(EXIT_SUCCESS);
      }
      count++;
    }
  }

  // Busy wait, required for proper execution
  for (int i = 0; i < (mOne->row * mTwo->col); i++) {
    wait(0);
  }

  // This ends the clock time and computes the duration
  clock_gettime(CLOCK_REALTIME, &end);
  double totalTime = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1000000000.0;

  // Called to display product matrix
  displayResults(mOne, mTwo, mProduct);

  // This prints the execution time duration
  printf("\t\t\tExecution time: %fs\n", totalTime);

  // Releasing the used memory space
  munmap((void *)mProduct->data, mOne->row * mTwo->col);
  munmap((void *)mProduct, sizeof(matrix));
  munmap((void *)mOne->data, mOne->row * mOne->col);
  munmap((void *)mOne, sizeof(matrix));
  munmap((void *)mTwo->data, mTwo->row * mTwo->col);
  munmap((void *)mTwo, sizeof(matrix));
}

// Named shared memory process implementation
void namedProcess(char *fOne, char *fTwo) {
  int count = 0;
  // Create a named space in memoy for each matrix
  int fdOne = shm_open("locOne", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  int fdTwo = shm_open("locTwo", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  int fdProduct = shm_open("locProduct", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  // This sets the size of the memory location to the desired value
  ftruncate(fdOne, sizeof(matrix));
  ftruncate(fdTwo, sizeof(matrix));
  ftruncate(fdProduct, sizeof(matrix));
  FILE *fileOne = fopen(fOne, "r"); // Loads file one into file variable
  FILE *fileTwo = fopen(fTwo, "r"); // Loads file one into file variable
  matrix *mOne =
      mmap(NULL, sizeof(matrix), PROT_READ | PROT_WRITE, MAP_SHARED, fdOne,
           0); // Space is set aside for matrix one
  matrix *mTwo =
      mmap(NULL, sizeof(matrix), PROT_READ | PROT_WRITE, MAP_SHARED, fdTwo,
           0); // Space is set aside for matrix two

  // Checks to make sure a valid file is loaded
  if (fileOne == NULL) {
    printf("Can't Load File\n");
    exit(0);
  }

  // The first two values in the matrix file are row,col and get loaded here
  fscanf(fileOne, "%d", &mOne->row);
  fscanf(fileOne, "%d", &mOne->col);

  // Pointer is created that will store matrix one
  mOne->data = mmap(NULL, (sizeof(int) * mOne->row * mOne->col),
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,
                    0); // Space is set aside for matrix one data

  // These for loops are used to iterate through the file and store the values
  for (int i = 0; i < mOne->row; i++) {
    for (int j = 0; j < mOne->col; j++) {
      fscanf(fileOne, "%d", &mOne->data[i * mOne->col + j]);
    }
  }
  fclose(fileOne);

  // The first two values in the matrix file are row,col and get loaded here
  fscanf(fileTwo, "%d", &mTwo->row);
  fscanf(fileTwo, "%d", &mTwo->col);

  // Pointer is created that will store matrix two
  mTwo->data = mmap(NULL, (sizeof(int) * mTwo->row * mTwo->col),
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,
                    0); // Space is set aside for matrix two data

  // These for loops are used to iterate through the file and store the values
  for (int i = 0; i < mTwo->row; i++) {
    for (int j = 0; j < mTwo->col; j++) {
      fscanf(fileTwo, "%d", &mTwo->data[i * mTwo->col + j]);
    }
  }
  fclose(fileTwo);

  // Using the time.h library to calculate execution time
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  matrix *mProduct =
      mmap(NULL, sizeof(matrix), PROT_READ | PROT_WRITE, MAP_SHARED, fdProduct,
           0); // Space is set aside for product matrix
  mProduct->data = mmap(NULL, (sizeof(int) * mOne->row * mTwo->col),
                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,
                        0); // Space is set aside for product matrix data

  for (int i = 0; i < mOne->row; i++) {
    for (int j = 0; j < mTwo->col; j++) {
      pid_t pid = fork(); // Fork into 2 processes

      if (pid < 0) { // Error
        printf("Fork failed, terminating\n");
        exit(EXIT_FAILURE);
      } else if (pid == 0) { // Child PID is 0
        sharedMemChild(mOne, mTwo, mProduct, count, i, j);
        exit(EXIT_SUCCESS);
      }
      count++;
    }
  }

  // Busy wait, required for proper execution
  for (int i = 0; i < (mOne->row * mTwo->col); i++) {
    wait(0);
  }

  // This ends the clock time and computes the duration and prints it
  clock_gettime(CLOCK_REALTIME, &end);
  double totalTime = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1000000000.0;

  // Called to display product matrix
  displayResults(mOne, mTwo, mProduct);

  // This prints the execution time duration
  printf("\t\t\tExecution time: %fs\n", totalTime);

  // Releasing the used memory space
  munmap((void *)mProduct->data, mOne->row * mTwo->col);
  munmap((void *)mProduct, sizeof(matrix));
  munmap((void *)mOne->data, mOne->row * mOne->col);
  munmap((void *)mOne, sizeof(matrix));
  munmap((void *)mTwo->data, mTwo->row * mTwo->col);
  munmap((void *)mTwo, sizeof(matrix));
  shm_unlink("memLoc");
}