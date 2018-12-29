#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std;

condition_variable cv;
mutex mtx;
int sum = 0;
void thread_1()
{
    unique_lock<mutex> ulock(mtx);
    cv.wait(ulock); //等待条件变量被唤醒, 阻塞该线程
    cout << sum++ << endl;
    ulock.unlock();
}
void thread_2()
{
    unique_lock<mutex> ulock(mtx);
    cv.wait(ulock); //等待条件变量被唤醒, 阻塞该线程
    cout << sum++ << endl;
    ulock.unlock();
}
int main(int argc, char **argv)
{
    thread t1(thread_1);
    thread t2(thread_2);

    this_thread::sleep_for(chrono::seconds(5));
    cv.notify_one();
    t1.join();
    t2.join();
    return 0;
}
