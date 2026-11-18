#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 5              // Número de filósofos (y tenedores)

// Estados de los filósofos
#define THINKING 0
#define HUNGRY 1
#define EATING 2

// Macros para P(s) y V(s)
#define P(s) sem_wait(s)
#define V(s) sem_post(s)

int state[N];            // Estado de cada filósofo
sem_t mutex;             // Exclusión mutua para modificar los estados
sem_t S[N];              // Un semáforo por cada filósofo para bloquearse si no puede comer

// Función 'test'
void test(int i) {
    if (state[i] == HUNGRY && 
        state[(i + N - 1) % N] != EATING && 
        state[(i + 1) % N] != EATING) {
        
        state[i] = EATING;
        printf("[FILÓSOFO %d] ¡Puede comer! Cambia a estado EATING.\n", i);
        V(&S[i]); // Libera al filósofo para que coma
    }
}

// Función para adquirir los tenedores / intentar comer
void take_forks(int i) {
    P(&mutex);
    state[i] = HUNGRY;
    printf("[FILÓSOFO %d] Tiene hambre (HUNGRY). Verificando vecinos...\n", i);
    test(i);
    V(&mutex);
    P(&S[i]); // Si no pudo comer, se bloquea aquí hasta que un vecino lo despierte
}

// Función para dejar los tenedores / terminar de comer
void put_forks(int i) {
    P(&mutex);
    state[i] = THINKING;
    printf("[FILÓSOFO %d] Terminó de comer. Vuelve a pensar (THINKING).\n", i);
    
    // Comprueba si los vecinos de la izquierda y derecha pueden comer ahora
    test((i + N - 1) % N);
    test((i + 1) % N);
    
    V(&mutex);
}

// Rutina que ejecuta cada hilo (filósofo)
void* filosofo(void* num) {
    int i = *((int*) num);

    while (1) {
        // Pensar
        printf("[FILÓSOFO %d] Está pensando...\n", i);
        sleep(rand() % 3 + 1); // Simula el tiempo pensando

        // Intentar comer
        take_forks(i);

        // Comer
        printf("[FILÓSOFO %d] Está COMIENDO la cena :P...\n", i);
        sleep(rand() % 3 + 1); // Simula el tiempo comiendo

        // Dejar tenedores
        put_forks(i);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t hilos[N];
    int ids[N];

    // Inicializar semáforos
    sem_init(&mutex, 0, 1);
    for (int i = 0; i < N; i++) {
        sem_init(&S[i], 0, 0);
        state[i] = THINKING;
        ids[i] = i;
    }

    // Crear los 5 hilos de los filósofos
    for (int i = 0; i < N; i++) {
        pthread_create(&hilos[i], NULL, filosofo, &ids[i]);
    }

    // Como es un bucle infinito (while true), esperamos a que los hilos corran
    for (int i = 0; i < N; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Destruir semáforos (en caso de salir)
    sem_destroy(&mutex);
    for (int i = 0; i < N; i++) {
        sem_destroy(&S[i]);
    }

    return EXIT_SUCCESS;
}