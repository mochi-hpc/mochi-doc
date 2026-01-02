/*
 * Proper error handling in Argobots
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>

void check_error(int ret, const char *msg)
{
    if (ret != ABT_SUCCESS) {
        char *err_str;
        size_t len;
        ABT_error_get_str(ret, NULL, &len);
        err_str = (char *)malloc(len);
        ABT_error_get_str(ret, err_str, &len);
        fprintf(stderr, "Error in %s: %s\n", msg, err_str);
        free(err_str);
        ABT_finalize();
        exit(1);
    }
}

int main(int argc, char **argv)
{
    int ret;
    ABT_xstream xstream;
    ABT_thread thread;
    ABT_pool pool;

    printf("=== Error Handling Example ===\n\n");

    /* Initialize with error checking */
    ret = ABT_init(argc, argv);
    check_error(ret, "ABT_init");
    printf("Argobots initialized successfully\n");

    /* Get primary execution stream */
    ret = ABT_xstream_self(&xstream);
    check_error(ret, "ABT_xstream_self");

    ret = ABT_xstream_get_main_pools(xstream, 1, &pool);
    check_error(ret, "ABT_xstream_get_main_pools");

    /* Try to create invalid thread (NULL function) */
    printf("\nAttempting invalid operation (NULL function)...\n");
    ret = ABT_thread_create(pool, NULL, NULL, ABT_THREAD_ATTR_NULL, &thread);

    if (ret != ABT_SUCCESS) {
        printf("Expected error occurred:\n");
        char *err_str;
        size_t len;
        ABT_error_get_str(ret, NULL, &len);
        err_str = (char *)malloc(len);
        ABT_error_get_str(ret, err_str, &len);
        printf("  Error code: %d\n", ret);
        printf("  Error message: %s\n", err_str);
        free(err_str);
    }

    printf("\nAlways check return values for robust error handling\n");

    ABT_finalize();
    return 0;
}
