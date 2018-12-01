#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include <vector>
#include <queue>
#include <thread>
#include <iostream>
#include <stdexcept>
#include <condition_variable>
#include <memory> //unique_ptr
#include <functional>

const int MAX_THREADS = 10; //最大线程数目

typedef std::function<void(void)> Task;

class ThreadPool
{
  public:
    /*默认开一个线程*/
    ThreadPool(int number = 1);
    ~ThreadPool();
    /*往请求队列＜task_queue＞中添加任务<T *>*/
    bool append(Task task);

  private:
    /*工作线程需要运行的函数,不断的从任务队列中取出并执行*/
    static void *worker(void *arg);
    void run();

  private:
    std::vector<std::thread> work_threads; /*工作线程*/
    std::queue<Task> tasks_queue;          /*任务队列*/

    std::mutex queue_mutex;
    std::condition_variable condition; /*必须与unique_lock配合使用*/
    bool stop;
};

ThreadPool::ThreadPool(int number) : stop(false)
{
    if (number <= 0 || number > MAX_THREADS)
        throw std::exception();
    for (int i = 0; i < number; i++)
    {
        std::cout << "创建第" << i << "个线程 " << std::endl;
        work_threads.emplace_back(ThreadPool::worker, this);
    }
}

inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (auto &ww : work_threads)
        ww.join();
}

bool ThreadPool::append(Task task)
{
    /*操作工作队列时一定要加锁，因为他被所有线程共享*/
    queue_mutex.lock();
    tasks_queue.push(task);
    queue_mutex.unlock();
    condition.notify_one(); //线程池添加进去了任务，自然要通知等待的线程
    return true;
}
void *ThreadPool::worker(void *arg)
{
    ThreadPool *pool = (ThreadPool *)arg;
    pool->run();
    return pool;
}
void ThreadPool::run()
{
    while (!stop)
    {
        std::unique_lock<std::mutex> lk(this->queue_mutex);
        /*　unique_lock() 出作用域会自动解锁　*/
        this->condition.wait(lk, [this] { return !this->tasks_queue.empty(); });
        //如果任务队列不为空，就停下来等待唤醒
        if (this->tasks_queue.empty())
        {
            continue;
        }
        else
        {
            Task task = tasks_queue.front();
            tasks_queue.pop();
            task();
        }
    }
}
#endif