#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <time.h>

struct dados_compartilhados {
    int vetor[100000];
    int tamanho;
};

int vetor_validador[100000];
int tamanho_validador = 100000;

void inicializar_dados(struct dados_compartilhados* dc) {
    srand(time(NULL));
    dc->tamanho = 100000;
    for (int i = 0; i < 100000; i++) {
        int numero = (rand() % 100) + 1;
        dc->vetor[i] = numero;
        vetor_validador[i] = numero;
    }
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

int validar_resultados(struct dados_compartilhados* dc) {
    if (dc->tamanho != tamanho_validador) {
        return 0;
    }
    for (int i = 0; i < dc->tamanho; i++) {
        if (dc->vetor[i] != vetor_validador[i]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    struct timespec tempo_inicio, tempo_fim;
    long long tempo_sem_processos, tempo_com_processos;

    int mem_fd = shm_open("/minha_memoria", O_CREAT | O_RDWR, 0666);
    ftruncate(mem_fd, sizeof(struct dados_compartilhados));
    struct dados_compartilhados* dc = mmap(NULL, sizeof(struct dados_compartilhados), PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0);

    inicializar_dados(dc);

    sem_t* semaforo = sem_open("/meu_semaforo", O_CREAT, 0666, 1);

    clock_gettime(CLOCK_MONOTONIC, &tempo_inicio);
    remocao_sequencial();
    clock_gettime(CLOCK_MONOTONIC, &tempo_fim);
    tempo_sem_processos = (tempo_fim.tv_sec - tempo_inicio.tv_sec) * 1000000000LL + (tempo_fim.tv_nsec - tempo_inicio.tv_nsec);

    clock_gettime(CLOCK_MONOTONIC, &tempo_inicio);

    pid_t pid1 = fork();

    if (pid1 == 0) {
        for (int i = 100000 - 1; i >= 0; i--) {
            sem_wait(semaforo);
            if (i < dc->tamanho && dc->vetor[i] % 2 == 0) {
                for (int j = i; j < dc->tamanho - 1; j++) {
                    dc->vetor[j] = dc->vetor[j + 1];
                }
                dc->tamanho--;
            }
            sem_post(semaforo);
        }
        exit(0);
    }

    pid_t pid2 = fork();

    if (pid2 == 0) {
        for (int i = 100000 - 1; i >= 0; i--) {
            sem_wait(semaforo);
            if (i < dc->tamanho && dc->vetor[i] % 5 == 0) {
                for (int j = i; j < dc->tamanho - 1; j++) {
                    dc->vetor[j] = dc->vetor[j + 1];
                }
                dc->tamanho--;
            }
            sem_post(semaforo);
        }
        exit(0);
    }

    wait(NULL);
    wait(NULL);

    clock_gettime(CLOCK_MONOTONIC, &tempo_fim);
    tempo_com_processos = (tempo_fim.tv_sec - tempo_inicio.tv_sec) * 1000000000LL + (tempo_fim.tv_nsec - tempo_inicio.tv_nsec);

    printf("Resultado correto? %s\n", validar_resultados(dc) ? "SIM" : "NAO");
    printf("Tempo sequencial (sem processos): %lld ns\n", tempo_sem_processos);
    printf("Tempo concorrente (com processos + memoria compartilhada): %lld ns\n", tempo_com_processos);

    sem_close(semaforo);
    sem_unlink("/meu_semaforo");
    munmap(dc, sizeof(struct dados_compartilhados));
    shm_unlink("/minha_memoria");

    return 0;
}