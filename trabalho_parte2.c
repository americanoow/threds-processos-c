#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define tamanho 500

int matriz_a[tamanho][tamanho];
int matriz_b[tamanho][tamanho];
int resultado_sem_threads[tamanho][tamanho];
int resultado_com_threads[tamanho][tamanho];

struct limites_matriz {
    int linha_inicio;
    int linha_fim;
};

void inicializar_matrizes() {
    srand(time(NULL));
    for (int i = 0; i < tamanho; i++) {
        for (int j = 0; j < tamanho; j++) {
            matriz_a[i][j] = rand() % 10;
            matriz_b[i][j] = rand() % 10;
        }
    }
}

void soma_normal() {
    for (int i = 0; i < tamanho; i++) {
        for (int j = 0; j < tamanho; j++) {
            resultado_sem_threads[i][j] = matriz_a[i][j] + matriz_b[i][j];
        }
    }
}

void* soma_bloco_thread(void* arg) {
    struct limites_matriz* dados = (struct limites_matriz*) arg;
    
    for (int i = dados->linha_inicio; i < dados->linha_fim; i++) {
        for (int j = 0; j < tamanho; j++) {
            resultado_com_threads[i][j] = matriz_a[i][j] + matriz_b[i][j];
        }
    }
    
    pthread_exit(NULL);
}

int main() {
    struct timespec tempo_inicio, tempo_fim;
    long long tempo_sequencial, tempo_concorrente;

    inicializar_matrizes();

    clock_gettime(CLOCK_MONOTONIC, &tempo_inicio);
    soma_normal();
    clock_gettime(CLOCK_MONOTONIC, &tempo_fim);
    
    tempo_sequencial = (tempo_fim.tv_sec - tempo_inicio.tv_sec) * 1000000000LL + (tempo_fim.tv_nsec - tempo_inicio.tv_nsec);

    pthread_t id_threads[4];
    struct limites_matriz dados_threads[4];
    int linhas_por_thread = 125;

    clock_gettime(CLOCK_MONOTONIC, &tempo_inicio);

    for (int i = 0; i < 4; i++) {
        dados_threads[i].linha_inicio = i * linhas_por_thread;
        dados_threads[i].linha_fim = (i + 1) * linhas_por_thread;
        
        pthread_create(&id_threads[i], NULL, soma_bloco_thread, (void*)&dados_threads[i]);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(id_threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &tempo_fim);
    tempo_concorrente = (tempo_fim.tv_sec - tempo_inicio.tv_sec) * 1000000000LL + (tempo_fim.tv_nsec - tempo_inicio.tv_nsec);

    printf("Tempo sem threads: %lld ns\n", tempo_sequencial);
    printf("Tempo com threads: %lld ns\n", tempo_concorrente);

    return 0;
}