// incluir las librerias
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>

#define BUFFER_SIZE 50                                                                 // define el tamaño del buffer

// nombres de los semaforos
#define SEM_MUTEX1 "/sem_mutex1"                                                       // Semáforo mutex para controlar acceso a buffer1
#define SEM_EMPTY1 "/sem_empty1"                                                       // Semáforo que cuenta los espacios vacíos en buffer1
#define SEM_FULL1 "/sem_full1"                                                         // Semáforo que cuenta los elementos llenos en buffer1
#define SEM_MUTEX2 "/sem_mutex2"                                                       // Semáforo mutex para controlar acceso a buffer2
#define SEM_EMPTY2 "/sem_empty2"                                                       // Semáforo que cuenta los espacios vacíos en buffer2
#define SEM_FULL2 "/sem_full2"                                                         // Semáforo que cuenta los elementos llenos en buffer2

// Estructura de datos compartidos
typedef struct {
    int buffer1[BUFFER_SIZE];                                                          // Buffer 1 para los elementos, serian 50
    int buffer2[BUFFER_SIZE];                                                          // Buffer 2 para los resultados, también 50
    int in1, out1;                                                                     // indices para insertar y extraer en buffer1
    int in2, out2;                                                                     // indices para insertar y extraer en buffer2
} shared_data_t;

// fun para el proceso P1 que genera números aleatorios e inserta en buffer1
void P1(shared_data_t *shared_data, sem_t *mutex1, sem_t *empty1, sem_t *full1) {
    int item;
    while (1) {
        item = 15;                                                           // genera un item aleatorio entre 0 y 99
        sem_wait(empty1);                                                              // revisa que haya espacio vacío en el buffer1
        sem_wait(mutex1);                                                              // adquiere el mutex para acceso exclusivo a buffer1
        shared_data->buffer1[shared_data->in1] = item;                                 // inserta el ítem en buffer1
        printf("P1 insertó el ítem en buffer1: %d\n", item);                           // mensaje para llevar un registro
        shared_data->in1 = (shared_data->in1 + 1) % BUFFER_SIZE;                       // actualiza el índice de inserción del buffer 1
        sem_post(mutex1);                                                              // libera el mutex
        sem_post(full1);                                                               // señala que hay un nuevo ítem en buffer1
        sleep(1);                                                                      // simula tiempo de procesamiento
    }
}

// fun para el proceso P2: Extrae de buffer1, eleva al cuadrado e inserta en buffer1
void P2(shared_data_t *shared_data, sem_t *mutex1, sem_t *empty1, sem_t *full1) {
    int item;
    while (1) {
        sem_wait(full1);                                                              // Espera a que haya un ítem disponible en buffer1
        sem_wait(mutex1);                                                             // Adquiere el mutex para acceso exclusivo a buffer1
        item = shared_data->buffer1[shared_data->out1];                               // Extrae el ítem de buffer1
        printf("P2 extrajo el ítem de buffer1: %d\n", item);                          // mensaje para llevar registro
        shared_data->out1 = (shared_data->out1 + 1) % BUFFER_SIZE;                    // Actualiza el índice de extracción
       // sem_post(mutex1);                                                             // Libera el mutex
       // sem_post(empty1);                                                             // señala que hay un espacio vacío en buffer1
        item = item * item;                                                           // Eleva el ítem al cuadrado
        //sem_wait(empty1);                                                             // Espera a que haya espacio vacío en buffer1 para el ítem al cuadrado
        //sem_wait(mutex1);                                                             // adquiere el mutex
        shared_data->buffer1[shared_data->in1] = item;                                // Inserta el item al cuadrado en buffer1
        printf("P2 insertó el ítem al cuadrado en buffer1: %d\n", item);              // mensaje para llevar registro
        shared_data->in1 = (shared_data->in1 + 1) % BUFFER_SIZE;                      // Actualiza el índice de inserción
        sem_post(mutex1);                                                             // Libera el mutex
        sem_post(full1);                                                              // señala que hay un nuevo ítem en buffer1
        sleep(1);                                                                     // Simula tiempo de procesamiento
    }
}

