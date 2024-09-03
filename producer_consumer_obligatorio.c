// inclusión de librerías necesarias
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N1 50                                   // Definir el tamaño del buffer 1
#define N2 50                                   // Definir el tamaño del buffer 2
#define SEM_MUTEX "/sem_mutex"                  // Definir el nombre del semáforo mutex, para la exclusión mutua
#define SEM_EMPTY "/sem_empty"                  // Definir el nombre del semáforo empty, para el conteo de los espacios vacíos
#define SEM_FULL "/sem_full"                    // Definir el nombre del semáforo full, para el conteo de los espacios llenos

sem_t *mutex;                                   // Declarar el puntero para el semáforo mutex
sem_t *empty;                                   // Declarar el puntero para el semáforo empty
sem_t *full;                                    // Declarar el puntero para el semáforo full

int BUFFER1[N1];                                // Declarar el buffer con el tamaño del BUFFER1 N1 (que es 50)
int BUFFER2[N2];                                // Declarar el buffer con el tamaño del BUFFER1 N2 (que es 50)
int in1 = 0;                                    // Índice de escritura del BUFFER 1, comenzando en 0
int out1 = 0;                                   // Índice de lectura del BUFFER 1, comenzando en 0
int in2 = 0;                                    // Índice de escritura del BUFFER 2, comenzando en 0
int out2 = 0;                                   // Índice de lectura del BUFFER 2, comenzando en 0

void produce_item(int *item);                   // Declarar la función para producir un ítem y asignarlo al puntero de memoria *item
void enter_item(int item);                      // Declarar la función para ingresar un ítem al buffer
void remove_item(int *item);                    // Declarar la función para remover un ítem del buffer y asignarlo al puntero de memoria *item
void consume_item(int item);                    // Declarar la función para consumir un ítem del buffer
void* producer_thread(void *arg);               // Declarar la función para el hilo del productor
void* consumer_thread(void *arg);               // Declarar la función para el hilo del consumidor

void produce_item(int *item)                    // Definir la función para producir un valor que se asignará al puntero de memoria *item
{
    static int counter = 0;                     // Variable estática para contar los ítems producidos
    *item = counter++;                          // Asignar el valor de counter al puntero *item y luego incrementar counter
}                                               // Por ejemplo, en la primera llamada counter es 0 y se incrementa a 1

void enter_item(int item)                       // Definir la función para ingresar el ítem creado al buffer
{
    BUFFER1[in1] = item;                          // Insertar el ítem en la posición [in] del buffer
    in1 = (in1 + 1) % N1;                          // Incrementar in y usar módulo N para mantenerlo dentro de los límites del buffer
    printf("Se insertó el ítem: %d\n", item);   // Imprimir el ítem que se insertó
}

void remove_item(int *item)                     // Definir la función para remover un ítem del buffer
{
    *item = BUFFER1[out1];                        // Asignar el valor del buffer en la posición [out] al puntero *item
    out1 = (out1 + 1) % N1;                        // Incrementar out y usar módulo N para mantenerlo dentro de los límites del buffer
}

void consume_item(int item)
{
    printf("Se consumió el ítem: %d\n", item);  // Imprimir el ítem que se consumió
}

void* producer_thread(void *arg)                // Función para el hilo del productor
{
    int item;                                   // Variable para almacenar el ítem producido
    while (1) {                                 // Bucle infinito para que el productor produzca indefinidamente
        produce_item(&item);                    // Invocar la función para producir un ítem
        sem_wait(empty);                        // Esperar a que haya un espacio vacío en el buffer
        sem_wait(mutex);                        // Adquirir el semáforo mutex para acceso exclusivo al buffer
        enter_item(item);                       // Ingresar el ítem al buffer
        sem_post(mutex);                        // Liberar el semáforo mutex
        sem_post(full);                         // Señalar que hay un nuevo ítem en el buffer
        sleep(1);                               // Dormir 1 segundo, simulando tiempo de producción
    }
    return NULL;
}

void* consumer_thread(void *arg)                // Función para el hilo del consumidor
{
    int item;                                   // Variable para almacenar el ítem consumido
    while (1) {                                 // Bucle infinito para que el consumidor consuma indefinidamente
        sem_wait(full);                         // Esperar a que haya un ítem disponible en el buffer
        sem_wait(mutex);                        // Adquirir el semáforo mutex para acceso exclusivo al buffer
        remove_item(&item);                     // Remover el ítem del buffer
        sem_post(mutex);                        // Liberar el semáforo mutex
        sem_post(empty);                        // Señalar que se ha liberado un espacio en el buffer
        consume_item(item);                     // Consumir el ítem removido del buffer
        sleep(1);                               // Dormir 1 segundo, simulando tiempo de consumo
    }
    return NULL;
}

int main()                                      // Punto de entrada al programa
{
    pthread_t prod, cons;                       // Crear hilos para productor y consumidor

    mutex = sem_open(SEM_MUTEX, O_CREAT, 0644, 1);  // Crear o abrir el semáforo mutex, inicializado en 1
    empty = sem_open(SEM_EMPTY, O_CREAT, 0644, N);  // Crear o abrir el semáforo empty, inicializado en N (indica que el buffer está vacío)
    full = sem_open(SEM_FULL, O_CREAT, 0644, 0);    // Crear o abrir el semáforo full, inicializado en 0 (indica que el buffer está vacío)

    // Verificar si hubo algún error al crear los semáforos
    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Crear el hilo del productor
    if (pthread_create(&prod, NULL, producer_thread, NULL) != 0) {
        perror("pthread_create producer");
        exit(EXIT_FAILURE);
    }
    // Crear el hilo del consumidor
    if (pthread_create(&cons, NULL, consumer_thread, NULL) != 0) {
        perror("pthread_create consumer");
        exit(EXIT_FAILURE);
    }

    pthread_join(prod, NULL);                       // Esperar a que el hilo del productor termine
    pthread_join(cons, NULL);                       // Esperar a que el hilo del consumidor termine

    // Cerrar y eliminar los semáforos
    sem_close(mutex);
    sem_close(empty);
    sem_close(full);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_EMPTY);
    sem_unlink(SEM_FULL);

    return 0;                                       // Devolver 0 si el programa se ejecuta sin errores
}
