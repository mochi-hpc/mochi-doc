/*
 * Producer-consumer pattern with mutex and condition variable
 */

#include <stdio.h>
#include <abt.h>

#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 5
#define BUFFER_SIZE 3

typedef struct {
    int buffer[BUFFER_SIZE];
    int count;
    int in;
    int out;
    ABT_mutex mutex;
    ABT_cond not_full;
    ABT_cond not_empty;
} shared_buffer_t;

typedef struct {
    int id;
    shared_buffer_t *shared;
} worker_arg_t;

void producer(void *arg)
{
    worker_arg_t *worker = (worker_arg_t *)arg;
    shared_buffer_t *buf = worker->shared;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = worker->id * 100 + i;

        ABT_mutex_lock(buf->mutex);

        /* Wait while buffer is full */
        while (buf->count == BUFFER_SIZE) {
            printf("Producer %d: buffer full, waiting...\n", worker->id);
            ABT_cond_wait(buf->not_full, buf->mutex);
        }

        /* Add item to buffer */
        buf->buffer[buf->in] = item;
        buf->in = (buf->in + 1) % BUFFER_SIZE;
        buf->count++;
        printf("Producer %d: produced item %d (buffer: %d/%d)\n",
               worker->id, item, buf->count, BUFFER_SIZE);

        /* Signal consumers */
        ABT_cond_signal(buf->not_empty);

        ABT_mutex_unlock(buf->mutex);
    }
}

void consumer(void *arg)
{
    worker_arg_t *worker = (worker_arg_t *)arg;
    shared_buffer_t *buf = worker->shared;
    int num_items = (NUM_PRODUCERS * ITEMS_PER_PRODUCER) / NUM_CONSUMERS;

    for (int i = 0; i < num_items; i++) {
        ABT_mutex_lock(buf->mutex);

        /* Wait while buffer is empty */
        while (buf->count == 0) {
            printf("Consumer %d: buffer empty, waiting...\n", worker->id);
            ABT_cond_wait(buf->not_empty, buf->mutex);
        }

        /* Remove item from buffer */
        int item = buf->buffer[buf->out];
        buf->out = (buf->out + 1) % BUFFER_SIZE;
        buf->count--;
        printf("  Consumer %d: consumed item %d (buffer: %d/%d)\n",
               worker->id, item, buf->count, BUFFER_SIZE);

        /* Signal producers */
        ABT_cond_signal(buf->not_full);

        ABT_mutex_unlock(buf->mutex);
    }
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread producers[NUM_PRODUCERS];
    ABT_thread consumers[NUM_CONSUMERS];
    worker_arg_t prod_args[NUM_PRODUCERS];
    worker_arg_t cons_args[NUM_CONSUMERS];
    shared_buffer_t shared;

    ABT_init(argc, argv);

    printf("=== Producer-Consumer Pattern ===\n");
    printf("Producers: %d, Consumers: %d, Buffer: %d\n\n",
           NUM_PRODUCERS, NUM_CONSUMERS, BUFFER_SIZE);

    /* Initialize shared buffer */
    shared.count = 0;
    shared.in = 0;
    shared.out = 0;
    ABT_mutex_create(&shared.mutex);
    ABT_cond_create(&shared.not_full);
    ABT_cond_create(&shared.not_empty);

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create producers */
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        prod_args[i].id = i;
        prod_args[i].shared = &shared;
        ABT_thread_create(pool, producer, &prod_args[i],
                          ABT_THREAD_ATTR_NULL, &producers[i]);
    }

    /* Create consumers */
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        cons_args[i].id = i;
        cons_args[i].shared = &shared;
        ABT_thread_create(pool, consumer, &cons_args[i],
                          ABT_THREAD_ATTR_NULL, &consumers[i]);
    }

    /* Wait for all */
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        ABT_thread_free(&producers[i]);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        ABT_thread_free(&consumers[i]);
    }

    /* Cleanup */
    ABT_cond_free(&shared.not_empty);
    ABT_cond_free(&shared.not_full);
    ABT_mutex_free(&shared.mutex);

    printf("\nAll items produced and consumed successfully\n");

    ABT_finalize();
    return 0;
}
