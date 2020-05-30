#include "Barrier.h"

Barrier::Barrier(unsigned int num_of_threads){
	this->num_of_threads=num_of_threads;
	sem_init(&sem,0,num_of_threads);
}

void Barrier::wait(){
	sem_wait(&sem);
}

Barrier::~Barrier(){
	sem_destroy(&sem);
}
