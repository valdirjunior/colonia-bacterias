# Colônias  de Bactérias #

Disciplina: Sistemas Operacionais.

Tema: Deadlock.

Aluno: Valdir Rugiski Junior.

Para o aprendizado sobre compartilhamento de recursos por tarefas distintas, foram desenvolvidas aplicações em `C` que simulam cenários em que podem ocorrer deadlocks. As principais bibliotecas utilizadas foram `pthread.h` e `semaphore.h`. Criei 4 versões onde cada uma apresenta uma situação um pouco diferente. O problema simulado é: colônias de bactérias (threads) que disputam por espaço e alimento (semáforos).

**NOTA: As aplicações foram desenvolvidas para sistemas Linux.**

**ATENÇÃO: O foco das aplicações está na compreensão do processamento multithreading e na gestão de recursos compartilhados. Por isso, não levaremos em conta a quantificação física de alimento e espaço, trataremos como recursos ilimitados, porém com acesso controlado pelos semáforos.**

Segue a descrição:

1.  No arquivo [deadlock.c](./deadlock.c) defini 2 tipos de colônias, assim, algumas colônias obtém os recursos na ordem contrária das demais, ocasionando possíveis deadlocks. Não há mecanismos que previnam ou solucionem.

2. No arquivo [ordenacao.c](./ordenacao.c) apliquei o método de Ordenação de Recursos. Todas as colônias vão obter os recursos na mesma ordem. Desta forma elimina-se a possibilidade de uma colônia reter um recurso que outra tentará pegar.

3. No arquivo [hierarquia.c](./hierarquia.c) apliquei o método de Hierarquia de Recursos. Adicionei um novo recurso de controle que chamei de técnico de laboratório. Haverá a mesma quantidade de técnicos em relação aos outros recursos. Cada colônia precisará obter um técnico antes de tentar obter um recurso. Tambem mantive os dois tipos de colônias.

4. No arquivo [timeout.c](./timeout.c) apliquei o método de Timeout. Quando uma colônia pegar o primeiro recurso e não conseguir pegar o segundo, irá tentar novamente durante um tempo predefinido. Caso extrapole o tempo e não tenha obtido o segundo recurso, o primeiro recurso será liberado e irá tentar obter os dois recursos posteriormente.

