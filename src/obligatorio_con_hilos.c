#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>   // Para sem_open
#include <sys/stat.h> // Para permisos de sem_open

#define BUFFER_SIZE 50                           // Definir el tamaño de los buffers circulares
#define SEM_MUTEX1 "/sem_mutex1"                 // Nombre del semáforo mutex para buffer1
#define SEM_EMPTY1 "/sem_empty1"                 // Nombre del semáforo empty para buffer1
#define SEM_FULL1 "/sem_full1"                   // Nombre del semáforo full para buffer1
#define SEM_MUTEX2 "/sem_mutex2"                 // Nombre del semáforo mutex para buffer2
#define SEM_EMPTY2 "/sem_empty2"                 // Nombre del semáforo empty para buffer2
#define SEM_FULL2 "/sem_full2"                   // Nombre del semáforo full para buffer2

// Semáforos para los buffers
sem_t *mutex1, *empty1, *full1;                  // Semáforos para buffer1
sem_t *mutex2, *empty2, *full2;                  // Semáforos para buffer2

// Buffers circulares y sus índices
int buffer1[BUFFER_SIZE];                       // Buffer 1
int buffer2[BUFFER_SIZE];                       // Buffer 2
int in1 = 0, out1 = 0;                          // Índices para buffer1
int in2 = 0, out2 = 0;                          // Índices para buffer2

// Función para generar un ítem aleatorio (simula la generación de datos)
void produce_item(int *item) {
    *item = rand() % 100;                        // Genera un número aleatorio entre 0 y 99
}

// Función para insertar un ítem en buffer1
void enter_item_buffer1(int item) {
    buffer1[in1] = item;                         // Inserta el ítem en la posición 'in' del buffer1
    printf("P1 insertó el ítem en buffer1: %d\n", item);
    in1 = (in1 + 1) % BUFFER_SIZE;               // Actualiza el índice 'in' circularmente
}

// Función para remover un ítem del buffer1
void remove_item_buffer1(int *item) {
    *item = buffer1[out1];                       // Extrae el ítem de la posición 'out' del buffer1
    printf("P2 extrajo el ítem de buffer1: %d\n", *item);
    out1 = (out1 + 1) % BUFFER_SIZE;             // Actualiza el índice 'out' circularmente
}

// Función para insertar un ítem en buffer2
void enter_item_buffer2(int item) {
    buffer2[in2] = item;                         // Inserta el ítem en la posición 'in' del buffer2
    printf("P3 insertó la suma en buffer2: %d\n", item);
    in2 = (in2 + 1) % BUFFER_SIZE;               // Actualiza el índice 'in' circularmente
}

// Función para remover un ítem del buffer2
void remove_item_buffer2(int *item) {
    *item = buffer2[out2];                       // Extrae el ítem de la posición 'out' del buffer2
    printf("P4 extrajo el ítem de buffer2: %d\n", *item);
    out2 = (out2 + 1) % BUFFER_SIZE;             // Actualiza el índice 'out' circularmente
}

// Función para el hilo P1: Genera números aleatorios e inserta en buffer1
void* P1(void *arg) {
    int item;
    while (1) {
        produce_item(&item);                     // Genera un ítem
        sem_wait(empty1);                       // Espera a que haya espacio vacío en buffer1
        sem_wait(mutex1);                       // Adquiere el mutex para buffer1
        enter_item_buffer1(item);               // Inserta el ítem en buffer1
        sem_post(mutex1);                       // Libera el mutex
        sem_post(full1);                        // Señala que hay un nuevo ítem en buffer1
        sleep(1);                              // Simula tiempo de procesamiento
    }
    return NULL;
}

// Función para el hilo P2: Extrae de buffer1, eleva al cuadrado e inserta en buffer1
void* P2(void *arg) {
    int item;
    while (1) {
        sem_wait(full1);                        // Espera a que haya un ítem disponible en buffer1
        sem_wait(mutex1);                      // Adquiere el mutex para buffer1
        remove_item_buffer1(&item);            // Extrae el ítem de buffer1
        item = item * item;                    // Eleva el ítem al cuadrado
        sem_post(mutex1);                      // Libera el mutex
        sem_post(empty1);                      // Señala que hay un espacio vacío en buffer1
        sem_wait(empty1);                     // Espera a que haya espacio vacío en buffer1
        sem_wait(mutex1);                      // Adquiere el mutex para buffer1
        enter_item_buffer1(item);              // Inserta el ítem al cuadrado en buffer1
        sem_post(mutex1);                      // Libera el mutex
        sem_post(full1);                       // Señala que hay un nuevo ítem en buffer1
        sleep(1);                             // Simula tiempo de procesamiento
    }
    return NULL;
}

