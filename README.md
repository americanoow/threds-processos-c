# Trabalho de SO: threads, processos e memória compartilhada

Oi! Este repositório é o trabalho de Sistemas Operacionais em que eu mexi com threads e processos em C, no Linux. O objetivo era entender na prática o que acontece quando um programa cria "vários caminhos de execução" ao mesmo tempo, e o que dá errado quando eles tentam mexer nos mesmos dados.

O enunciado completo está resumido abaixo, com as respostas que eu encontrei.

## O que cada parte faz

**Parte 1: criando threads.** O programa pede um número N, cria N threads e cada uma imprime o próprio identificador. Serve para ver que threads são independentes e que a ordem em que elas rodam não é garantida.

**Parte 2: somando matrizes.** Duas matrizes 500x500 são somadas de duas formas: sequencial e com 4 threads, cada uma cuidando de 125 linhas. No fim, comparo o tempo das duas com precisão de nanossegundos.

**Parte 3: removendo números do vetor.** Um vetor com 100 mil números aleatórios (entre 1 e 100) passa por duas threads ao mesmo tempo. Uma remove os pares, a outra remove os múltiplos de 5. Ambas mexem no mesmo vetor, e é aí que mora o problema. O programa roda sem semáforo (e dá errado) e com semáforo (e dá certo). Também existe uma função que calcula o resultado de forma sequencial, para eu conferir se o resultado com threads está certo.

**Parte 4: a mesma coisa, mas com processos.** Em vez de threads, uso `fork()` para criar dois processos. Como processos não compartilham memória por padrão, os dados ficam numa área de memória compartilhada criada com `shm_open` e `mmap`.

## As respostas que o enunciado pede

**As saídas da Parte 1.** As threads rodam ao mesmo tempo, então a ordem das linhas impressas muda de uma execução para outra. Às vezes duas linhas aparecem embaralhadas. Isso não é erro: é o comportamento normal de concorrência. Também dá para ver que cada thread tem um identificador diferente.

**`pthread_self` e `gettid` retornam o mesmo valor?** Não. Eu testei e os números são bem diferentes:

- `pthread_self()` devolve um identificador da biblioteca de threads (pthreads). É um número grande, parecido com um endereço de memória, e só faz sentido dentro do processo.
- `gettid()` devolve o identificador que o kernel do Linux dá para a thread. É um número pequeno e único no sistema inteiro, o mesmo que aparece no `top` ou no `ps`.

Em uma execução, a saída foi algo como `TID 128872932701888 e 547`. O primeiro número é do pthreads e o segundo é do kernel.

**Qual é mais rápido na Parte 3?** O sequencial. Numa execução, o sequencial levou cerca de 5,3 segundos e a versão com threads e semáforo levou cerca de 6,2 segundos. O semáforo obriga uma thread a esperar a outra terminar, então elas não trabalham de verdade em paralelo. Aqui o ganho foi a correção, não a velocidade.

**O que acontece sem semáforo?** As duas threads mexem no vetor ao mesmo tempo. Quando uma remove um elemento e desloca o resto, a outra pode estar olhando para uma posição que já mudou. O resultado vira uma mistura das duas operações, e o tamanho final do vetor fica errado. Numa execução, o programa acusou `NAO` no teste de validação.

**Como eu sei que o resultado com threads está certo?** Tenho uma função que faz a mesma remoção de forma sequencial e guarda o resultado num vetor separado. Depois comparo elemento por elemento. Se o tamanho e os valores batem, o resultado é `SIM`.

## Resultados que eu obtive

Rodei no WSL (Ubuntu), compilando com `gcc -Wall -pthread`. Os tempos mudam de máquina para máquina e até de uma execução para outra, então trate os números como uma referência.

| Parte | Resultado |
|---|---|
| 1 | Cada thread imprimiu um TID diferente |
| 2 | Sequencial: 1,39 ms · Com 4 threads: 1,12 ms |
| 3 | Sem semáforo: `NAO` · Com semáforo: `SIM` |
| 4 | Com processos e memória compartilhada: `SIM` |

Na Parte 2, a diferença é pequena porque a matriz é relativamente pequena e criar threads tem um custo. Matrizes maiores tendem a mostrar mais ganho.

## Como rodar

Os programas usam bibliotecas do Linux, então no Windows o caminho é o WSL.

```bash
gcc -Wall -pthread -o parte1 trabalho_parte1.c
gcc -Wall -pthread -o parte2 trabalho_parte2.c
gcc -Wall -pthread -o parte3 trabalho_parte3.c
gcc -Wall -pthread -o parte4 trabalho_parte4.c -lrt

./parte1
./parte2
./parte3
./parte4
```

O `-lrt` na Parte 4 é necessário em algumas versões do glibc, por causa da memória compartilhada.

Como a semente do gerador aleatório muda com o horário (`time(NULL)`), cada execução sorteia números diferentes. Vale rodar a Parte 3 algumas vezes para ver se o resultado sem semáforo continua errado.

## O que eu levei de aprendizado

- Threads dividem a memória, e isso é ótimo até duas delas escreverem no mesmo lugar ao mesmo tempo.
- Um programa pode dar certo numa execução e errado na outra. Por isso compensa ter uma forma de conferir o resultado.
- Paralelizar nem sempre deixa o código mais rápido. Sincronização e criação de threads custam tempo.
