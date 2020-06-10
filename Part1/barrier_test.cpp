#include <iostream>
#include "../src/Barrier.h"

using namespace std;

// globals
namespace GLOBALS
{
    sem_t mutex;
    unsigned int n;
    int count;
    int *results;
    pthread_t* threads;
    Barrier *barrier;
}

void* threadfunc(void* tid)
{
    long k = (long)tid;

    sem_wait(&GLOBALS::mutex);
        ++GLOBALS::count;
    sem_post(&GLOBALS::mutex);

    GLOBALS::barrier->wait();
    GLOBALS::results[k] = GLOBALS::count;

    return nullptr;
}

bool test_single_use()
{
    GLOBALS::count = 0;
    long i;
    for (i = 0; i < GLOBALS::n; ++i)
    {
        GLOBALS::results[i] = 0;
    }
    for (i = 0; i < GLOBALS::n; ++i)
    {
        pthread_create(&GLOBALS::threads[i], nullptr, threadfunc, (void*)i);
    }
    for (i = 0; i < GLOBALS::n; ++i)
    {
        pthread_join(GLOBALS::threads[i], nullptr);
    }
    for (i = 0; i < GLOBALS::n; ++i)
    {
        /**
         * Every thread @i sets the results[i] with the number which was in the
         * variable @count right after it woke up from barrier.
         * A failure here means there was a thread that woke up from the barrier
         * before one or more of the other threads got there.
         */
        if (GLOBALS::results[i] != GLOBALS::n)
        {
            cout << "Test failed! Thread number " << i << " wrote: " << GLOBALS::results[i] << " instead of: " << GLOBALS::n << endl;
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[])
{
    // initialize globals
    do
    {
        cout << "Enter number of threads: ";
        cin >> GLOBALS::n;
        // input validation
        if (GLOBALS::n <= 0)
        {
            cout << "Number of threads should be positive!" << endl;
        }
    } while (GLOBALS::n <= 0);

    GLOBALS::results = new int[GLOBALS::n];
    GLOBALS::threads = new pthread_t[GLOBALS::n];
    sem_init(&GLOBALS::mutex, 0, 1);
    GLOBALS::barrier = new Barrier(GLOBALS::n);

    // tests
    // test the barrier for first use
    if(!test_single_use())
    {
        cout << "First test failed!" << endl;
        goto cleanup;
    }
    // test the barrier for reusability
    unsigned int iterations;
    cout << "Enter number of iterations: ";
    cin >> iterations;
    for (int i = 0; i < iterations; ++i)
    {
        if (!test_single_use())
        {
            cout << "Test failed on iteration #" << i << "!" << endl;
            cout << "Barrier is probably not reusable." << endl;
            goto cleanup;
        }
    }

    cout << "Test completed successfully :)" << endl;

    // release globals
    cleanup:
    delete GLOBALS::barrier;
    delete[] GLOBALS::threads;
    delete[] GLOBALS::results;
    sem_destroy(&GLOBALS::mutex);
}