// Función para el hilo P3: Extrae dos valores de buffer1, los suma e inserta en buffer2
void* P3(void *arg) {
    int x, y, sum;
    while (1) {
        sem_wait(full1);                        // Espera a que haya un ítem disponible en buffer1
        sem_wait(mutex1);                      // Adquiere el mutex para buffer1
        remove_item_buffer1(&x);               // Extrae el primer ítem de buffer1
        sem_post(mutex1);                      // Libera el mutex
        sem_post(empty1);                      // Señala que hay un espacio vacío en buffer1

        sem_wait(full1);                        // Espera a que haya otro ítem disponible en buffer1
        sem_wait(mutex1);                      // Adquiere el mutex para buffer1
        remove_item_buffer1(&y);               // Extrae el segundo ítem de buffer1
        sem_post(mutex1);                      // Libera el mutex
        sem_post(empty1);                      // Señala que hay un espacio vacío en buffer1

        sum = x + y;                           // Calcula la suma de los dos ítems
        printf("P3 sumó los ítems %d y %d, resultado %d\n", x, y, sum);

        sem_wait(empty2);                      // Espera a que haya espacio vacío en buffer2
        sem_wait(mutex2);                     // Adquiere el mutex para buffer2
        enter_item_buffer2(sum);              // Inserta la suma en buffer2
        sem_post(mutex2);                     // Libera el mutex
        sem_post(full2);                      // Señala que hay un nuevo ítem en buffer2
        sleep(1);                            // Simula tiempo de procesamiento
    }
    return NULL;
}

// Función para el hilo P4: Extrae de buffer2 e imprime
void* P4(void *arg) {
    int item;
    while (1) {
        sem_wait(full2);                       // Espera a que haya un ítem disponible en buffer2
        sem_wait(mutex2);                     // Adquiere el mutex para buffer2
        remove_item_buffer2(&item);           // Extrae el ítem de buffer2
        sem_post(mutex2);                     // Libera el mutex
        sem_post(empty2);                     // Señala que hay un espacio vacío en buffer2
        printf("P4 imprimió el ítem: %d\n", item); // Imprime el ítem extraído
        sleep(1);                            // Simula tiempo de procesamiento
    }
    return NULL;
}

int main() {
    pthread_t threads[4];                     // Array para almacenar los identificadores de los hilos

    // Inicializar semáforos
    mutex1 = sem_open(SEM_MUTEX1, O_CREAT, 0644, 1);  // Crear o abrir el semáforo mutex para buffer1
    empty1 = sem_open(SEM_EMPTY1, O_CREAT, 0644, BUFFER_SIZE);  // Crear o abrir el semáforo empty para buffer1
    full1 = sem_open(SEM_FULL1, O_CREAT, 0644, 0);    // Crear o abrir el semáforo full para buffer1

    mutex2 = sem_open(SEM_MUTEX2, O_CREAT, 0644, 1);  // Crear o abrir el semáforo mutex para buffer2
    empty2 = sem_open(SEM_EMPTY2, O_CREAT, 0644, BUFFER_SIZE);  // Crear o abrir el semáforo empty para buffer2
    full2 = sem_open(SEM_FULL2, O_CREAT, 0644, 0);    // Crear o abrir el semáforo full para buffer2

    // Verificar si hubo algún error al crear los semáforos
    if (mutex1 == SEM_FAILED || empty1 == SEM_FAILED || full1 == SEM_FAILED ||
        mutex2 == SEM_FAILED || empty2 == SEM_FAILED || full2 == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Crear los hilos para los procesos P1, P2, P3, y P4
    if (pthread_create(&threads[0], NULL, P1, NULL) != 0) {
        perror("pthread_create P1");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&threads[1], NULL, P2, NULL) != 0) {
        perror("pthread_create P2");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&threads[2], NULL, P3, NULL) != 0) {
        perror("pthread_create P3");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&threads[3], NULL, P4, NULL) != 0) {
        perror("pthread_create P4");
        exit(EXIT_FAILURE);
    }

    // Esperar a que todos los hilos terminen (en realidad, estos hilos corren indefinidamente)
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    // Cerrar y eliminar los semáforos
    sem_close(mutex1);
    sem_close(empty1);
    sem_close(full1);
    sem_unlink(SEM_MUTEX1);
    sem_unlink(SEM_EMPTY1);
    sem_unlink(SEM_FULL1);

    sem_close(mutex2);
    sem_close(empty2);
    sem_close(full2);
    sem_unlink(SEM_MUTEX2);
    sem_unlink(SEM_EMPTY2);
    sem_unlink(SEM_FULL2);

    return 0;  // Finaliza el programa
}
