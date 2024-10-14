#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#define TIME_LIMIT 20

// #define NUM_THREADS 4
// #define NUM_RECURSOS 2

#ifndef __GLIBC_USE_LIB_EXT1
    typedef int errno_t;
#endif

typedef struct {
    int popInicial;
    int txCrescimento;
    int tempoTotal;
    int numThreads;
    int numRecursos;
} InputFlags;

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

static struct option long_options[] = 
{
    {"popInicial", required_argument, NULL, 'p'},
    {"txCrescimento", required_argument, NULL, 'x'},
    {"tempoTotal", required_argument, NULL, 't'},
    {"numThreads", required_argument, NULL, 'n'},
    {"numRecursos", required_argument, NULL, 'r'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

errno_t manualImput(int argc, char **argv, InputFlags *inputFlags);
void* threadFunction(void *args);
void handleTimeout(int signum);

int main(int argc, char **argv) {

    srand(time(NULL));

    errno_t err;
    InputFlags inputFlags;

    err = manualImput(argc, argv, &inputFlags);

    sem_t alimento;
    sem_init(&alimento, 0, inputFlags.numRecursos);

    sem_t espaco;
    sem_init(&espaco, 0, inputFlags.numRecursos);

    ThreadArgs *threadArgs = malloc(sizeof(ThreadArgs) * inputFlags.numThreads);
    pthread_t threads[inputFlags.numThreads];

    for(int i = 0; i < inputFlags.numThreads; i++) {
        int tipo = rand() % 3;
        if(tipo == 1) {
            threadArgs[i].tipoColonia = 2;
        } else {
            threadArgs[i].tipoColonia = 1;
        }
        threadArgs[i].threadNum = i + 1;
        threadArgs[i].popInicial = inputFlags.popInicial;
        threadArgs[i].txCrescimento = inputFlags.txCrescimento;
        threadArgs[i].tempoTotal = inputFlags.tempoTotal;
        threadArgs[i].tempoAtual = 1;
        threadArgs[i].popAtual = 100;
        threadArgs[i].alimento = &alimento;
        threadArgs[i].espaco = &espaco;
        threadArgs[i].tempoDecorrido = 0;

        pthread_create(&threads[i], NULL, threadFunction, &threadArgs[i]);
    }

    for (int i = 0; i < inputFlags.numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&alimento);
    sem_destroy(&espaco);

    return 0;
}

errno_t manualImput(int argc, char **argv, InputFlags *inputFlags) {
    int opt;
    bool obtevePopInicial = false;
    bool obteveTxCrescimento = false;
    bool obteveTempoTotal = false;
    bool obteveNumRecursos = false;

    inputFlags->numThreads = sysconf(_SC_NPROCESSORS_ONLN);

    while ((opt = getopt_long(argc, argv, "p:x:t:n:r:h", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'p':
            inputFlags->popInicial = atof(optarg);
            obtevePopInicial = true;
            break;
        case 'x':
            inputFlags->txCrescimento = atof(optarg);
            obteveTxCrescimento = true;
            break;
        case 't':
            inputFlags->tempoTotal = atoi(optarg);
            obteveTempoTotal = true;
            break;
        case 'n':
            inputFlags->numThreads = atoi(optarg);
            break;
        case 'r':
            inputFlags->numRecursos = atoi(optarg);
            obteveNumRecursos = true;
            break;
        case 'h':
            printf("Acessando: %s [OPÇÕES]\n", argv[0]);
            printf("Opções: \n");
            printf("\t-p, --popInicial\t\tPopulação inicial\n");
            printf("\t-x, --txCrescimento\t\tTaxa de crescimento\n");
            printf("\t-t, --tempoTotal\t\tTempo total");
            printf("\t-n, --numThreads\t\tNúmero de threads\n");
            printf("\t-r, --numRecursos\t\tNúmero de recursos\n");
            printf("\t-h, --help\t\t\tExibir esta mensagem de ajuda\n");
            return EINVAL;
        default:
            break;
        }
    }

    if(!obtevePopInicial) {
        fprintf(stderr, "Informe a população inicial (-p)\n");
        return EINVAL;
    }

    if(!obteveTxCrescimento) {
        fprintf(stderr, "Informe a taxa de crescimento (-x)\n");
        return EINVAL;
    }

    if(!obteveTempoTotal) {
        fprintf(stderr, "Informe o tempo total (-t)\n");
        return EINVAL;
    }

    if(!obteveNumRecursos) {
        fprintf(stderr, "Informe a quantidade de recursos (-r)\n");
        return EINVAL;
    }
    return EXIT_SUCCESS;   
}

void* threadFunction(void *args) {
    time_t tempoInicio = time(NULL);

    ThreadArgs *threadArgs = (ThreadArgs *)args;

    while (threadArgs->tempoAtual <= threadArgs->tempoTotal) {
        sleep(rand() % 5);

        signal(SIGALRM, handleTimeout);
        alarm(TIME_LIMIT);

        if(threadArgs->tipoColonia == 1) {
            sem_wait(threadArgs->alimento);
            printf("Thread %d[tipo %d] pegou alimento!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);    
        } else {
            sem_wait(threadArgs->espaco);
            printf("Thread %d[tipo %d] pegou espaço!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
        }

        if(threadArgs->tipoColonia == 1) {
            sem_wait(threadArgs->espaco);
            printf("Thread %d[tipo %d] pegou espaço!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
        } else {
            sem_wait(threadArgs->alimento);
            printf("Thread %d[tipo %d] pegou alimento!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
        }

        alarm(0);

        sleep(rand() % 5);

        threadArgs->popAtual = threadArgs->popAtual * (1 + (threadArgs->txCrescimento / 10));

        sem_post(threadArgs->alimento);
        printf("Thread %d[tipo %d] liberou alimento!\n", threadArgs->threadNum, threadArgs->tipoColonia);
        sem_post(threadArgs->espaco);
        printf("Thread %d[tipo %d] liberou espaço!\n", threadArgs->threadNum, threadArgs->tipoColonia);
        printf("\n");
        printf("Tempo de crescimento da thread %d: %d\n\n", threadArgs->threadNum, threadArgs->tempoAtual);
        threadArgs->tempoAtual ++; 
    }
    threadArgs->tempoDecorrido = difftime(time(NULL), tempoInicio);

    printf("Thread %d terminou\n", threadArgs->threadNum);
    printf("Tipo da colonia: %d\n", threadArgs->tipoColonia);
    printf("População inicial: %g\n", threadArgs->popInicial);
    printf("Taxa de crescimento: %g\n", threadArgs->txCrescimento / 10);
    printf("Tempo: %d\n", threadArgs->tempoTotal);
    printf("População final: %g\n", threadArgs->popAtual);
    printf("Tempo decorrido: %gs\n", threadArgs->tempoDecorrido);
    printf("\n\n");
}

void handleTimeout(int signum) {
    printf("DEADLOCK DETECTADO!!! Tempo de espera excedido! Encerrando...\n\n");
    exit(1);
}