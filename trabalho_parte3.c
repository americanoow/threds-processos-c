#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

int vetor_concorrente[100000];
int tamanho_concorrente = 100000;

int vetor_validador[100000];
int tamanho_validador = 100000;

sem_t semaforo;
int usar_semaforo = 1;

void inicializar_vetores() {
    srand(time(NULL));
    for (int i = 0; i < 100000; i++) {
        int numero = (rand() % 100) + 1;
        vetor_concorrente[i] = numero;
        vetor_validador[i] = numero;
    }
}

void* remover_pares(void* arg) {
    if (usar_semaforo) {
        sem_wait(&semaforo);
    }
    
    for (int i = tamanho_concorrente - 1; i >= 0; i--) {
        if (i < tamanho_concorrente && vetor_concorrente[i] % 2 == 0) {
            for (int j = i; j < tamanho_concorrente - 1; j++) {
                vetor_concorrente[j] = vetor_concorrente[j + 1];
            }
            tamanho_concorrente--;
        }
    }
    
    if (usar_semaforo) {
        sem_post(&semaforo);
    }
    pthread_exit(NULL);
}

void* remover_multiplos_cinco(void* arg) {
    if (usar_semaforo) {
        sem_wait(&semaforo);
    }
    
    for (int i = tamanho_concorrente - 1; i >= 0; i--) {
        if (i < tamanho_concorrente && vetor_concorrente[i] % 5 == 0) {
            for (int j = i; j < tamanho_concorrente - 1; j++) {
                vetor_concorrente[j] = vetor_concorrente[j + 1];
            }
            tamanho_concorrente--;
        }
    }
    
    if (usar_semaforo) {
        sem_post(&semaforo);
    }
    pthread_exit(NULL);
}

void remocao_sequencial() {
    for (int i = tamanho_validador - 1; i >= 0; i--) {
        if (vetor_validador[i] % 2 == 0 || vetor_validador[i] % 5 == 0) {
            for (int j = i; j < tamanho_validador - 1; j++) {
                vetor_validador[j] = vetor_validador[j + 1];
            }
            tamanho_validador--;
        }
    }
}

int validar_resultados() {
    if (tamanho_concorrente != tamanho_validador) {
        return 0;
    }
    for (int i = 0; i < tamanho_concorrente; i++) {
        if (vetor_concorrente[i] != vetor_validador[i]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    struct timespec tempo_inicio, tempo_fim;
    long long tempo_sem_threads, tempo_com_threads;
    pthread_t t1, t2;

    // --- SETUP DO TESTE 1 ---
    inicializar_vetores();
    sem_init(&semaforo, 0, 1);

    clock_gettime(CLOCK_MONOTONIC, &tempo_inicio);
    remocao_sequencial(); // Cria o gabarito do Teste 1
    clock_gettime(CLOCK_MONOTONIC, &tempo_fim);
    tempo_sem_threads = (tempo_fim.tv_sec - tempo_inicio.tv_sec) * 1000000000LL + (tempo_fim.tv_nsec - tempo_inicio.tv_nsec);

    usar_semaforo = 0;
    pthread_create(&t1, NULL, remover_pares, NULL);
    pthread_create(&t2, NULL, remover_multiplos_cinco, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("--- TESTE SEM SEMAFORO ---\n");
    printf("Resultado correto? %s\n", validar_resultados() ? "SIM" : "NAO");

    // --- SETUP DO TESTE 2 (O B.O. ESTAVA AQUI) ---
    tamanho_concorrente = 100000;
    tamanho_validador = 100000; // Tem que resetar o tamanho validador
    inicializar_vetores();      // Sorteia os novos números aleatórios
    remocao_sequencial();       // Tem que criar um novo gabarito com os novos números

    usar_semaforo = 1;
    clock_gettime(CLOCK_MONOTONIC, &tempo_inicio);
    pthread_create(&t1, NULL, remover_pares, NULL);
    pthread_create(&t2, NULL, remover_multiplos_cinco, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    clock_gettime(CLOCK_MONOTONIC, &tempo_fim);
    tempo_com_threads = (tempo_fim.tv_sec - tempo_inicio.tv_sec) * 1000000000LL + (tempo_fim.tv_nsec - tempo_inicio.tv_nsec);

    printf("\n--- TESTE COM SEMAFORO ---\n");
    printf("Resultado correto? %s\n", validar_resultados() ? "SIM" : "NAO");
    printf("Tempo sequencial (sem threads): %lld ns\n", tempo_sem_threads);
    printf("Tempo concorrente (com threads + semaforo): %lld ns\n", tempo_com_threads);

    sem_destroy(&semaforo);
    return 0;
}