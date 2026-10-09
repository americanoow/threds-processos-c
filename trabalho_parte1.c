#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>


void* executar_thread(void *arg){
    unsigned long yy_pthread = (unsigned long)pthread_self();
    unsigned long zz_tdi = (unsigned long)syscall(SYS_gettid);
    printf("Eu sou a thread com TID %lu e %lu\n", yy_pthread, zz_tdi);
    pthread_exit(NULL);
}

int main(){
    int n;

    printf("Digite o numero de threads: ");
    if(scanf("%d", &n) != 1 || n<=0){
        printf("Digite um numero positivo!\n");
        return 1;
    }

    pthread_t *threads = malloc(n * sizeof(pthread_t));
    if (threads == NULL) {
        perror("Erro ao alocar memória");
        return 1;
    }

    for(int i = 0; i < n; i++){
        if (pthread_create(&threads[i], NULL, executar_thread, NULL) !=0 ){
            perror("Erro ao criar thread");
            free(threads);
            return 1;

        }
    }

    for (int i = 0; i < n; i++){
        pthread_join(threads[i], NULL);
    }

    free(threads);
    return 0;

}