// fun para el proceso P3: Extrae dos valores de buffer1, los suma e inserta en buffer2
void P3(shared_data_t *shared_data, sem_t *mutex1, sem_t *empty1, sem_t *full1, sem_t *mutex2, sem_t *empty2, sem_t *full2) {
    int x, y, sum;
    while (1) {
        sem_wait(full1);                                                              // Espera a que haya un ítem disponible en buffer1
        sem_wait(mutex1);                                                             // espera el mutex
        x = shared_data->buffer1[shared_data->out1];                                  // Extrae el primer ítem de buffer1
        printf("P3 extrajo el primer ítem de buffer1: %d\n", x);                      // mensaje para llevar registro
        shared_data->out1 = (shared_data->out1 + 1) % BUFFER_SIZE;                    // se accede a out1 y agrega 1, sin escapar del tamaño del buffer
        sem_post(mutex1);                                                             // Libera el mutex
        sem_post(empty1);                                                             // señala que hay un espacio vacío en buffer1
        sem_wait(full1);                                                              // Espera a que haya otro ítem disponible en buffer1
        sem_wait(mutex1);                                                             // adquiere el mutex
        y = shared_data->buffer1[shared_data->out1];                                  // Extrae el segundo ítem de buffer1
        printf("P3 extrajo el segundo ítem de buffer1: %d\n", y);                     // mensaje para llevar registro
        shared_data->out1 = (shared_data->out1 + 1) % BUFFER_SIZE;                    // actualiza el indice de extraccion del buffer 1
        sem_post(mutex1);                                                             // Libera el mutex
        sem_post(empty1);                                                             // señala que hay un espacio vacío en buffer1
        sum = x + y;                                                                  // Calcula la suma de los dos ítems
        printf("P3 sumó los ítems %d y %d, resultado %d\n", x, y, sum);               // mensaje para el registro
        sem_wait(empty2);                                                             // espera a que haya un espacio vacío en el buffer 2
        sem_wait(mutex2);                                                             // adquiere el mutex del buffer 2
        shared_data->buffer2[shared_data->in2] = sum;                                 // inserta la suma en buffer2
        printf("P3 insertó la suma en buffer2: %d\n", sum);                           // mensaje para el registro
        shared_data->in2 = (shared_data->in2 + 1) % BUFFER_SIZE;                      // Actualiza el índice de inserción del buffer2
        sem_post(mutex2);                                                             // Libera el mutex
        sem_post(full2);                                                              // señala que hay un nuevo ítem en buffer2
        sleep(1);                                                                     // Simula tiempo de procesamiento
    }
}

// fun para el proceso P4 que extrae de buffer2 e imprime
void P4(shared_data_t *shared_data, sem_t *mutex2, sem_t *empty2, sem_t *full2) {
    int item;
    while (1) {
        sem_wait(full2);                                                              // espera a que haya un ítem disponible para extraer en el buffer2
        sem_wait(mutex2);                                                             // adquiere el mutex del buffer 2
        item = shared_data->buffer2[shared_data->out2];                               // Extrae el ítem de buffer2
        printf("P4 imprimió el ítem: %d\n", item);                                    // mensaje para el log
        shared_data->out2 = (shared_data->out2 + 1) % BUFFER_SIZE;                    // Actualiza el índice de extracción del buffer2
        sem_post(mutex2);                                                             // Libera el mutex
        sem_post(empty2);                                                             // señala que hay un espacio vacío en buffer2
        sleep(1);                                                                     // Simula tiempo de procesamiento
    }
}

