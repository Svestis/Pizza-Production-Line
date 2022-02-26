#include "pizza2.h"

//-----Variables----
unsigned char cooks = (char)Ncook;
unsigned char oven = (char)Noven;
unsigned char deliver = (char)Ndeliver;
float* maxTime;
float* totalCoolTime;
float* maxCoolTime;
float* totalTime;
int RandSeed;

//pthread related
pthread_mutex_t cooks_lock;
pthread_mutex_t ovens_lock;
pthread_mutex_t deliver_lock;
pthread_mutex_t clock_lock;
pthread_cond_t cond_cook;
pthread_cond_t cond_oven;
pthread_cond_t cond_deliver;

//Main
int main (int argc, char **argv){
    
    //Var declaration/init
    int Ncust;
    int rc;
    totalTime = (float*) malloc(sizeof(float));
    maxTime = (float*) malloc(sizeof(float));
    totalCoolTime = (float*) malloc(sizeof(float));
    maxCoolTime = (float*) malloc(sizeof(float));
    *totalTime = 0;
    *maxTime = 0;
    *totalCoolTime = 0;
    *maxCoolTime = 0;
    
    //Parameter number check and error handling
    if (argc != 3){
        fprintf (stderr, "Wrong number of arguments. Arguments should be exactly 2.\nTry again!");
        exit (EXIT_FAILURE);
    }
    
    //Initializing variables
    RandSeed = custAtoi(argv[2], 2);
    Ncust = custAtoi(argv[1], 1);
    
    //Checking that customers is greater than 0
    if (Ncust <= 0){
      fprintf (stderr, "Customers have to be greater than 0.\nTry again!");
      exit (EXIT_FAILURE);
    }
    
    printf("Starting execution with %d customers and %d random seed.\n\n", Ncust, RandSeed);
    
    //Needed variables for threads - init
    pthread_t threads[Ncust];
    int id[Ncust];
    pthread_mutex_init(&cooks_lock, NULL);
    pthread_mutex_init(&clock_lock, NULL);
    pthread_mutex_init(&ovens_lock, NULL);
    pthread_mutex_init(&deliver_lock, NULL);
    pthread_cond_init(&cond_oven, NULL);
    pthread_cond_init(&cond_cook, NULL);
    pthread_cond_init(&cond_deliver, NULL);
    
    //Creating threads
    for(int i = 0; i < Ncust; i++){
        id[i] = i+1;
        rc = pthread_create(&threads[i], NULL, custOrder, &id[i]);
        sleep((rand_r(&RandSeed)%(Torderhigh-Torderlow+1))+Torderlow);//Waiting random time for next order
    }
    
    //Joining threads for finishing
    for(int i = 0; i < Ncust; i++){
        pthread_join(threads[i], NULL);
    }

    //Printing totals / avg   
    printf("\nThe time needed for the longest delivery is %d min.\\ %.2f hr.\n", (int)*maxTime, *maxTime/60);
    printf("The average delivery time per customer is %.2f min.\\ %d sec.\n", *totalTime/Ncust, (int)(*totalTime/Ncust)*60);
    printf("\nThe time needed for the longest cooling time is %d min.\\ %.2f hr.\n", (int)*maxCoolTime, *maxCoolTime/60);
    printf("The average cooling time per customer is %.2f min.\\ %d sec.\n", *totalCoolTime/Ncust, (int)(*totalCoolTime/Ncust)*60);

    pthread_mutex_destroy(&cooks_lock);
    pthread_mutex_destroy(&clock_lock);
    pthread_mutex_destroy(&ovens_lock);
    pthread_mutex_destroy(&deliver_lock);
    pthread_cond_destroy(&cond_oven);
    pthread_cond_destroy(&cond_cook);
    pthread_cond_destroy(&cond_deliver);
    free(totalTime);
    free(maxTime);
    free(totalCoolTime);
    free(maxCoolTime);
}

