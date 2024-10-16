/*
Aplicando a prevenção por Timeout ou Recurso de Recuperação:
Alteração:
    - Adicionado uso da função sem_trywait, com verificação de timeout.

Todas as colônias terão uma função de timeout, esta função fará com que o primeiro recurso seja
liberado caso não seja possível obter o segundo dentro de um intervalo de tempo definido.
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
#include <signal.h>

#define TIME_LIMIT 20  //Tempo em segundos para soltar o recurso e tentar novamente

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
        //Defino a primeira thread como tipo 2, e as demais como tipo 1
        if(i == 0) {
            threadArgs[i].tipoColonia = 2;
        } else {
            threadArgs[i].tipoColonia = 1;
        }
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

//Função que cada thread irá executar
void* threadFunction(void *args) {
    time_t tempoInicio = time(NULL); //Marcador de tempo do início da execução da thread
    double tempoColetaRecurso;       //Marcador para o tempo aguardando segundo recurso

    ThreadArgs *threadArgs = (ThreadArgs *)args;

    while (threadArgs->tempoAtual <= threadArgs->tempoTotal) {
        //Thread dorme até 5s antes de tentar pegar recurso
        sleep(rand() % 5);

        //Marcador do início do tempo de aguardo por recurso
        time_t tempoInicioColetaRecurso = time(NULL);

        //Se a colônia for do tipo 1, irá pegar o alimento primeiro e depois o espaço
        if(threadArgs->tipoColonia == 1) {
            sem_wait(threadArgs->alimento);
            printf("Thread %d[tipo %d] pegou alimento!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);    
        } else { //Caso tipo 2, pegará primeiro o espaço e depois o alimento
            sem_wait(threadArgs->espaco);
            printf("Thread %d[tipo %d] pegou espaço!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
        }

        //Thread  tentará pegará o segundo recurso
        while (1) {
            if(threadArgs->tipoColonia == 1) {
                printf("Thread %d[tipo %d] tentando pegar espaço\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
                if(sem_trywait(threadArgs->espaco) == 0) { //Caso sem_trywait retorne 0, significa que conseguiu o recurso
                    printf("Thread %d[tipo %d] pegou espaço!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
                    break; //Caso tenha conseguido pegar o recurso
                }

                //Difftime calcula a diferença de tempo dos parâmetros e retorna o valor do tipo double em segundos 
                tempoColetaRecurso = difftime(time(NULL), tempoInicioColetaRecurso);

                if(tempoColetaRecurso >= TIME_LIMIT){ //Caso o tempo de tenha extrapolado irá liberar o recurso 
                    printf("Thread %d[tipo %d] não conseguiu pegar espaço! Liberando alimento!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
                    sem_post(threadArgs->alimento);
                    return NULL;
                }
            } else {
                printf("Thread %d[tipo %d] tentando pegar alimento\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
                if(sem_trywait(threadArgs->alimento) == 0) {
                    printf("Thread %d[tipo %d] pegou alimento!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
                    break; //Caso tenha conseguido pegar o recurso
                }

                //Difftime calcula a diferença de tempo dos parâmetros e retorna o valor do tipo double em segundos
                tempoColetaRecurso = difftime(time(NULL), tempoInicioColetaRecurso);

                if(tempoColetaRecurso >= TIME_LIMIT){ //Caso o tempo de tenha extrapolado irá liberar o recurso
                    printf("Thread %d[tipo %d] não conseguiu pegar alimento! Liberando espaço!\n\n", threadArgs->threadNum, threadArgs->tipoColonia);
                    sem_post(threadArgs->espaco);
                    return NULL;
                }
            }
            sleep(1); //thread dorme por 1s antes de tentar novamente
        }

        //Thread dorme por até 5s após pegar os dois recursos, simulando crescimento
        sleep(rand() % 5);

        //Somatória de crescimento: P(t) = P(0) * e^(rt), no caso, o meu P(0) é o P(t-1), pois calcula a cada passo de tempo, e o t da exponencial será sempre 1
        threadArgs->popAtual = threadArgs->popAtual * exp((threadArgs->txCrescimento / 100));

        //Thread libera os recursos
        sem_post(threadArgs->alimento);
        printf("Thread %d[tipo %d] liberou alimento!\n", threadArgs->threadNum, threadArgs->tipoColonia);
        sem_post(threadArgs->espaco);
        printf("Thread %d[tipo %d] liberou espaço!\n", threadArgs->threadNum, threadArgs->tipoColonia);
        printf("\n");
        printf("Tempo de crescimento da thread %d: %d\n\n", threadArgs->threadNum, threadArgs->tempoAtual);
        threadArgs->tempoAtual ++; 
    }
    //Cálculo do tempo real em segundos que a thread executou
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