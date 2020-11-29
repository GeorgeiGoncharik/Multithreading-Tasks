// 2_AVS.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>

using namespace std;

void mutex_counter(int numTasks, int numThreads) {
    vector<int8_t> arr(numTasks, 0);
    vector<thread> threads(numThreads);
    mutex door;
    int shared_counter = 0;

    auto start = chrono::high_resolution_clock::now();


    for (int i = 0; i < numThreads; i++) {
        threads[i] = thread([&] {
            while (true) {
                door.lock();
                if (shared_counter == numTasks) {
                    door.unlock();
                    return;
                }
                arr[shared_counter] += 1;
                shared_counter++;
                door.unlock();
            }
            });
        threads[i].join();
    }

    auto stop = chrono::high_resolution_clock::now();


    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

    cout << "[mutex]duration: " << duration.count() << " milliseconds. " << "threads count: " << numThreads << ".\n";

    for (int i = 0; i < numTasks; i++) {
        if (arr[i] != 1) {
            cout << "not 1! index: " << i << endl;
            break;
        }
    }
}

void mutex_counter_with_sleep(int numTasks, int numThreads) {
    vector<int8_t> arr(numTasks, 0);
    vector<thread> threads(numThreads);
    mutex door;
    int shared_counter = 0;

    auto start = chrono::high_resolution_clock::now();


    for (int i = 0; i < numThreads; i++) {
        threads[i] = thread([&] {
            while (true) {
                door.lock();
                if (shared_counter == numTasks) {
                    door.unlock();
                    return;
                }
                arr[shared_counter] += 1;
                shared_counter++;
                this_thread::sleep_for(chrono::nanoseconds(10));
                door.unlock();
            }
            });
        threads[i].join();
    }

    auto stop = chrono::high_resolution_clock::now();


    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

    cout << "[mutex_sleep]duration: " << duration.count() << " milliseconds. " << "threads count: " << numThreads << ".\n";

    for (int i = 0; i < numTasks; i++) {
        if (arr[i] != 1) {
            cout << "not 1! index: " << i << endl;
            break;
        }
    }
}

void atomic_counter(int numTasks, int numThreads) {
    vector<int8_t> arr(numTasks, 0);
    vector<thread> threads(numThreads);
    atomic<int> atomic_counter{ 0 };

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; i++) {
        threads[i] = thread([&] {
            while (true) {
                int value = atomic_counter.fetch_add(1);
                if (value >= numTasks) {
                    return;
                }
                arr[value] += 1;
            }
            });
        threads[i].join();
    }

    auto stop = chrono::high_resolution_clock::now();


    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

    cout << "[atomic]duration: " << duration.count() << " milliseconds. " << "threads count: " << numThreads << ".\n";

    for (int i = 0; i < numTasks; i++) {
        if (arr[i] != 1) {
            cout << "not 1! index: " << i << endl;
            break;
        }
    }
}

void atomic_counter_with_sleep(int numTasks, int numThreads) {
    vector<int8_t> arr(numTasks, 0);
    vector<thread> threads(numThreads);
    atomic<int> atomic_counter{ 0 };

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; i++) {
        threads[i] = thread([&] {
            while (true) {
                int value = atomic_counter.fetch_add(1);
                if (value >= numTasks) {
                    return;
                }
                arr[value] += 1;
                this_thread::sleep_for(chrono::nanoseconds(10));
            }
            });
        threads[i].join();
    }

    auto stop = chrono::high_resolution_clock::now();


    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

    cout << "[atomic_sleep]duration: " << duration.count() << " milliseconds. " << "threads count: " << numThreads << ".\n";

    for (int i = 0; i < numTasks; i++) {
        if (arr[i] != 1) {
            cout << "not 1! index: " << i << endl;
            break;
        }
    }
}

int main()
{
    for (int i = 4; i <= 32; i *= 2) {
        mutex_counter(1024 * 1024, i);
        atomic_counter(1024 * 1024, i);
        mutex_counter_with_sleep(1024 * 1024, i);
        atomic_counter_with_sleep(1024 * 1024, i);
    }
        
    return 0;
}



// Необходимо использовать модули стандартной библиотеки:
// thread, mutex, atomic, future, conditional_variable.
