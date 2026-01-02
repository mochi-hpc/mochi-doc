/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include <stdio.h>
#include <mpi.h>
#include <margo.h>
#include <flock/flock-server.h>
#include <flock/flock-bootstrap.h>

int main(int argc, char** argv)
{
    int ret;
    int rank, size;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Initialize Margo
    margo_instance_id mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    // Initialize provider args
    struct flock_provider_args args = FLOCK_PROVIDER_ARGS_INIT;
    flock_group_view_t initial_view = FLOCK_GROUP_VIEW_INITIALIZER;
    args.initial_view = &initial_view;

    // Bootstrap using MPI
    uint16_t provider_id = 42;
    ret = flock_group_view_init_from_mpi(
        mid, provider_id, MPI_COMM_WORLD, &initial_view);

    if(ret != FLOCK_SUCCESS) {
        fprintf(stderr, "Rank %d: Failed to initialize view from MPI\n", rank);
        margo_finalize(mid);
        MPI_Finalize();
        return -1;
    }

    printf("Rank %d: Bootstrapped group via MPI\n", rank);
    printf("Rank %d: Group size: %zu\n", rank, initial_view.members.size);

    // Configure with static backend
    const char* config = "{ \"group\":{ \"type\":\"static\", \"config\":{} } }";

    // Register provider
    flock_provider_t provider;
    ret = flock_provider_register(mid, provider_id, config, &args, &provider);
    assert(ret == FLOCK_SUCCESS);

    printf("Rank %d: Flock provider registered\n", rank);

    // Synchronize before finalizing
    MPI_Barrier(MPI_COMM_WORLD);

    if(rank == 0) {
        printf("All ranks initialized successfully\n");
    }

    // Wait for finalize (only rank 0 waits, others finalize immediately)
    if(rank == 0) {
        margo_wait_for_finalize(mid);
    } else {
        margo_finalize(mid);
    }

    MPI_Finalize();
    return 0;
}
