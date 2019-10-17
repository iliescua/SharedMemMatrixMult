#include "multMatrix.h"
#include <stdio.h>

int main() {
  printf("\n\t\t\tSingle Process Execution\n");
  singProcess("matD", "matE");
  printf("\n==================================================================="
         "===========\n\n\t\t\t Multi-Process Execution\n");
  multProcess("matD", "matE");
  printf("\n==================================================================="
         "===========\n\n\t\t\t Anonymous Shared Memory Execution\n");
  anonProcess("matD", "matE");
  printf("\n==================================================================="
         "===========\n\n\t\t\t Named Shared Memory Execution\n");
  namedProcess("matD", "matE");

  return 0;
}