#ifndef THREAD_SAFE_LIST_H_
#define THREAD_SAFE_LIST_H_

#include <pthread.h>
#include <iostream>
#include <iomanip> // std::setw

using namespace std;

template <typename T>
class List {
    public:
        /**
         * Constructor
         */
        List() : size(0) { //TODO: add your implementation
            this->head = NULL;
            if(pthread_mutex_init(&size_lock, NULL) != 0){
                cerr << "pthread_mutex_init: failed" << endl;
                exit(-1);
            }
        }

        /**
         * Destructor
         */
        ~List(){ //TODO: add your implementation
            Node* temp = this->head;
            while(this->temp != NULL){
                Node* dummy = temp;
                temp = temp->next;
                delete dummy;
            }
            if(pthread_mutex_destroy(&(this->size_lock) != 0)){
                cerr << "pthread_mutex_destroy: failed" << endl;
                exit(-1);
            }
        }

        class Node {
            public:
                T data;
                Node *next;
                // TODO: Add your methods and data members
                pthread_mutex_t lock;
                Node(T d, Node* n = nullptr): data(d), next(n){
                    if(pthread_mutex_init(&lock, NULL) != 0){
                        cerr << "pthread_mutex_init: failed" << endl;
                        exit(-1);
                    }
                }
                ~Node(){
                    //delete data;
                    if(pthread_mutex_destroy(&lock) != 0){
                        cerr << "pthread_mutex_destroy: failed" << endl;
                        exit(-1);
                    }
                };
        };

        /**
         * Insert new node to list while keeping the list ordered in an ascending order
         * If there is already a node has the same data as @param data then return false (without adding it again)
         * @param data the new data to be added to the list
         * @return true if a new node was added and false otherwise
         */
        bool insert(const T& data) {
			Node* current = this->head;
            if(nullptr == current){
                //meaning, we have en empty list
                Node* first = new Node(data);
                this->head = first;
                pthread_mutex_lock(&(this->size_lock));
                this->size++;
                pthread_mutex_unlock(&(this->size_lock));
                return true;
            }
            pthread_mutex_lock(&(current->lock));
			Node* next = current->next;
			pthread_mutex_lock(&(next->lock));
            if(current->data > data){
                //then we need to insert the node to be the first in the list
                Node* first = new Node(data, next);
                this->head = first;
                pthread_mutex_lock(&(this->size_lock));
                this->size++;
                pthread_mutex_unlock(%(this->size_lock));
                pthread_mutex_unlock(&(current->lock));
                pthread_mutex_unlock(&(next->lock));
                return true;
            }
            
			while(next != NULL && next->data < data){
                if(current->data == data){
                    return false;
                }
				Node* tmp = current;
				current = next;
				next = current->next;
				
				if(next!= NULL) pthread_mutex_lock(&next->lock);
				pthread_mutex_unlock(&tmp->lock);
			}
			
            if(next->data == data){
				pthread_mutex_unlock(&current->lock);
				pthread_mutex_unlock(&next->lock);
                return false;
            }
            
			Node* newNode = new Node(data,next);
		    current->next = newNode;
			// if an error occurred/detected don’t call the hook but instead
			// release any locks and return false indicating that insert was
			// failed 
			/*
			std::cerr << insert: failed;
			exit(-1);
			*/
			__insert_test_hook();
			
			//Unlock
			pthread_mutex_unlock(&current->lock);
			pthread_mutex_unlock(&next->lock);
            pthread_mutex_lock(&(this->size_lock));
            this->size++;
            pthread_mutex_unlock(&(this->size_lock));
            return true;
        }

        /**
         * Remove the node that its data equals to @param value
         * @param value the data to lookup a node that has the same data to be removed
         * @return true if a matched node was found and removed and false otherwise
         */
        bool remove(const T& value) {
            if(nullptr == this->head){
                return false;
            }
			Node* current = this->head;
			//First node should be removed:
			if(current->data == value){
                pthread_mutex_lock(&(current->lock));
				if(current->next != NULL){
                    this->head = current->next;
                } else {
					this->head = NULL;
				}
				//TODO: destroy current here
                pthread_mutex_unlock(&(current->lock));
                delete(current);
                pthread_mutex_lock(&(this->size_lock));
                this->size--;
                pthread_mutex_unlock(&(this->size_lock));
				return true;
			}
			//Search for the node that should be removed
			pthread_mutex_lock(&current->lock);
			Node* next=current->next;
			pthread_mutex_lock(&next->lock);
			while(next != NULL && next->data != value){
				Node* tmp = current;
				current = next;
				next = current->next;
				if(next != NULL){
                    pthread_mutex_lock(&next->lock);
                }
				pthread_mutex_unlock(&tmp->lock);
			}
			if(next == NULL){
				pthread_mutex_unlock(&current->lock);
				return false;
			}
			
			//remove the corresponding node
			pthread_mutex_lock(&next->next->lock);
			current->next = next->next;
			
			
			
			// if an error occurred/detected don’t call the hook but instead
			// release any locks and return false indicating that remove was
			// failed
			__remove_test_hook();
			
			//Unlock
			pthread_mutex_unlock(&current->lock);
			pthread_mutex_unlock(&next->lock);
			pthread_mutex_unlock(&next->next->lock);
            pthread_mutex_lock(&(this->size_lock));
            this->size--;
            pthread_mutex_unlock(&(this->size_lock));
			return true;
        }

        /**
         * Returns the current size of the list
         * @return current size of the list
         */
        unsigned int getSize() {
			//TODO: add your implementation
            return this->size;
        }

		// Don't remove
        void print() {
            Node* temp = head;
            if (temp == NULL){
                cout << "";
            } else if (temp->next == NULL){
                cout << temp->data;
            } else {
                while (temp != NULL) {
                    cout << right << setw(3) << temp->data;
                    temp = temp->next;
                    cout << " ";
                }
            }
            cout << endl;
        }
		// Don't remove
        virtual void __insert_test_hook() {}
		// Don't remove
        virtual void __remove_test_hook() {}

    private:
        Node* head;
        int size;
        pthread_mutex_t size_lock;
    // TODO: Add your own methods and data members
};

#endif //THREAD_SAFE_LIST_H_
