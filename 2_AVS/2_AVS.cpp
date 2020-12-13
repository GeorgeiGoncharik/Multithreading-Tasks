// 2_AVS.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
// https://ru.coursera.org/lecture/os-v-razrabotke-po/siemafory-m-iutieksy-monitory-2oCqx


#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <queue>
#include <future>
#include <atomic>

using namespace std;

class Counter
{
public:
    virtual void Inc(int& value) = 0;
};

class MutexCounter : public Counter
{
private:
    int counter;
    mutex door;

public:
    MutexCounter()
    {
        counter = 0;
    }

    void Inc(int& value) override
    {
        door.lock();
        value = counter++;
        door.unlock();
    }
};

class AtomicCounter : public Counter
{
private:
    atomic<int> atom;

public:
    AtomicCounter()
    {
        atom.store(0);
    }

    void Inc(int& value) override
    {
        value = atom.fetch_add(1);
    }
};

void counter(Counter* counter, int numTasks, int numThreads, int delayNano) {
    vector<int8_t> arr(numTasks, 0);
    vector<thread> threads(numThreads);
    mutex door;
    int shared_counter = 0;
    atomic<int> atomic_counter{ 0 };

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; i++) {
        threads[i] = thread([&] {
            int index;
            auto delay = std::chrono::nanoseconds(delayNano);
            counter->Inc(index);
            while (index < numTasks)
            {
                arr[index]++;
                counter->Inc(index);
                this_thread::sleep_for(delay);
            }});
    }

    for (int i = 0; i < numThreads; i++)
        threads[i].join();

    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

    cout << "duration: " << duration.count() << " milliseconds. " << "threads count: " << numThreads << " delay(ns): " << delayNano << ".\n";

    for (int i = 0; i < numTasks; i++) {
        if (arr[i] != 1) {
            cout << "value: " << static_cast<int8_t>(arr[i]) << "index: " << i << endl;
        }
    }
}

//Задание 2

class Queue {
public:
    // Записывает элемент в очередь.
    // Гсли очередь фиксированного размер и заполнена,
    // поток повисает внутри функции пока не освободится место
    virtual void push(uint8_t val) = 0;
    // Если очередь пуста, ждем 1 мс записи в очередь.
    // Если очередь не пуста, помещает значение головы в val,
    // удаляет голову и возвращает true.
    // Если очередь по прежнему пуста, возвращаем false
    virtual bool pop(uint8_t& val) = 0;
};

class DynamicQueue : public Queue {
private:
    queue<uint8_t> queue;
    mutex door;

public:
    void push(uint8_t val) override {
        door.lock();
        queue.push(val);
        door.unlock();
    }

    bool pop(uint8_t& val) override {
        door.lock();
        if (queue.empty()) {
            door.unlock();
            this_thread::sleep_for(chrono::milliseconds(1));
            return false;
        }
        val = queue.front();
        queue.pop();
        door.unlock();
        return true;
    }
};

class MutexQueue : public Queue {
private:
    uint8_t* queue;
    mutex door;
    int size;
    condition_variable conditionPush, conditionPop;
    atomic<int> popIndex, pushIndex, elementsCount;

public:
    MutexQueue(int size): size(size) {
        queue = new uint8_t[size];
        for (int i = 0; i < size; i++)
            queue[i] = 0;
        elementsCount = pushIndex = popIndex = 0;
    }

    void push(uint8_t val) override {
        unique_lock<mutex> lock(door);
        while (elementsCount == size)
            conditionPush.wait(lock);
        queue[pushIndex % size] = val;
        pushIndex++;
        elementsCount++;
        conditionPop.notify_one();
    }

    bool pop(uint8_t& val) override {
        unique_lock<mutex> lock(door);
        while (elementsCount == 0)
            conditionPop.wait(lock);
        val = queue[popIndex % size];
        queue[popIndex % size] = 0;
        popIndex++;
        elementsCount--;
        conditionPush.notify_one();
        return true;
    }

    ~MutexQueue() {
        delete[] queue;
    }
};

