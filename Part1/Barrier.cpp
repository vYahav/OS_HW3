#include "Barrier.h"

Barrier::Barrier(unsigned int num_of_threads) : N(num_of_threads), count(0) {
	sem_init(&barrier_sem, 0, 0);
    sem_init(&flow_sem, 0, 1);
    sem_init(&count_sem, 0, 1);
}

void Barrier::wait(){
	sem_wait(&flow_sem);
    sem_wait(&barrier_sem);
    this->count++;
    if(count == this->N){
        sem_post(&barrier_sem);
    } else {
        sem_post(&flow_sem);
    }
    sem_wait(&count_sem);
    count--;
    if(count == 0){
        sem_wait(&barrier_sem);
        sem_post(&flow_sem);
    }
    sem_post(&count_sem);
    //Wait because there is no N threads waiting.
												
	//When there are N threads waiting, restart sem for the next time
	//we'll use the barrier.
}

Barrier::~Barrier(){
	sem_destroy(&barrier_sem);
    sem_destroy(&flow_sem);
    sem_destroy(&count_sem);
}
