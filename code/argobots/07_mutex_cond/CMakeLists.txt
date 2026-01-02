cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-07 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Find pthreads
find_package (Threads REQUIRED)

# Build examples
add_executable (07_abt_producer_consumer producer_consumer.c)
target_link_libraries (07_abt_producer_consumer PkgConfig::ABT)

add_executable (07_abt_shared_queue shared_queue.c)
target_link_libraries (07_abt_shared_queue PkgConfig::ABT)

add_executable (07_abt_pthread_interop pthread_interop.c)
target_link_libraries (07_abt_pthread_interop PkgConfig::ABT Threads::Threads)
