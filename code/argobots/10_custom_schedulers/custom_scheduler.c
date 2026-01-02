/*
 * Custom Scheduler Example: Simple FIFO scheduler implementation
 * Demonstrates the scheduler interface and main loop
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>

#define NUM_THREADS 4

/* Scheduler data structure */
typedef struct {
    ABT_pool pool;
    int num_executed;
} sched_data_t;

/* Scheduler init function */
static int sched_init(ABT_sched sched, ABT_sched_config config)
{
    sched_data_t *data = (sched_data_t *)malloc(sizeof(sched_data_t));
    ABT_sched_get_pools(sched, 1, 0, &data->pool);
    data->num_executed = 0;

    ABT_sched_set_data(sched, data);
    printf("Custom scheduler initialized\n");
    return ABT_SUCCESS;
}

/* Scheduler main loop */
static void sched_run(ABT_sched sched)
{
    sched_data_t *data;
    ABT_sched_get_data(sched, (void **)&data);

    while (1) {
        ABT_unit unit;
        ABT_pool_pop(data->pool, &unit);

        if (unit != ABT_UNIT_NULL) {
            /* Execute the work unit */
            ABT_xstream_run_unit(unit, data->pool);
            data->num_executed++;
        } else {
            /* No more work, check if we should finish */
            ABT_bool stop;
            ABT_sched_has_to_stop(sched, &stop);
            if (stop == ABT_TRUE) {
                break;
            }
            /* Yield to avoid busy-waiting */
            ABT_xstream_check_events(sched);
        }
    }

    printf("Custom scheduler executed %d work units\n", data->num_executed);
}

/* Scheduler finalization */
static int sched_free(ABT_sched sched)
{
    sched_data_t *data;
    ABT_sched_get_data(sched, (void **)&data);
    free(data);
    printf("Custom scheduler finalized\n");
    return ABT_SUCCESS;
}

void work_func(void *arg)
{
    int id = *(int *)arg;
    printf("  Work unit %d executing\n", id);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_sched sched;
    ABT_sched_def sched_def;
    ABT_thread threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    ABT_init(argc, argv);

    printf("=== Custom Scheduler Example ===\n\n");

    /* Create pool */
    ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC,
                          ABT_TRUE, &pool);

    /* Define custom scheduler */
    sched_def.type = ABT_SCHED_TYPE_ULT;
    sched_def.init = sched_init;
    sched_def.run = sched_run;
    sched_def.free = sched_free;
    sched_def.get_migr_pool = NULL;

    /* Create custom scheduler */
    ABT_sched_create(&sched_def, 1, &pool, ABT_SCHED_CONFIG_NULL, &sched);

    /* Set scheduler on primary xstream */
    ABT_xstream_self(&xstream);
    ABT_xstream_set_main_sched(xstream, sched);

    /* Create work units */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        ABT_thread_create(pool, work_func, &thread_ids[i],
                          ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    /* Wait for all */
    for (int i = 0; i < NUM_THREADS; i++) {
        ABT_thread_free(&threads[i]);
    }

    printf("\nCustom scheduler completed all work\n");

    ABT_finalize();
    return 0;
}
