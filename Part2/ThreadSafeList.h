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
        List() : size(0) {
            this->head = NULL;
            if(pthread_mutex_init(&size_lock, NULL) != 0){
                cerr << "pthread_mutex_init: failed" << endl;
                exit(-1);
            }
        }

        /**
         * Destructor
         */
        ~List(){
            Node* temp = this->head;
            while(temp != NULL){
                Node* dummy = temp;
                temp = temp->next;
                delete dummy;
            }
            if(pthread_mutex_destroy(&(this->size_lock)) != 0){
                cerr << "pthread_mutex_destroy: failed" << endl;
                exit(-1);
            }
        }

        class Node {
            public:
                T data;
                Node *next;
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
                pthread_mutex_lock(&(first->lock));
                this->head = first;
                pthread_mutex_lock(&(this->size_lock));
                this->size++;
                pthread_mutex_unlock(&(this->size_lock));
                __insert_test_hook();
                pthread_mutex_unlock(&(first->lock));
                return true;
            }
            pthread_mutex_lock(&(current->lock));
			Node* next = current->next;
			if(next != NULL){
				pthread_mutex_lock(&(next->lock));
			}
            if(current->data > data){
                pthread_mutex_unlock(&(next->lock));
                //then we need to insert the node to be the first in the list
                Node* first = new Node(data, current);
                this->head = first;
                pthread_mutex_lock(&(this->size_lock));
                this->size++;
                pthread_mutex_unlock(&(this->size_lock));
                __insert_test_hook();
                pthread_mutex_unlock(&(current->lock));
                return true;
            } else if(current->data == data) {
                pthread_mutex_unlock(&(current->lock));
                pthread_mutex_unlock(&(next->lock));
                return false;
            }
			while(next != NULL && next->data < data){
                //finding the right place
                Node* tmp = current;
				current = next;
				next = current->next;
				if(next!= NULL){
                    pthread_mutex_lock(&next->lock);
                }
				pthread_mutex_unlock(&tmp->lock);
			}
            if(next != nullptr && next->data == data){
				pthread_mutex_unlock(&current->lock);
				pthread_mutex_unlock(&next->lock);
                return false;
            }
            
			Node* newNode = new Node(data,next);
		    current->next = newNode;
		    
		    pthread_mutex_lock(&(this->size_lock));
            this->size++;
            pthread_mutex_unlock(&(this->size_lock));
			// if an error occurred/detected don’t call the hook but instead
			// release any locks and return false indicating that insert was
			// failed 
			
			//std::cerr << insert: failed;
			//exit(-1);
			
			__insert_test_hook();
			
			//Unlock
			if(current!=NULL){
                pthread_mutex_unlock(&current->lock);
            }
            if(next != NULL){
                pthread_mutex_unlock(&next->lock);
            }
            return true;
        }

        /**
         * Remove the node that its data equals to @param value
         * @param value the data to lookup a node that has the same data to be removed
         * @return true if a matched node was found and removed and false otherwise
         */
        bool remove(const T& value) { 
			Node* current = this->head;
            pthread_mutex_lock(&(current->lock));
			if(nullptr == this->head){
                pthread_mutex_unlock(&(current->lock));
                return false;
            }
            //First node should be removed:
			if(current->data == value){
				if(current->next != NULL){
                    this->head = current->next;
                } else {
					this->head = NULL;
				}
				pthread_mutex_lock(&(this->size_lock));
                this->size--;
                pthread_mutex_unlock(&(this->size_lock));
				__remove_test_hook();
                pthread_mutex_unlock(&(current->lock));
                delete(current);
				return true;
			}
			//Search for the node that should be removed
			Node* next = current->next;
			//std::cout<<"LOCK: " <<next->data<<endl;
			pthread_mutex_lock(&next->lock);
            Node* tmp;
           // cout<<"--Trying to delete "<<value<<"--"<<endl;
			while(next != NULL && next->data != value){
				tmp = current;
				current = next;
				next = current->next;
				if(next != NULL){
				//	std::cout<<"LOCK: " <<next->data<<endl;
                    pthread_mutex_lock(&next->lock);
                }
               // std::cout<<"UNLOCK: " <<tmp->data<<endl;
				pthread_mutex_unlock(&tmp->lock);
				//std::cout<<"done: " <<tmp->data<<endl;
			}
			if(next == NULL){
				pthread_mutex_unlock(&current->lock);
				return false;
			}
			
			//remove the corresponding node
			//std::cout<<"LOCK: " <<next->next->data<<endl;
			//pthread_mutex_lock(&next->next->lock);
			//std::cout<<"done: " <<next->next->data<<endl;
			current->next = next->next;
			
			// if an error occurred/detected don’t call the hook but instead
			// release any locks and return false indicating that remove was
			// failed
			__remove_test_hook();
			//cout<<"------"<<endl;
			//Unlock
			pthread_mutex_unlock(&current->lock);
			pthread_mutex_unlock(&next->lock);
			//pthread_mutex_unlock(&next->next->lock);
            delete(next);
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
};

#endif //THREAD_SAFE_LIST_H_
