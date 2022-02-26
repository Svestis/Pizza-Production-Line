//Including needed libraries
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

//Including constants
#define Ncook 6
#define Noven 5
#define Torderlow 1
#define Torderhigh 5
#define Norderlow 1
#define Norderhigh 5
#define Tprep 1
#define Tbake 10

//Custom boolean type
typedef enum{FALSE = 0, TRUE} boolean;

//functions
int custAtoi (char input[], int argument_num);
void *custOrder(void *x);