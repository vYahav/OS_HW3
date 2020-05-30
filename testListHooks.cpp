#include "ThreadSafeList.h"
#include <iostream>
#include <assert.h>
using namespace std;

template <typename T>
class MyList : public List<T> {
 public:
  void __insert_test_hook() override {
    if (!hook_enabled)
      return;
    this->remove(val_to_remove);
  }
  void activateHook(int val) {
    hook_enabled = true;
    val_to_remove = val;
  }
  void deactivateHook() {
    hook_enabled = false;
  }
  void setValueToRemove(int val) {
    val_to_remove = val;
  }
 private:
  bool hook_enabled = false;
  int val_to_remove = 0;
};

int main() {
  List<int>* l = new MyList<int>();
  for (int i=0; i<20; i+=2) {
    l->insert(i);
  }
  l->print(); // should print: 0,2,4,6,8,10,12,14,16,18
  assert(l->getSize() == 10);

  // the node to be removed should be a pred of the one being inserted
  // because hand locks are already acquired on the inserted node
  ((MyList<int>*)l)->activateHook(10);
  l->insert(17);

  ((MyList<int>*)l)->setValueToRemove(6);
  l->insert(11);

  assert(l->getSize() == 10);
  l->print(); // should print: 0,2,4,8,11,12,14,16,17,18
  return 0;
}
