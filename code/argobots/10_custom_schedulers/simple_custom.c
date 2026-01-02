/*
 * Simplified custom scheduler example
 * Demonstrates minimal scheduler implementation
 */

#include <stdio.h>
#include <abt.h>

#define NUM_WORK 4

static void simple_sched_run(ABT_sched sched)
{
    ABT_pool pool;
    ABT_sched_get_pools(sched, 1, 0, &pool);
    printf("Simple scheduler starting\n");

    while (1) {
        ABT_unit unit;
        ABT_pool_pop(pool, &unit);

        if (unit != ABT_UNIT_NULL) {
            ABT_xstream_run_unit(unit, pool);
            printf("  Executed one work unit\n");
        } else {
            ABT_bool stop;
            ABT_sched_has_to_stop(sched, &stop);
            if (stop) break;
            ABT_xstream_check_events(sched);
        }
    }

    printf("Simple scheduler finished\n");
}

void work(void *arg)
{
    printf("    Work %d running\n", *(int *)arg);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_sched sched;
    ABT_sched_def sched_def = {
        .type = ABT_SCHED_TYPE_ULT,
        .init = NULL,
        .run = simple_sched_run,
        .free = NULL,
        .get_migr_pool = NULL
    };
    ABT_thread threads[NUM_WORK];
    int ids[NUM_WORK];

    ABT_init(argc, argv);

    ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC, ABT_TRUE, &pool);
    ABT_sched_create(&sched_def, 1, &pool, ABT_SCHED_CONFIG_NULL, &sched);

    ABT_xstream_self(&xstream);
    ABT_xstream_set_main_sched(xstream, sched);

    for (int i = 0; i < NUM_WORK; i++) {
        ids[i] = i;
        ABT_thread_create(pool, work, &ids[i], ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    for (int i = 0; i < NUM_WORK; i++) {
        ABT_thread_free(&threads[i]);
    }

    ABT_finalize();
    return 0;
}