class AtomicQueue : public Queue {
private:
    atomic<uint8_t>* queue;
    int size;
    atomic<int> pushIndex, popIndex;

public:
    AtomicQueue(int size): size(size) {
        pushIndex = 0;
        popIndex = 0;
        queue = new atomic<uint8_t>[size];
        for (int i = 0; i < size; i++)
            queue[i] = 0;
    }

    void push(uint8_t val) override {
        while (true) {
            int currentPushIndex = pushIndex.load();

            uint8_t x = queue[currentPushIndex % size];
            if (x != 0) continue;

            if (pushIndex.compare_exchange_strong(currentPushIndex,
                currentPushIndex + 1)) {
                if (queue[currentPushIndex % size].compare_exchange_strong(x, val))
                    return;
            }
        }
    }

    bool pop(uint8_t& val) override {
        int currentPopIndex = popIndex.load();
        if (currentPopIndex == pushIndex.load()) {
            return false;
        }

        uint8_t x = queue[currentPopIndex % size];
        if (x == 0) return false;
        if (popIndex.compare_exchange_strong(currentPopIndex,
            currentPopIndex + 1)) {

            if (queue[currentPopIndex % size].compare_exchange_strong(x, 0)) {
                val = x;
                return true;
            }
        }
        return false;
    }

    ~AtomicQueue() {
        delete[] queue;
    }
};

void test_queue(Queue& queue, int producerNum, int consumerNum, int taskNum) {
    atomic<int> sum(0);

    auto producer = [&]() {
        for (int i = 0; i < taskNum; i++) {
            queue.push(1);
        }
    };

    auto consumer = [&]() {
        for(int i = 0; i < taskNum * producerNum / consumerNum; i++) {
            uint8_t poppedValue = 0;
            while (!queue.pop(poppedValue));
            sum.fetch_add(poppedValue);
        }
    };

    vector<thread> threads;;
    for (int i = 0; i < consumerNum; i++)
        threads.push_back(thread(consumer));
    for (int i = 0; i < producerNum; i++)
        threads.push_back(thread(producer));

    for (int i = 0; i < consumerNum + producerNum; i++)
        threads[i].join();

    cout << "consumerNum: " << consumerNum << " producerNum: " << producerNum << endl;
    if (sum == taskNum * producerNum)
        cout << "Ok!\n";
}

void measure_time(Queue& queue) {
    vector<int> consumerNums = { 1, 2, 4 };
    vector<int> produserNums = { 1, 2, 4 };
    int taskNum = 4 * 1024 * 1024;

    for (auto consumer : consumerNums) {
        for (auto producer : produserNums) {
            auto start = chrono::high_resolution_clock::now();
            test_queue(queue, producer, consumer, taskNum);
            auto end = chrono::high_resolution_clock::now();
            auto time = chrono::duration_cast<chrono::milliseconds>(end - start);
            cout << "time: " << time.count() << "ms\n\n";
        }
    }
}

int main()
{
    int ex;
    cout << "exercsize no.: ";
    cin >> ex;
    if (ex == 1) 
    {
        int delay;
        for (int i = 4; i <= 32; i *= 2) {
            delay = 0;
            cout << "delay: " << delay << endl;
            counter(new MutexCounter(),1024 * 1024, i, delay);
            counter(new AtomicCounter(),1024 * 1024, i, delay);
            delay = 10;
            cout << "delay: " << delay << endl;
            counter(new MutexCounter(), 1024 * 1024, i, delay);
            counter(new AtomicCounter(), 1024 * 1024, i, delay);
        }
    }
    else if (ex == 2)
    {
        DynamicQueue dynamic;
        measure_time(dynamic);

        vector<int> sizes = { 1, 4, 16 };
        for (auto size : sizes) {
            MutexQueue mutex(size);
            cout << "size: " << size << endl;
            measure_time(mutex);
        }

        for (auto size : sizes) {
            AtomicQueue atomic(size);
            cout << "size: " << size << endl;
            measure_time(atomic);
        }
    }
    return 0;
}



// Необходимо использовать модули стандартной библиотеки:
// thread, mutex, atomic, future, conditional_variable.
