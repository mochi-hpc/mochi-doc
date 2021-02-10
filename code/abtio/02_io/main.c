#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <abt-io.h>

int main(int argc, char** argv)
{
    // Argobots must be initialized, either with ABT_init
    // or with margo_init, thallium::engine, or thallium::abt.
    ABT_init(argc, argv);

    // abt_io_init takes the number of ES to create as a parameter.
    abt_io_instance_id abtio = abt_io_init(2);

    // open a file
    int fd = abt_io_open(abtio, "test.txt", O_WRONLY | O_APPEND | O_CREAT, 0600);

    // write to the file
    abt_io_pwrite(abtio, fd, "This is a test", 14, 0);

    // close the file
    abt_io_close(abtio, fd);

    // ABT-IO must be finalized before Argobots it finalized.
    abt_io_finalize(abtio);

    ABT_finalize();

    return 0;
}
