#include "Barrier.h"

Barrier::Barrier(unsigned int num_of_threads) : N(num_of_threads), count(0) {
	sem_init(&mutex, 0, 1);
    sem_init(&turnstile1, 0, 0);
    sem_init(&turnstile2, 0, 0);
}

void Barrier::wait(){
    this->step1();
    this->step2();
}

Barrier::~Barrier(){
	sem_destroy(&mutex);
    sem_destroy(&turnstile1);
    sem_destroy(&turnstile2);
}
