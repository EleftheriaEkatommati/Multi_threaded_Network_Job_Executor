#ifndef THREAD_FUNCTIONS_H
#define THREAD_FUNCTIONS_H

#include <pthread.h>
#include <queue>
using namespace std;

struct ThreadArgs { // arguments που περνάμε στo controller thread
    int client_socket;
    int process_id;
    int mainsocket;
};

extern pthread_mutex_t queueMutex;
extern pthread_cond_t notEmpty;
extern pthread_cond_t notFull;
extern int bufferSize;

extern pthread_t * workers;
extern int threadPoolSize;



void* workerThreadFunction(void* arg);
void* controllerThreadFunction(void* arg);

#endif /* THREAD_FUNCTIONS_H */