void *custOrder(void *x){
    int id = *(int *)x; //id
    struct timespec start, finish, finishCooking; //time
    clock_gettime(CLOCK_REALTIME, &start); //start time for customer
    float custTime;
    float coolTime;
    
    //Cooks -- locking and unlocking to check availability and decrease if available
    pthread_mutex_lock(&cooks_lock);
    while(cooks==0){
        pthread_cond_wait(&cond_cook, &cooks_lock);
    }
    cooks = cooks-1;
    pthread_mutex_unlock(&cooks_lock);
    int pizzas = rand_r((&RandSeed))%(Norderhigh-Norderlow+1)+Norderlow;
    
    sleep(Tprep*pizzas); //Preparation time
    
    //oven -- locking and unlocking to check availability and decrease if available
    pthread_mutex_lock(&ovens_lock);
    while(oven==0){
        pthread_cond_wait(&cond_oven, &ovens_lock);
    }
    oven = oven-1;
    pthread_mutex_unlock(&ovens_lock);
    
    //Cooks -- locking and unlocking to increase available cooks
    pthread_mutex_lock(&cooks_lock);
    cooks = cooks+1;
    pthread_mutex_unlock(&cooks_lock);
    
    sleep(Tbake);//bake time
    clock_gettime(CLOCK_REALTIME, &finishCooking); //finish cooking time
    
    //deliverer -- locking and unlocking to check availability and decrease if available
    pthread_mutex_lock(&deliver_lock);
    while(deliver==0){
        pthread_cond_wait(&cond_deliver, &deliver_lock);
    }
    deliver = deliver-1;
    pthread_mutex_unlock(&deliver_lock);
    
    //oven -- locking and unlocking to increase available ovens
    pthread_mutex_lock(&ovens_lock);
    oven = oven+1;
    pthread_mutex_unlock(&ovens_lock);
    
    int deliveryTime = (rand_r(&RandSeed))%(Thigh-Tlow+1)+Tlow;
    sleep(deliveryTime); //Waiting random time for delivery

    //Unblocking any blocked threads
    pthread_cond_signal(&cond_cook);
    pthread_cond_signal(&cond_oven);
    
    clock_gettime(CLOCK_REALTIME, &finish); //Getting end of this customer time
    pthread_mutex_lock(&clock_lock); //Locking for calculations
    custTime = finish.tv_sec - start.tv_sec; //Customer time
    coolTime = finish.tv_sec - finishCooking.tv_sec; //Cool time
     if (custTime > *maxTime) {
        *maxTime = custTime;
    }
    if (coolTime > *maxCoolTime) {
        *maxCoolTime = coolTime;
    }
    *totalTime = *totalTime + custTime; //Total time
    *totalCoolTime = *totalCoolTime + coolTime;
    printf("The order with id %d was delivered in %d min.\\ %d sec. and was cooling %d min.\\ %d sec.\n", id, (int)custTime, (int)custTime*60, (int)coolTime, (int)coolTime*60); //printing required exit
    pthread_mutex_unlock(&clock_lock); //Unclocking
    
    sleep(deliveryTime); //Return of deliverer
    
    //deliverer -- locking and unlocking to increase available deliverers
    pthread_mutex_lock(&deliver_lock);
    deliver = deliver+1;
    pthread_mutex_unlock(&deliver_lock);

    //Unblocking any blocked delieverer threads
    pthread_cond_signal(&cond_deliver);
    
    //Exiting thread
    pthread_exit(NULL);
}
    
int custAtoi (char input[], int argument_num){
    
    //Declaring vars
    boolean holder = TRUE; //Holding the boolean value
    int i = 0; //Enumerator counter

    //checking for negative numbers
    if (input[0] == '-'){
      i = 1;
    }

    //iterating through input to check if everything is number
    for (; input[i] != 0; i++){
        //if non numeric character is found seting the boolean value to false
        if (!isdigit(input[i])){
            fprintf(stderr, "Non numeric character found on argument number %d.\nTry again!", argument_num);
            exit(EXIT_FAILURE);
        }
    }

    //Checking if there are too many digits to be stored in int
    if (i > 10){
      fprintf (stderr, "Argument number %d too long.\nTry again", argument_num);
      exit(EXIT_FAILURE);
    }
    
    //Returning value
    return atoi(input);
}