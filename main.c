#include <getopt.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <semaphore.h>

#define NUM_THREADS 8
#define NUM_RECURSOS 8

typedef struct {
    int threadNum;
    int tipoColonia;
    double popInicial;
    double txCrescimento;
    double popAtual;
    int tempoTotal;
    int tempoAtual;
    sem_t* alimento;
    sem_t* espaco;
    double tempoDecorrido;
}ThreadArgs;

void* threadFunction(void *args);

int main() {

    srand(time(NULL));

    sem_t alimento;
    sem_init(&alimento, 0, NUM_RECURSOS);

    sem_t espaco;
    sem_init(&espaco, 0, NUM_RECURSOS);

    ThreadArgs *threadArgs = malloc(sizeof(ThreadArgs) * NUM_THREADS);
    pthread_t threads[NUM_THREADS];

    for(int i = 0; i < NUM_THREADS; i++) {
        int tipo = rand() % 2;
        if(tipo == 1) {
            threadArgs[i].tipoColonia = 2;
        } else {
            threadArgs[i].tipoColonia = 1;
        }
        threadArgs[i].threadNum = i + 1;
        threadArgs[i].popInicial = 100;
        threadArgs[i].txCrescimento = 0.2;
        threadArgs[i].tempoTotal = 300;
        threadArgs[i].tempoAtual = 1;
        threadArgs[i].popAtual = 100;
        threadArgs[i].alimento = &alimento;
        threadArgs[i].espaco = &espaco;
        threadArgs[i].tempoDecorrido = 0;

        pthread_create(&threads[i], NULL, threadFunction, &threadArgs[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

void* threadFunction(void *args) {
    time_t tempoInicio = time(NULL);

    ThreadArgs *threadArgs = (ThreadArgs *)args;

    while (threadArgs->popAtual <= threadArgs->tempoTotal) {
        sleep(rand() % 5);

        if(threadArgs->tipoColonia == 1) {
            sem_wait(threadArgs->alimento);
        } else {
            sem_wait(threadArgs->espaco);
        }

        if(threadArgs->tipoColonia == 1) {
            sem_wait(threadArgs->espaco);
        } else {
            sem_wait(threadArgs->alimento);
        }


        threadArgs->popAtual = threadArgs->popAtual * (1 + threadArgs->txCrescimento);

        sem_post(threadArgs->alimento);
        sem_post(threadArgs->espaco);
        threadArgs->tempoAtual++;
    }
    threadArgs->tempoDecorrido = difftime(time(NULL), tempoInicio);

    printf("Thread: %d\n", threadArgs->threadNum);
    printf("Tipo da colonia: %d\n", threadArgs->tipoColonia);
    printf("População inicial: %g\n", threadArgs->popInicial);
    printf("Taxa de crescimento: %g\n", threadArgs->txCrescimento);
    printf("Tempo: %d\n", threadArgs->tempoTotal);
    printf("População final: %g\n", threadArgs->popAtual);
    printf("Tempo decorrido: %gs\n", threadArgs->tempoDecorrido);
    printf("\n\n");
}