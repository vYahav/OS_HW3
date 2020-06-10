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
            this->dummy = T();
            head = new Node(dummy);
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
                Node* dummy1 = temp;
                temp = temp->next;
                delete dummy1;
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
            Node* it = this->head, *temp = nullptr;
            pthread_mutex_lock(&(it->lock));
            while(nullptr != it->next){
                pthread_mutex_lock(&(it->next->lock));
                if(data <= it->next->data)
                    break;
                temp = it;
                it = it->next;
                pthread_mutex_unlock(&(temp->lock));
            }
            if(nullptr == it->next){
                it->next = new Node(data);
                this->incrementSize();
                __insert_test_hook();
                pthread_mutex_unlock(&(it->lock));
                return true;
            }
            if(it->next->data == data){
                pthread_mutex_unlock(&(it->next->lock));
                pthread_mutex_unlock(&(it->lock));
                return false;
            }
            
            Node* node = new Node(data, it->next);
            it->next = node;
            this->incrementSize();
            __insert_test_hook();
            pthread_mutex_unlock(&(node->next->lock));
            pthread_mutex_unlock(&(it->lock));
            return true;
        }



        /**
         * Remove the node that its data equals to @param value
         * @param value the data to lookup a node that has the same data to be removed
         * @return true if a matched node was found and removed and false otherwise
         */
        bool remove(const T& value) {
           Node* it = this->head, *temp = nullptr;
           pthread_mutex_lock(&(it->lock));
           while(nullptr != it->next){
                pthread_mutex_lock(&(it->next->lock));
                if(value == it->next->data)
                    break;
                temp = it;
                it = it->next;
                pthread_mutex_unlock(&(temp->lock));
           }
           if(nullptr == it->next){
                pthread_mutex_unlock(&(it->lock));
                return false;
           }
           Node* node = it->next;
           it->next = it->next->next;
           pthread_mutex_unlock(&(node->lock));
           delete node;
           this->decrementSize();
           __remove_test_hook();
           pthread_mutex_unlock(&(it->lock));
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
            Node* temp = head->next;
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

   /* bool isSorted(){
        pthread_mutex_t dummy_mutex;
        pthread_mutex_init(&dummy_mutex, NULL);
        pthread_mutex_lock(&dummy_mutex);
        if(!head) {
            pthread_mutex_unlock(&dummy_mutex);
            return true;
        }else{
            pthread_mutex_lock(&head->lock);
            pthread_mutex_unlock(&dummy_mutex);
        }
        Node* prev = head;
        Node* curr = head->next;
        while(curr) {
            pthread_mutex_lock(&curr->lock);
            if(prev->data >= curr->data) {
                pthread_mutex_unlock(&curr->lock);
                pthread_mutex_unlock(&prev->lock);
                return false;
            }
            pthread_mutex_unlock(&prev->lock);
            prev = curr;
            curr = curr->next;
        }
        pthread_mutex_unlock(&prev->lock);
        return true;
    }*/
    
    bool isSorted(){ //TODO REMOVE
        Node* prev = head->next;
        if(!prev)
            return true;
        pthread_mutex_lock(&prev->lock);
        Node* curr = head->next->next;
        while(curr) {
            pthread_mutex_lock(&curr->lock);
            if(prev->data >= curr->data) {
                pthread_mutex_unlock(&curr->lock);
                pthread_mutex_unlock(&prev->lock);
                return false;
            }
            pthread_mutex_unlock(&prev->lock);
            prev = curr;
            curr = curr->next;
      }
      pthread_mutex_unlock(&prev->lock);
      return true;
    }

    private:
        Node* head;
        T dummy;
        int size;
        pthread_mutex_t size_lock;
        void decrementSize(){
            pthread_mutex_lock(&(this->size_lock));
            this->size--;
            pthread_mutex_unlock(&(this->size_lock));
        }
        void incrementSize(){
            pthread_mutex_lock(&(this->size_lock));
            this->size++;
            pthread_mutex_unlock(&(this->size_lock));
        }
};

#endif //THREAD_SAFE_LIST_H_
