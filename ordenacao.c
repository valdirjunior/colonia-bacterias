/*
Aplicando a prevenção por Ordenação de Recursos:
Alteração:
    - Removido os diferentes tipos de colônias.

Todas as colônias pegarão recurso na mesma orden, evitando que uma delas bloqueie
um recurso que a outra está tentando pegar.

Para compilar utilize:

gcc -o ordenacao ordenacao.c -lpthread -lrt -lm

Para executar utilize(Você pode substituir pelos valores de sua preferência):

./ordenacao -p 200 -x 3 -t 15 -n 4 -r 1

-p = população inicial                    ex: 200
-x = taxa de crescimento em %             ex: 3%
-t = tempo total simulado em segundos     ex: 15
-n = número de threads                    ex: 4
-r = número de recursos de cada tipo      ex: 1

Para que haja deadlock, o número de recursos precisa ser  menor do que o
número de threads. Mais tempo de simulação também favorece para que ocorram
deadlocks. Caso não informe o número de threads, a quantidade será definida
pelo seu processador.
Em caso de dúvida sobre os comandos, execute no terminal:

./ordenacao -h

ou

./ordenacao --help

*/

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

#ifndef __GLIBC_USE_LIB_EXT1
    typedef int errno_t;
#endif

//Struct para as flags via linha de comando
typedef struct {
    int popInicial;
    int txCrescimento;
    int tempoTotal;
    int numThreads;
    int numRecursos;
} InputFlags;

//Argumentos que cada thread receberá
typedef struct {
    int threadNum;               //Número da thread, para visualizar os eventos melhor
    int tipoColonia;             //Tipo da colônia, para que haja cenário para o deadlock
    double popInicial;           //População inicial da colônia de bactérias
    double txCrescimento;        //Taxa de crescimento em %, ex: 2 = 2%
    double popAtual;             //Contador de população no decorrer do tempo
    int tempoTotal;              //Tempo final da simulação de crescimento, não será o tempo real, e sim o limite do contador
    int tempoAtual;              //Contador de tempo
    sem_t* alimento;             //Semáforo para o alimento
    sem_t* espaco;               //Semáforo para o espaço
    double tempoDecorrido;       //Contador do tempo real de execução, para controle e debug
}ThreadArgs;

//Struct para as opções de entrada manual de argumentos
static struct option long_options[] = 
{
    {"populacao", required_argument, NULL, 'p'},
    {"taxa", required_argument, NULL, 'x'},
    {"tempo", required_argument, NULL, 't'},
    {"threads", required_argument, NULL, 'n'},
    {"recursos", required_argument, NULL, 'r'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

errno_t manualImput(int argc, char **argv, InputFlags *inputFlags);
void* threadFunction(void *args);

int main(int argc, char **argv) {

    srand(time(NULL));

    errno_t err;
    InputFlags inputFlags;

    err = manualImput(argc, argv, &inputFlags);

    if (err != 0) {
        return 1;
    }    

    //Inicializa semáforos de alimentos
    sem_t alimento;
    sem_init(&alimento, 0, inputFlags.numRecursos);

    //Inicializa semáforos de espaços
    sem_t espaco;
    sem_init(&espaco, 0, inputFlags.numRecursos);

    ThreadArgs *threadArgs = malloc(sizeof(ThreadArgs) * inputFlags.numThreads);
    pthread_t threads[inputFlags.numThreads];

    //Inicializador das threads
    for(int i = 0; i < inputFlags.numThreads; i++) {
        threadArgs[i].tipoColonia = 1;
        threadArgs[i].threadNum = i + 1;
        threadArgs[i].popInicial = inputFlags.popInicial;
        threadArgs[i].txCrescimento = inputFlags.txCrescimento;
        threadArgs[i].tempoTotal = inputFlags.tempoTotal;
        threadArgs[i].tempoAtual = 1;
        threadArgs[i].popAtual = inputFlags.popInicial;
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

//Função para receber os argumentos via linha de comando
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
            printf("\t-p, --populacao popInicial\t\tPopulação inicial\n");
            printf("\t-x, --taxa txCrescimento\t\tTaxa de crescimento\n");
            printf("\t-t, --tempo tempoTotal\t\t\tTempo total\n");
            printf("\t-n, --threads numThreads\t\tNúmero de threads\n");
            printf("\t-r, --recursos numRecursos\t\tNúmero de recursos\n");
            printf("\t-h, --help\t\t\t\tExibir esta mensagem de ajuda\n");
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

//Função que cada thread irá executar
void* threadFunction(void *args) {
    time_t tempoInicio = time(NULL);

    ThreadArgs *threadArgs = (ThreadArgs *)args;

    while (threadArgs->tempoAtual <= threadArgs->tempoTotal) {
        //Thread dorme até 2s antes de tentar pegar recurso
        sleep(rand() % 2);
        // sleep(2);

        //Thread pega primero o espaço
        sem_wait(threadArgs->espaco);
        printf("Thread %d[tipo %d] pegou espaço!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
        //Thread pega depois o alimento
        sem_wait(threadArgs->alimento);
        printf("Thread %d[tipo %d] pegou alimento!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
        
        //Thread dorme por até 5s após pegar os dois recursos, simulando crescimento
        sleep(rand() % 5);
        // sleep(2);

        //Somatória de crescimento: P(t) = P(0) * e^(rt), no caso, o meu P(0) é o P(t-1), pois calcula a cada passo de tempo, e o t da exponencial será sempre 1
        threadArgs->popAtual = threadArgs->popAtual * exp((threadArgs->txCrescimento / 100));

        //Thread libera os recursos
        sem_post(threadArgs->alimento);
        printf("Thread %d[tipo %d] liberou alimento!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
        sem_post(threadArgs->espaco);
        printf("Thread %d[tipo %d] liberou espaço!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
        
        printf("Tempo de crescimento da thread %d: %d\n\n", threadArgs->threadNum, threadArgs->tempoAtual);
        threadArgs->tempoAtual ++; 
    }
    //Cálculo do tempo real em segundos que a thread executou
    threadArgs->tempoDecorrido = difftime(time(NULL), tempoInicio);

    printf("Thread %d terminou\n", threadArgs->threadNum);
    printf("População inicial: %g\n", threadArgs->popInicial);
    printf("Taxa de crescimento: %g%\n", threadArgs->txCrescimento);
    printf("Tempo: %d\n", threadArgs->tempoTotal);
    printf("População final: %g\n", threadArgs->popAtual);
    printf("Tempo decorrido: %gs\n", threadArgs->tempoDecorrido);
    printf("\n");
}