#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 10
#define SEM_MUTEX "/sem_mutex"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL "/sem_full"

sem_t *mutex;
sem_t *empty;
sem_t *full;

int buffer[N];
int in = 0;
int out = 0;

void produce_item(int *item);
void enter_item(int item);
void remove_item(int *item);
void consume_item(int item);
void* producer_thread(void *arg);
void* consumer_thread(void *arg);

void produce_item(int *item)
{
    static int counter = 0;
    *item = counter++;
}

void enter_item(int item)
{
    buffer[in] = item;
    in = (in + 1) % N;
}

void remove_item(int *item)
{
    *item = buffer[out];
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
