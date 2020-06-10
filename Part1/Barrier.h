#ifndef BARRIER_H_
#define BARRIER_H_

#include <semaphore.h>

class Barrier {
private:
	unsigned int N;
    unsigned int count;
	sem_t mutex;
    sem_t turnstile1;
    sem_t turnstile2;
    void step1(){
        sem_wait(&this->mutex);
        this->count++;
        if(this->count == this->N){
            for(unsigned int i = 0; i < this->N; i++){
                sem_post(&turnstile1);
            }
        }
        sem_post(&this->mutex);
        sem_wait(&turnstile1);
    }
    void step2(){
        sem_wait(&this->mutex);
        this->count--;
        if(this->count == 0){
            for(unsigned int i = 0; i < this->N; i++){
                sem_post(&turnstile2);
            }
        }
        sem_post(&this->mutex);
        sem_wait(&turnstile2);
    }
public:
    Barrier(unsigned int num_of_threads);
    void wait();
    ~Barrier();
    // TODO: define the member variables
	// Remember: you can only use semaphores!
};

#endif // BARRIER_H_