int main() {
    // crear y mapear la memoria compartida
    int shm_fd = shm_open("/shm", O_CREAT | O_RDWR, 0666);                           // (shm_fd: variable que almacena) (shm_open: crea un objeto de memoria compartida) ("/shm": nombre del objeto de memoria compartida), (O_CREAT: flag para crear el objeto si no existe) (O_RDWR, 0666: abre el objeto con permisos 0666);
    ftruncate(shm_fd, sizeof(shared_data_t));                                        // ajusta el tamaño de la memoria compartida en la estructura para que pueda tener los buffer y todo adentro
    // mapear la memoria, sino los punteros no tendrían acceso a la memoria compartida de los buffer o los procesos
    shared_data_t *shared_data = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    shared_data->in1 = 0;                                                             // inicializar el indice 1 de insercion
    shared_data->out1 = 0;                                                            // inicializar el indice 1 de extracción
    shared_data->in2 = 0;                                                             // inicializar el indice 2 de insercion
    shared_data->out2 = 0;                                                            // inicializar el indice 1 de extracción

    sem_t *mutex1 = sem_open(SEM_MUTEX1, O_CREAT, 0644, 1);                           // inicializar el emáforo mutex para buffer1
    sem_t *empty1 = sem_open(SEM_EMPTY1, O_CREAT, 0644, BUFFER_SIZE);                 // semaforo que cuenta los espacios vacíos en buffer1
    sem_t *full1 = sem_open(SEM_FULL1, O_CREAT, 0644, 0);                             // Semáforo que cuenta los ítems llenos en buffer1

    sem_t *mutex2 = sem_open(SEM_MUTEX2, O_CREAT, 0644, 1);                           // semáforo mutex para buffer2, igual que con el 1 solo que para el 2
    sem_t *empty2 = sem_open(SEM_EMPTY2, O_CREAT, 0644, BUFFER_SIZE);                 // smáforo que cuenta los espacios vacíos en buffer2, igual que con el 1 solo que para el 2
    sem_t *full2 = sem_open(SEM_FULL2, O_CREAT, 0644, 0);                             // semaforo que cuenta los ítems llenos en buffer2, lo mismo que para el 1 solo que para el 2

    if (mutex1 == SEM_FAILED || empty1 == SEM_FAILED || full1 == SEM_FAILED ||        // chequeo si los semaforos se crearon correctamente
        mutex2 == SEM_FAILED || empty2 == SEM_FAILED || full2 == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    // los fork
    pid_t pid1 = fork();                                                              // fork del P1
    if (pid1 == 0) {                                                                  // si el pid del fork es 0
        P1(shared_data, mutex1, empty1, full1);                                       // se ejecuta P1
        exit(0);                                                                      // finaliza el fork una vez terminado P1
    }

    pid_t pid2 = fork();                                                              // fork del P2, igual que con P1
    if (pid2 == 0) {
        P2(shared_data, mutex1, empty1, full1);                                       
        exit(0);
    }

    pid_t pid3 = fork();                                                              // fork de P3
    if (pid3 == 0) {
        P3(shared_data, mutex1, empty1, full1, mutex2, empty2, full2);
        exit(0);
    }

    pid_t pid4 = fork();                                                             // fork de P4
    if (pid4 == 0) {
        P4(shared_data, mutex2, empty2, full2);
        exit(0);
    }

    // esperar a que todos los procesos hijos (los fork) terminen
    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);

    // eliminar la memoria compartida y cerrar los semáforos cuando termina el programa (si es que termina)
    shm_unlink("/shm");
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

    return 0;
}


/*
Link usados:
información de los semáforos: http://www.csc.villanova.edu/~mdamian/threads/posixsem.html
información del problema: https://medium.com/@sohamshah456/producer-consumer-programming-with-c-d0d47b8f103f
información de multiprocesos: https://mmlind.github.io/post/2020-10-05-how_to_simultaneously_write_to_shared_memory_with_multiple_processes/
informacion del problema con esta solucion: https://youtu.be/uHtzOFwgD74?si=M3tfAGIXeUQtz2-X
información sobre los fork: https://www.youtube.com/watch?v=cex9XrZCU14
https://www.youtube.com/watch?v=xVSPv-9x3gk
https://thelinuxcode.com/c_fork_system_call/

el desafio de mapear la memoria y los procesos:
https://stackoverflow.com/questions/8103560/run-two-c-programs-simultaneously-sharing-memory-how
https://stackoverflow.com/questions/1664519/creating-accessing-shared-memory-in-c
https://pubs.opengroup.org/onlinepubs/9699919799/functions/mmap.html
https://thelinuxcode.com/using_mmap_function_linux/
*/
