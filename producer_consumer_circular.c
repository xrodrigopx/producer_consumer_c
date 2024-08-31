// inclusión de librerías necesarias
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 10                                    //definir el tamaño del buffer
#define SEM_MUTEX "/sem_mutex"                  //definir el semaforo mutex, para la exclusion mutua
#define SEM_EMPTY "/sem_empty"                  //definir el semaforo empty, para el calculo de los espacios vacios
#define SEM_FULL "/sem_full"                    //definir el semaforo full, para el calculo de los espacios llenos

sem_t *mutex;                                   //declarar el semaforo mutex
sem_t *empty;                                   //declarar el semaforo empty
sem_t *full;                                    //declarar el semaforo full

int buffer[N];                                  //declarar el buffer
int in = 0;                                     //declarar la variable para el numero de inseciones, comienza en 0, no hay nada en el buffer todavía
int out = 0;                                    //declarar la variable para el numero de extracciones, comienza en 0, no se ha retirado nada del buffer todavía

void produce_item(int *item);                   //declarar la funcion para producir un item al buffer
void enter_item(int item);                      //declarar la funcion para ingresar un item al buffer
void remove_item(int *item);                    //declarar la funcion para retirar un item del buffer
void consume_item(int item);                    //declarar la funcion para consumir un item del buffer
void* producer_thread(void *arg);               //declarar la funcion para el hilo del productor
void* consumer_thread(void *arg);               //declarar la funcion para el hilo del consumidor

void produce_item(int *item)                    //definir la funcion para producir un item al buffer
{
    static int counter = 0;                     //definir la variable counter, comienza en 0
    *item = counter++;                          //el item es igual a counter, y luego se incrementa en 1, porque es lo que se produjo, todavía no se ha insertado
}

void enter_item(int item)                       //definir la función para ingresar el item creado al buffer
{
    buffer[in] = item;                          //el buffer en la posición in es igual al item que se produjo
    in = (in + 1) % N;                          //in se incrementa en 1, y se calcula el modulo N, para que si in llega a N, vuelva a 0, este es el efecto circular
    printf("Produced item: %d\n", item);        //imprimir el item que se produjo
}

void remove_item(int *item)                     //definir la función para remover un item del buffer
{
    *item = buffer[out];                        //el item es igual al buffer en la posición out, esto quiere decir que se va a consumir
    out = (out + 1) % N;
}

void consume_item(int item)
{
    printf("Consumed item: %d\n", item);
}

void* producer_thread(void *arg)
{
    int item;
    while (1) {
        produce_item(&item);
        sem_wait(empty);
        sem_wait(mutex);
        enter_item(item);
        sem_post(mutex);
        sem_post(full);
        sleep(1);
    }
    return NULL;
}

void* consumer_thread(void *arg)
{
    int item;
    while (1) {
        sem_wait(full);
        sem_wait(mutex);
        remove_item(&item);
        sem_post(mutex);
        sem_post(empty);
        consume_item(item);
        sleep(1);
    }
    return NULL;
}

int main()
{
    pthread_t prod, cons;

    // Initialize named semaphores
    mutex = sem_open(SEM_MUTEX, O_CREAT, 0644, 1);
    empty = sem_open(SEM_EMPTY, O_CREAT, 0644, N);
    full = sem_open(SEM_FULL, O_CREAT, 0644, 0);

    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Create threads
    if (pthread_create(&prod, NULL, producer_thread, NULL) != 0) {
        perror("pthread_create producer");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&cons, NULL, consumer_thread, NULL) != 0) {
        perror("pthread_create consumer");
        exit(EXIT_FAILURE);
    }

    // Wait for threads to finish (they won't in this example)
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    // Clean up named semaphores
    sem_close(mutex);
    sem_close(empty);
    sem_close(full);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_EMPTY);
    sem_unlink(SEM_FULL);

    return 0;
}

/*
EL PROBLEMA DE LA SECCION CRITICA

1- exclusion mutua: si el proceso P esta sejecutando en la seccion critica, en la misma sección otro proceso no puede entrar a ejecutar. ejemplo: si tenemos una sola bici y somos 10 personas, solo una persona puede andar en ella, los otros debemos esperar

2- progress: si tenemos dos procesos que quieren entrar a la critical al mismo tiempo, solo aquellos procesos que no estén en su remainder serán aquellos habilitados a participar en la desición de si pueden ingresar en la crítical o no. quien puede entrar a la crítical? solo se consideran aquellos procesos que no se estén ejecutando en su remainder. esto no puede posponerse indefinidamente, en algún momento le tiene que tocar a todos. ejemplo: si alguien ya terminó de andar en la bici, tiene que ir al final de la fila para andar denuevo. solo las personas que estén en la fila podrán andar en la bici.

3- bounded waiting: debe existir un límite en el cual los procesos son habilitados a entrar en sus crítical después que hicieron el request para entrar a su crítical o después de que hicieron su entrada. ejemplo: una vez que anduviste en la bici tenes que esperar a que los otros 9 anden en ella para volver a usarla bro


ESQUEMA DE PROCESO

do {
    entry section               // permiso para entrar la critical
        critical section        // habiendo entrado en la critical
    exit section                // saliendo de la critical
        remainder section       // el remainder
} while (TRUE);
*/
