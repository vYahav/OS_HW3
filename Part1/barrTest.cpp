
#include <iostream>
#include <unistd.h>
#include "Barrier.h"

#define BARRIER_SIZE (100)
#define LOOPS (10)
#define JOIN_THREADS for (int j = 0; j < BARRIER_SIZE; ++j) { \
pthread_join(threads[j], nullptr); \
}

#define PASSED std::cout << "\t\t==>\t\t OK" << std::endl

int loop_count = 0;
int err = -1;
pthread_mutex_t count_lock;
int barr_size;
int loops;
struct threadArgs {
    bool running;
    Barrier* b;
    int num;
    threadArgs() : running(true), b(nullptr), num(0) {}
};


void* looper(void* arg) {
    auto args = (threadArgs*)arg;
    auto bar = args->b;

    for (int i = 1; i <= args->num; ++i) {
        pthread_mutex_lock(&count_lock);
        ++loop_count;
        pthread_mutex_unlock(&count_lock);

        bar->wait();

        if (loop_count != BARRIER_SIZE*(i)) {
            pthread_mutex_lock(&count_lock);
            std::cout << loop_count << " " << i << std::endl;
            err = i;
            pthread_mutex_unlock(&count_lock);
            return nullptr;
        }

        bar->wait();
    }

    return nullptr;
}

void* deadlockTest(void* a) {
    auto args = (threadArgs*)a;
    args->b->wait();

    args->running = false;
    pthread_exit(nullptr);
}

int main() {
    Barrier barrier(BARRIER_SIZE);
    pthread_t threads[BARRIER_SIZE];
    threadArgs args[BARRIER_SIZE];
    pthread_mutex_init(&count_lock, nullptr);

    std::cout << "Test 1: sending n-1 threads to barrier ... " << std::endl;
    for (int i = 0; i < BARRIER_SIZE-1; ++i) {
        args[i].b = &barrier;
        if (pthread_create(threads+i, NULL, deadlockTest, (void*)&args[i])) {
            perror("Failed to create thread");
        }
    }

    //while (barrier.waitingThreads() < BARRIER_SIZE-1);
    for (int i =0; i<BARRIER_SIZE-1; ++i) { if (!args[i].running) err = i; }
    if (err > -1) std::cout << "\t\tFAILED (thread " << err << " got through)" << std::endl;
    else {
        PASSED;
        args[BARRIER_SIZE-1].b = &barrier;
        pthread_create(&threads[BARRIER_SIZE-1], NULL, deadlockTest, (void*)&args[BARRIER_SIZE-1]);
    }

    JOIN_THREADS

    std::cout << "Test 2: trying to reuse ... " << std::endl;
    Barrier barrier2(BARRIER_SIZE);
    err = -1;

    for (int k = 0; k < BARRIER_SIZE; ++k) {
        args[k].b = &barrier2;
        args[k].num = 2;
        if (pthread_create(threads+k, NULL, looper, (void*)&args[k])) {
            perror("Failed to create thread");
        }
    }

    JOIN_THREADS

    if (err > 0) std::cout << "\t\tFAILED" << std::endl;
    else PASSED;

    std::cout << "Test 3: using in loop ... " << std::endl;
    err = -1;
    loop_count = 0;
    Barrier barrier3(BARRIER_SIZE);

    for (int k = 0; k < BARRIER_SIZE; ++k) {
        args[k].b = &barrier3;
        args[k].num = LOOPS;
        if (pthread_create(threads+k, NULL, looper, (void*)&args[k])) {
            perror("Failed to create thread");
        }
    }

    JOIN_THREADS

    if (err > 0) std::cout << "\t\tFAILED (loop " << err << ")" << std::endl;
    else PASSED;

    std::cout << "All done, see ya" << std::endl;

}
