#ifndef BARRIER_H_
#define BARRIER_H_

#include <semaphore.h>

class Barrier {
private:
	unsigned int N;
	sem_t sem;
public:
    Barrier(unsigned int num_of_threads);
    void wait();
    ~Barrier();
    // TODO: define the member variables
	// Remember: you can only use semaphores!
};

#endif // BARRIER_H_

