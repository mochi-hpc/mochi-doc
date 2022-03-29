#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <margo-timer.h>

void print_username(void* uargs) {
    const char* username = (const char*)uargs;
    fprintf(stderr, "Callback: username is %s\n", username);
}

int main(int argc, char** argv)
{
    margo_instance_id mid = margo_init("tcp",MARGO_CLIENT_MODE, 0, 0);

    margo_timer_t timer = MARGO_TIMER_NULL;

    const char* username = "Matthieu";

    margo_timer_create(mid, print_username, (void*)username, &timer);
    fprintf(stderr, "Timer created\n");

    margo_timer_start(timer, 1000);
    fprintf(stderr, "Timer submitted\n");

    margo_thread_sleep(mid, 500);
    fprintf(stderr, "This is printed before the callback\n");

    margo_thread_sleep(mid, 700);
    fprintf(stderr, "This is printed after the callback\n");

    margo_timer_start(timer, 1000);
    fprintf(stderr, "Timer resubmitted\n");

    margo_thread_sleep(mid, 500);

    margo_timer_cancel(timer);
    fprintf(stderr, "Timer was cancelled\n");

    margo_thread_sleep(mid, 700);
    fprintf(stderr, "No callback should have been printed\n");

    margo_timer_destroy(timer);

    margo_finalize(mid);

    return 0;
}
