#include "ThreadSafeList.h"
#include <vector>
#include <algorithm>
#include <cassert>
#include <fstream>

#define TEST_NUM 100
#define MAX_ACTIONS 50
#define NUM_RANGE 100

//      Don't change those      vvv
#define INSERT ((rand() % 100) % 5 >= 3)
#define JOIN(tnum) for (int j = 0; j < tnum; ++j) { \
pthread_join(threads[j], nullptr); \
}

using std::cout;
using std::endl;

struct threadArgs {
    List<int>* list;
    int num;
    threadArgs() : list(nullptr), num(0) {}
};

bool isInList(std::vector<int>& list, int num) {
    auto it = std::find(list.begin(), list.end(), num);
    return it != list.end();
}
void removeElement(std::vector<int>& list, int num) {
    list.erase(std::remove(list.begin(), list.end(), num), list.end());
}
void print(std::vector<int>& list, ofstream& of) {
    if (list.size() == 0) of << "" << endl;
    for (auto num : list) {
        of << right << setw(3) << num << " ";
    }
    of << endl;
}

void* insert(void* args) {
    auto tArgs = (threadArgs*)args;
    tArgs->list->insert(tArgs->num);
    return nullptr;
}
void* remove(void* args) {
    auto tArgs = (threadArgs*)args;
    tArgs->list->remove(tArgs->num);
    return nullptr;
}

void falsify(bool* nums) {
    for (int i = 0; i < NUM_RANGE; ++i) {
        nums[i] = false;
    }
}
int generateNum(bool* nums) {
    int num;
    do {
        num = rand() % NUM_RANGE;
    } while (nums[num]);
    nums[num] = true;

    return num;
}

int main() {
    pthread_t threads[MAX_ACTIONS];
    threadArgs args[MAX_ACTIONS];
    bool nums[NUM_RANGE];

    int num;
    int test = 1;
    List<int> list;
    std::vector<int> listCopy;

    ofstream res;
    res.open("test_res");
    srand(time(0));

    assert(list.getSize() == 0);

    // Insertion and deletion, threadless
    bool ok = false;
    for (int k = 0; k < 100; k+=2) {
        assert(list.getSize() == (unsigned int)k/2);
        ok = list.insert(k);
        assert(ok);
        ok = list.insert(k);
        assert(!ok);
    }
    for (int k = 0; k < 100; k+=2) {
        assert(list.getSize() == (unsigned int)(50 - k/2));
        ok = list.remove(k);
        assert(ok);
        ok = list.remove(k+1);
        assert(!ok);
    }

    assert(list.getSize() == 0);

    while (test++ <= TEST_NUM) {                          // Run TEST_NUM iterations
        cout << "TEST " << test-1 << endl;                // Print to screen
        res << "TEST " << test-1 << endl;                 // Print to expected result file
        int actions = rand() % MAX_ACTIONS;               // Random number of list actions (=threads)
        actions = actions < 10 ? actions+10 : actions;    // Don't run a test that's too small
        for (int i = 0; i < actions; ++i) {
            num = generateNum(nums);                      // Generate a number for insertion/deletion
            args[i].list = &list;
            args[i].num = num;                            // Initializing args for thread funcs

            if (INSERT) {
//                cout << "Inserting " << num << endl;
                if (!isInList(listCopy, num)) listCopy.push_back(num);
                pthread_create(threads+i, nullptr, insert, (void*)&args[i]);
            } else {
//                cout << "Removing " << num << endl;
                if (isInList(listCopy, num)) removeElement(listCopy, num);
                pthread_create(threads+i, nullptr, remove, (void*)&args[i]);
            }
        }

        JOIN(actions)           // Wait for all threads to finish
        falsify(nums);

        std::sort(listCopy.begin(), listCopy.end());

        list.print();           // Print list to screen
        print(listCopy, res);   // Print expected list to test_res

        assert(listCopy.size() == list.getSize());
    }
}

