#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

std::queue<int> dataQueue;  // 共享的数据队列
std::mutex mtx;  // 互斥锁，用于保护数据队列的访问
std::condition_variable cond;  // 条件变量，用于同步生产者和消费者线程

const int MAX_ITEMS = 10;  // 数据队列的最大容量

void producer() {
    for (int i = 0; i < 20; ++i) {
        std::unique_lock<std::mutex> lock(mtx);  // 获得互斥锁
        // 如果数据队列满了，生产者等待
        cond.wait(lock, [] { return dataQueue.size() < MAX_ITEMS; });
        // 生产数据并放入队列
        dataQueue.push(i);
        std::cout << "Produced: " << i << std::endl;
        lock.unlock();  // 解锁
        cond.notify_one();  // 通知一个等待的消费者
    }
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);  // 获得互斥锁
        // 如果数据队列为空，消费者等待
        cond.wait(lock, [] { return !dataQueue.empty(); });
        // 从队列中取出数据并消费
        int data = dataQueue.front();
        dataQueue.pop();
        std::cout << "Consumed: " << data << std::endl;
        lock.unlock();  // 解锁
        cond.notify_one();  // 通知一个等待的生产者
    }
}

int main() {
    // 创建生产者和消费者线程
    std::thread producerThread(producer);
    std::thread consumerThread(consumer);

    // 等待线程结束
    producerThread.join();
    consumerThread.join();

    return 0;
}
