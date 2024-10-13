#include <getopt.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <semaphore.h>

#define NUM_THREADS 4
#define NUM_RECURSOS 4

typedef struct {
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
    pthread_t **threads = malloc(sizeof(pthread_t*) * NUM_THREADS);

    for(int i = 0; i < NUM_THREADS; i++) {
        int tipo = rand() % 10;
        if(tipo == 1) {
            threadArgs[i].tipoColonia = 2;
        } else {
            threadArgs[i].tipoColonia = 1;
        }
        threadArgs[i].popInicial = 100;
        threadArgs[i].txCrescimento = 0.002;
        threadArgs[i].tempoTotal = 30;
        threadArgs[i].tempoAtual = 1;
        threadArgs[i].popAtual = 100;
        threadArgs[i].alimento = &alimento;
        threadArgs[i].espaco = &espaco;
        threadArgs[i].tempoDecorrido = 0;

        pthread_create(threads[i], NULL, threadFunction, &threadArgs[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(*threads[i], NULL);
    }

    return 0;
}

void* threadFunction(void *args) {
    time_t tempoInicio = time(NULL);

    ThreadArgs *threadArgs = (ThreadArgs *)args;

    while (threadArgs->popAtual <= threadArgs->tempoTotal) {
        sleep(rand() %5);

        sem_wait(threadArgs->espaco);
        sem_wait(threadArgs->alimento);

        threadArgs->popAtual = threadArgs->tempoAtual * (1 + threadArgs->txCrescimento);

        sem_post(threadArgs->alimento);
        sem_post(threadArgs->espaco);
        threadArgs->tempoAtual++;
    }
    threadArgs->tempoDecorrido = difftime(time(NULL), tempoInicio);

    printf("Tipo da colonia: %d\n", threadArgs->tipoColonia);
    printf("População inicial: %d\n", threadArgs->popInicial);
    printf("Taxa de crescimento: %d\n", threadArgs->txCrescimento);
    printf("Tempo: %d\n", threadArgs->tempoTotal);
    printf("População final: %d\n", threadArgs->popAtual);
    printf("Tempo decorrido: %d\n", threadArgs->tempoDecorrido);
}