/*
 * Reader-writer lock example
 * Multiple concurrent readers, exclusive writer
 */

#include <stdio.h>
#include <abt.h>

#define NUM_READERS 6
#define NUM_WRITERS 2

typedef struct {
    int value;
    ABT_rwlock rwlock;
} shared_data_t;

typedef struct {
    int worker_id;
    shared_data_t *shared;
} worker_arg_t;

void reader_thread(void *arg)
{
    worker_arg_t *worker = (worker_arg_t *)arg;

    ABT_rwlock_rdlock(worker->shared->rwlock);
    printf("Reader %d: reading value = %d\n",
           worker->worker_id, worker->shared->value);
    ABT_rwlock_unlock(worker->shared->rwlock);
}

void writer_thread(void *arg)
{
    worker_arg_t *worker = (worker_arg_t *)arg;

    ABT_rwlock_wrlock(worker->shared->rwlock);
    worker->shared->value++;
    printf("  Writer %d: wrote value = %d\n",
           worker->worker_id, worker->shared->value);
    ABT_rwlock_unlock(worker->shared->rwlock);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread readers[NUM_READERS];
    ABT_thread writers[NUM_WRITERS];
    worker_arg_t reader_args[NUM_READERS];
    worker_arg_t writer_args[NUM_WRITERS];
    shared_data_t shared;

    ABT_init(argc, argv);

    printf("=== Reader-Writer Lock Example ===\n");
    printf("Multiple concurrent readers, exclusive writers\n\n");

    shared.value = 0;
    ABT_rwlock_create(&shared.rwlock);

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create readers */
    for (int i = 0; i < NUM_READERS; i++) {
        reader_args[i].worker_id = i;
        reader_args[i].shared = &shared;
        ABT_thread_create(pool, reader_thread, &reader_args[i],
                          ABT_THREAD_ATTR_NULL, &readers[i]);
    }

    /* Create writers */
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_args[i].worker_id = i;
        writer_args[i].shared = &shared;
        ABT_thread_create(pool, writer_thread, &writer_args[i],
                          ABT_THREAD_ATTR_NULL, &writers[i]);
    }

    /* Wait for all */
    for (int i = 0; i < NUM_READERS; i++) {
        ABT_thread_free(&readers[i]);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        ABT_thread_free(&writers[i]);
    }

    ABT_rwlock_free(&shared.rwlock);

    printf("\nRWLock: readers can run concurrently, writers are exclusive\n");

    ABT_finalize();
    return 0;
}
