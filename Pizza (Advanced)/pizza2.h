//Including needed libraries
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

//Including constants
#define Ncook 2
#define Noven 5
#define Ndeliver 10
#define Torderlow 1
#define Torderhigh 5
#define Norderlow 1
#define Norderhigh 5
#define Tprep 1
#define Tbake 10
#define Tlow 5
#define Thigh 15

//Custom boolean type
typedef enum{FALSE = 0, TRUE} boolean;

//functions
int custAtoi (char input[], int argument_num);
void *custOrder(void *x);