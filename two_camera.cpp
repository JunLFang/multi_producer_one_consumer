#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <semaphore.h>

std::queue<std::pair<int, std::string>> cameraDataQueue1;  // 相机1数据队列（时间戳，数据）
std::queue<std::pair<int, std::string>> cameraDataQueue2;  // 相机2数据队列（时间戳，数据）
std::mutex mtx;  // 互斥锁，用于保护数据队列的访问
std::condition_variable cond;  // 条件变量，用于同步线程
sem_t sem1;  // 相机1的信号量
sem_t sem2;  // 相机2的信号量
bool isRunning = true;  // 线程退出标志

const int MAX_ITEMS = 10;  // 数据队列的最大容量

void cameraProducer1() {
    int timestamp = 0;
    while (isRunning) {
        sem_wait(&sem1);  // 等待信号量
        std::unique_lock<std::mutex> lock(mtx);
        cameraDataQueue1.push(std::make_pair(timestamp, "Camera 1 Data"));
        std::cout << "Camera 1 Produced: " << timestamp << std::endl;
        ++timestamp;
        lock.unlock();
        cond.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 模拟相机产生数据的时间间隔
    }
}

void cameraProducer2() {
    int timestamp = 0;
    while (isRunning) {
        sem_wait(&sem2);  // 等待信号量
        std::unique_lock<std::mutex> lock(mtx);
        cameraDataQueue2.push(std::make_pair(timestamp, "Camera 2 Data"));
        std::cout << "Camera 2 Produced: " << timestamp << std::endl;
        ++timestamp;
        lock.unlock();
        cond.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 模拟相机产生数据的时间间隔
    }
}

void timestampComparator() {
    while (isRunning) {
        std::unique_lock<std::mutex> lock(mtx);
        cond.wait(lock, [&] { return !cameraDataQueue1.empty() && !cameraDataQueue2.empty(); });

 
        if (!cameraDataQueue1.empty() && !cameraDataQueue2.empty()) {
            auto data1 = cameraDataQueue1.front();
            auto data2 = cameraDataQueue2.front();

            if (data1.first == data2.first) {
                std::cout << "OK - Timestamp: " << data1.first << std::endl;
            } else {
                std::cout << "NG - Timestamps not matched: " << data1.first << ", " << data2.first << std::endl;
            }

            cameraDataQueue1.pop();
            cameraDataQueue2.pop();
        } else if (!cameraDataQueue1.empty()) {
            std::cout << "Only Camera 1 Data available: " << cameraDataQueue1.front().first << std::endl;
            cameraDataQueue1.pop();
        } else if (!cameraDataQueue2.empty()) {
            std::cout << "Only Camera 2 Data available: " << cameraDataQueue2.front().first << std::endl;
            cameraDataQueue2.pop();
        }
        sem_post(&sem1);  // 释放相机1的信号量
        sem_post(&sem2);  // 释放相机2的信号量
    }
}

// int main() {
//     sem_init(&sem, 0, MAX_ITEMS);  // 初始化信号量，设置初始值为MAX_ITEMS

//     // 创建生产者和消费者线程
//     std::thread producerThread1(cameraProducer1);
//     std::thread producerThread2(cameraProducer2);
//     std::thread comparatorThread(timestampComparator);

//     // 主线程等待一段时间，模拟数据产生和处理的过程
//     // std::this_thread::sleep_for(std::chrono::seconds(10));

//     // 设置退出标志为false，通知所有等待的线程退出
//     // isRunning = false;
//     // // 通知生产者线程，以防它们被信号量阻塞
//     // for (int i = 0; i < 2; ++i) {
//     //     sem_post(&sem);
//     // }

//     // cond.notify_all();

//     // 等待线程结束
//     producerThread1.join();
//     producerThread2.join();
//     comparatorThread.join();

//     sem_destroy(&sem);  // 销毁信号量

//     return 0;
// }

int main() {
    sem_init(&sem1, 0, MAX_ITEMS);  // 初始化相机1的信号量
    sem_init(&sem2, 0, MAX_ITEMS);  // 初始化相机2的信号量


    // 创建生产者和消费者线程
    std::thread producerThread1(cameraProducer1);
    std::thread producerThread2(cameraProducer2);
    std::thread comparatorThread(timestampComparator);

    // 由于生产者和消费者线程将一直运行，主线程可以在这里执行其他任务，或者简单地等待
    producerThread1.join();
    producerThread2.join();
    comparatorThread.join();

    
    sem_destroy(&sem1);  // 销毁相机1的信号量
    sem_destroy(&sem2);  // 销毁相机2的信号量

    return 0;
}