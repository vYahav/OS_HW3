#include "Barrier.h"

Barrier::Barrier(unsigned int num_of_threads){
	this->N=num_of_threads;
	sem_init(&sem,0,this->N);
}

void Barrier::wait(){
	sem_trywait(&sem);
	int threadCounter;
	//Wait because there is no N threads waiting.
	while(sem_getvalue(&sem,&threadCounter) == 0 && threadCounter>0);
												
	//When there are N threads waiting, restart sem for the next time
	//we'll use the barrier.
	sem_destroy(&sem); 
	sem_init(&sem,0,this->N);
}

Barrier::~Barrier(){
	sem_destroy(&sem);
}
