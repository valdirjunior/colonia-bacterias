# Colônia  de Bactérias #

Para o aprendizado sobre compartilhamento de recursos por tarefas distintas, foram desenvolvidas aplicações em `C` que simulam cenários em que podem ocorrer deadlocks. As principais bibliotecas utilizadas foram `pthread.h` e `semaphore.h`. Criei 4 versões onde cada uma apresenta uma situação um pouco diferente. O problema simulado é: colônias de bactérias (threads) que disputam por espaço e alimento (semáforos).

Segue a descrição:


1.  No arquivo [deadlock.c](./deadlock.c) defini 2 tipos de colônias, assim, algumas colônias obtém os recursos na ordem contrária das demais, ocasionando possíveis deadlocks. Não há mecanismos que previnam ou solucionem.

2. No arquivo [ordenacao.c](./ordenacao.c) apliquei o método de Ordenação de Recursos. Todas as colônias vão obter os recursos na mesma ordem. Desta forma elimina-se a possibilidade de uma colônia reter um recurso que outra tentará pegar.

3. No arquivo [hierarquia.c](./hierarquia.c) apliquei o método de Hierarquia de Recursos. Adicionei um novo recurso de controle que chamei de técnico de laboratório. Haverá a mesma quantidade de técnicos em relação aos outros recursos. Cada colônia precisará obter um técnico antes de tentar obter um recurso. Tambem mantive os dois tipos de colônias.

4. No arquivo [timeout.c](./timeout.c) apliquei o método de Timeout. Quando uma colônia pegar o primeiro recurso e não conseguir pegar o segundo, irá tentar novamente durante um tempo predefinido. Caso extrapole o tempo e não tenha obtido o segundo recurso, o primeiro recurso será liberado e irá tentar novamente obter os dois recursos posteriormente.

5. Cada arquivo contém as instruções para compilar, executar e os comandos básicos. Caso preferir, pode consultar na seção [Instruções](#instruções)


# Crescimento Populacional #

As aplicações tem foco no processamento multi-thread e no compartilhamento de áreas de memória e recursos. Paralelo a isso, está sendo simulado o crescimento populacional de colônias de bactérias, dado pela equação:

$$P(t) = P(0) . e^{rt}$$

- *P(t)* = População no tempo final;
- *P(0)* = População no tempo 0, ou seja, a inicial;
- *r* = Taxa de crescimento;
- *t* = Tempo em segundos.

Como cada ciclo da thread simula 1s de tempo, o cálculo considera *t* como sendo sempre 1. *P(t)* será a somatória total, e *P(0)* será a população no início de cada ciclo. No problema, não levamos em conta a quantidade de alimento ou a aréa disponível, supomos que as quantidades sejam ilimitadas. Os recursos em questão são tratados como vias de acesso à essas regalias, ou seja, se eu executar com 1 recurso de cada, significa que tenho 1 via de acesso, 1 caminho, para alimento e para espaço. Desta maneira, conseguimos compreender o tema principal(compartilhamento de recursos) sem elevar considerávelmente a complexidade do problema.

# Instruções #

No cabeçalho de cada arquivo existe as instruções de compilação e execução. Para facilitar, deixarei abaixo cada uma. Fique atento às flags de entrada de argumentos, os valores podem ser alterados a sua preferência. Único adendo é sobre o número de recursos, que deve ser inferior ao número de threads. Caso não queira informar o número de threads, será definido automaticamente com base no processador da sua máquina. As demais flags são obrigatórias.

-p = população inicial                   | ex: 200

-x = taxa de crescimento em %            | ex: 3%

-t = tempo total simulado em segundos    | ex: 15

-n = número de threads                   | ex: 4

-r = número de recursos de cada tipo     | ex: 1

-h = help

Caso ainda tenha dúvidas, utilize:

    ./nomedoarquivocompilado --help

ou

    ./nomedoarquivocompilado -H    



## Deadlocks ##

[deadlock.c](./deadlock.c)

Compilação:

    gcc -o deadlock deadlock.c -lpthread -lrt -lm

Execução:

    ./deadlock -p 200 -x 3 -t 15 -n 4 -r 1

## Ordenação ##

[ordenacao.c](./ordenacao.c)

Compilação:
    
    gcc -o ordenacao ordenacao.c -lpthread -lrt -lm

Execução: 

    ./ordenacao -p 200 -x 3 -t 15 -n 4 -r 1

## Hierarquia ##

[hierarquia.c](./hierarquia.c)

Compilação:
    
    gcc -o hierarquia hierarquia.c -lpthread -lrt -lm

Execução: 

    ./hierarquia -p 200 -x 3 -t 15 -n 4 -r 1

## Timeout ##

[timeout.c](./timeout.c)

Compilação:
    
    gcc -o timeout timeout.c -lpthread -lrt -lm

Execução: 

    ./timeout -p 200 -x 3 -t 15 -n 4 -r 1
    
# Considerações Finais #

Foram realizados alguns testes com cenários distintos. Minha máquina pessoal possui somente 4 núcleos, então executo somente 4 threads, variando a quantidade de recursos e o tempo.

O principal cenário testado foi com 4 threads e 1 recurso de cada, com 15 intervalos de tempo. É o cenário com a maior probabilidade de ocorrer deadlock. Com mais recurso disponível foram pouquíssimas vezes que ocorreu, mesmo com mais tempo.

Para 4 threads, 1 recurso e 15s, o melhor desempenho foi do método [Timeout](./timeout.c).