5. Cada arquivo contém as instruções para compilar, executar e os comandos básicos. Caso preferir, pode consultar na seção [Instruções](#instruções).

# Crescimento Populacional #

As aplicações tem foco no processamento multithreading e no compartilhamento de áreas de memória e recursos. Paralelo a isso, está sendo simulado o crescimento populacional de colônias de bactérias, dado pela equação:

$$P(t) = P(0) . e^{rt}$$

- *P(t)* = População no tempo final;
- *P(0)* = População no tempo 0, ou seja, a inicial;
- *r* = Taxa de crescimento;
- *t* = Tempo em segundos.

Como cada ciclo da thread simula 1s de tempo, o cálculo considera *t* como sendo sempre 1. *P(t)* será a somatória total, e *P(0)* será a população no início de cada ciclo.

# Instruções #

No cabeçalho de cada arquivo existe as instruções de compilação e execução. Para facilitar, deixarei abaixo cada uma. Fique atento às flags de entrada de argumentos, os valores podem ser alterados a sua preferência. Único adendo é sobre o número de recursos, que deve ser inferior ao número de threads. Caso não queira informar o número de threads, será definido automaticamente com base no processador da sua máquina. As demais flags são obrigatórias.

| Flag                                     | Exemplo |
|:-----------------------------------------|:--------|
| -p = população inicial                   | ex: 200 |
| -x = taxa de crescimento em %            | ex: 3%  |
| -t = tempo total simulado em segundos    | ex: 15  |
| -n = número de threads                   | ex: 4   |
| -r = número de recursos de cada tipo     | ex: 1   |
| -h = help                                |

Caso ainda tenha dúvidas, utilize:

    ./nomedoarquivocompilado --help

ou

    ./nomedoarquivocompilado -h    

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
    
# Testes e Resultados #

Foram realizados alguns testes com cenários distintos. Minha máquina pessoal possui somente 4 núcleos, então executo somente 4 threads, variando a quantidade de recursos e o tempo.

O principal cenário testado foi com 4 threads e 1 recurso de cada, com 15 intervalos de tempo. É o cenário com a maior probabilidade de ocorrer deadlock. Com mais recurso disponível foram pouquíssimas vezes que ocorreu, mesmo com mais tempo.

### Segue abaixo tabela com 10 testes em cada cenário do arquivo [deadlock.c](./deadlock.c): ###

**Tempo definido para considerar deadlock = 20s.**

| Recursos | Tempo Simulado(s) | Tempo Real Médio(s) | Deadlocks|
|:--------:|:-----------------:|:--------------------:|:--------:|
| 1 | 15 | 21,6 | 10 |
| 1 | 30 | 22,7 | 10 |
| 2 | 15 | 73,4 | 0 |
| 2 | 30 | 141,9 | 1 |
| 3 | 15 | 48,3 | 0 |
| 3 | 30 | 92,7 | 0 | 

Na situação de escassez de recurso, todos os testes resultaram em deadlock, sendo que algumas poucas vezes avançava o primeiro ciclo das colônias. Quando temos 1 recurso para cada 2 threads, as chances diminuem drásticamente, sendo que ocorreu deadlock somente 1 vez num teste com 30s de tempo. A partir de 3 recursos as chances se tornam mínimas, e em 20 testes não houve deadlock.

#### Teste com 8 threads: ###

**Teste realizado em máquina com somente 4 núcleos. Três execuções para cada cenário.**

| Recursos | Tempo Simulado(s) | Tempo Real Médio(s) | Deadlocks |
|:--------:|:-----------------:|:-------------------:|:---------:|
| 1 | 15 | 22,6 | 3 |
| 2 | 15 | 26| 3 |
| 3 | 15 | 108,3 | 1 |
| 4 | 15 | 82,6 | 0 |

Os resultados podem ser diferentes caso realizado em uma máquina com mais unidades de processamento, mas observa-se comportamento similar ao teste anterior, onde as chances de ocorrer deadlock diminuem a medida que aumenta a oferta de recursos.

### Segue abaixo tabela de comparação de alguns cenários com os métodos de prevenção: ###

**Para os testes a seguir todos os sleeps, antes do início e simulando trabalho, foram fixados em 2s, para uma comparação mais justa**

| Prevenção | Threads | Recursos | Tempo Simulado(s) | Tempo Real(s) |
|:----------|:-------:|:--------:|:-----------------:|:-------------:|
| Ordenação | 4 | 1 | 15 | 122 |
| Hierarquia | 4 | 1 | 15 | 122 |
| Timeout | 4 | 1 | 15 | 80 |
| Ordenação | 4 | 1 | 30 | 242 |
| Hierarquia | 4 | 1 | 30 | 242 |
| Timeout | 4 | 1 | 30 | 144 |
| Ordenação | 4 | 2 | 15 | 62 |
| Hierarquia | 4 | 2 | 15 | 62 |
| Timeout | 4 | 2 | 15 | 80 |
| Ordenação | 4 | 2 | 30 | 122 |
| Hierarquia | 4 | 2 | 30 | 122 |
| Timeout | 4 | 2 | 30 | 140 |
| Ordenação | 4 | 3 | 15 | 62 |
| Hierarquia | 4 | 3 | 15 | 62 |
| Timeout | 4 | 3 | 15 | 62 |
| Ordenação | 4 | 3 | 30 | 122 |
| Hierarquia | 4 | 3 | 30 | 122 |
| Timeout | 4 | 3 | 30 | 122 |
| Ordenação | 4 | 1 | 60 | 484 |
| Hierarquia | 4 | 1 | 60 | 482 |
| Timeout | 4 | 1 | 60 | 262 |

No principal teste o melhor desempenho foi do método [Timeout](./timeout.c), enquanto [Ordenação](./ordenacao.c) e [Hierarquia](./hierarquia.c) tiveram desempenhos praticamente identicos. Nos testes com 4 threads, 1 recurso e 15s, chegou executar em 65% do tempo das outras. No mesmo cenário, mas com 30s, o desempenho foi de 59% do tempo. Ao aumentar o tempo de simulação, a diferença aumanta ainda mais. Com 60s, terminou em 54% do tempo.

A partir do momento que a quantidade de recursos disponíveis aumenta, o desempenho muda bastante. Para os testes com 4 threads e 2 recursos, [Timeout](./timeout.c) perdeu em desepenho. [Ordenação](./ordenacao.c) e [Hierarquia](./hierarquia.c) executaram em 77% e 87% do tempo, para 15s e 30s, respectivamente.

Quando testado com 3 recursos de cada, todos tiveram o mesmo praticamente desempenho para 15s e 30s, em tempo final, variando somente o tempo de cada thread. Não foi testado com o mesmo número de threads e recursos, pois este seria o cenário ideal, onde não haveria disputa por recurso.

### Teste com 8 threads(3 repetições): ###

| Prevenção | Threads | Recursos | Tempo Simulado(s) | Tempo Real Médio(s) |
|:----------|:-------:|:--------:|:-----------------:|:-------------------:|
| Ordenação | 8 | 1 | 15 | 242 |
| Hierarquia | 8 | 1 | 15 | 242,3 |
| Timeout | 8 | 1 | 15 | 94,3 |

Por fim, realizei um teste com maior escassez de recursos, com 8 threads(cpu com somente 4) e somente 1 recurso de cada. Novamente o método de [Timeout](./timeout.c) se mostra mais eficaz, executando em 39% do tempo dos demais.

# Considerações Finais #

Em relação à complexidade dos algoritmos, não tive grandes dificuldades, o maior desafio foi a implementação das flags por linha de comando, e a identificação de deadlock. Esta última, apesar de ser simples, levou algumas horas de testes e pesquisa, talvez por falta de conhecimento, ou por buscar por caminhos ruins.

Os 4 programas são quase idênticos, tendo alterações pontuais para atender a demanda de cada um, sendo o de [Ordenação](./ordenacao.c) o mais simples deles. O de [Timeout](./timeout.c) precisou de vários tentativas até encontrar uma lógica que não falhasse em algumas situações. Ainda poderia ter utilizado neste último a função `sem_timedwait` da biblioteca `pthread.h`, porém tive um pouco de dificuldade e preferi simplificar.

Por fim, pude compreender a importância da gestão dos recursos, para fim de evitar travamento e melhorar o desempenho de tarefas. A demanda da atividade era de implementar somente uma das técnicas, porém decidi me desafiar a fazer as três, podendo assim comparar a eficiência de cada em diferentes cenários.