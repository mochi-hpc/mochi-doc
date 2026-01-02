/*
 * Stencil with fine-grained synchronization using futures
 * Better parallelism than barriers by synchronizing only necessary dependencies
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>

#define NUM_CELLS 8
#define NUM_ITERATIONS 3

typedef struct {
    int cell_id;
    double *array;
    ABT_future *futures;  /* One future per cell */
} cell_arg_t;

void cell_update(void *arg)
{
    cell_arg_t *cell = (cell_arg_t *)arg;
    int id = cell->cell_id;

    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        /* Wait for dependencies: left and right neighbors from previous iteration */
        if (iter > 0) {
            if (id > 0) {
                ABT_future_wait(cell->futures[id - 1]);  /* Wait for left neighbor */
            }
            if (id < NUM_CELLS - 1) {
                ABT_future_wait(cell->futures[id + 1]);  /* Wait for right neighbor */
            }
        }

        /* Update this cell */
        int left = (id == 0) ? 0 : id - 1;
        int right = (id == NUM_CELLS - 1) ? NUM_CELLS - 1 : id + 1;
        double new_val = (cell->array[left] + cell->array[id] + cell->array[right]) / 3.0;

        cell->array[id] = new_val;
        printf("Cell %d, iteration %d: %.2f\n", id, iter, new_val);

        /* Signal that this cell is done with this iteration */
        ABT_future_set(cell->futures[id], NULL);

        /* Reset future for next iteration */
        if (iter < NUM_ITERATIONS - 1) {
            ABT_future_reset(cell->futures[id]);
        }
    }
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread threads[NUM_CELLS];
    ABT_future futures[NUM_CELLS];
    cell_arg_t cell_args[NUM_CELLS];
    double array[NUM_CELLS];

    ABT_init(argc, argv);

    printf("=== Stencil with Futures ===\n");
    printf("Fine-grained synchronization: each cell waits only for neighbors\n\n");

    /* Initialize array */
    for (int i = 0; i < NUM_CELLS; i++) {
        array[i] = i * 10.0;
    }

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create one future per cell (1 compartment each) */
    for (int i = 0; i < NUM_CELLS; i++) {
        ABT_future_create(1, NULL, &futures[i]);
    }

    /* Create cell update threads */
    for (int i = 0; i < NUM_CELLS; i++) {
        cell_args[i].cell_id = i;
        cell_args[i].array = array;
        cell_args[i].futures = futures;

        ABT_thread_create(pool, cell_update, &cell_args[i],
                          ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    /* Wait for all threads */
    for (int i = 0; i < NUM_CELLS; i++) {
        ABT_thread_free(&threads[i]);
    }

    /* Free futures */
    for (int i = 0; i < NUM_CELLS; i++) {
        ABT_future_free(&futures[i]);
    }

    printf("\nFutures allowed each cell to proceed independently,\n");
    printf("waiting only for actual dependencies (neighbors)\n");

    ABT_finalize();
    return 0;
}
