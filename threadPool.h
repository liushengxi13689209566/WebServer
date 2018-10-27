/*************************************************************************
	> File Name: threadPool.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月14日 星期日 21时52分53秒
 ************************************************************************/

#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <iostream>
#include <stdexcept>
#include <condition_variable>
#include <memory> /*unique_ptr*/

const int MAX_REQUESTS = 20000000; /*最大请求数目*/
const int MAX_THREADS = 8;		/*最大线程数目*/

template <typename T>
class threadPool
{
  public:
	threadPool(int number = 1); /*默认开一个线程*/
	~threadPool();
	bool append(T *request);
	static void *worker(void *arg);
	void run();

  private:
	std::vector<std::thread> work_threads;
	std::queue<T *> tasks_queue;

	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;
};

template <typename T>
threadPool<T>::threadPool(int number) : stop(nullptr)
{
	if (number <= 0 || number > MAX_THREADS)
		throw std::exception();

	for (int i = 0; i < number; i++)
	{
		// std::thread temp(worker, this);
		//不能先构造再插入
		work_threads.emplace_back(worker, this);
		work_threads.back().detach();
		// temp.detach();
	}
}

template <typename T>
threadPool<T>::~threadPool()
{
	work_threads.clear();
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (auto &ww : work_threads)
		ww.join();
}

template <typename T>
bool threadPool<T>::append(T *request)
{
	/*操作工作队列时一定要加锁，因为他被所有线程共享*/
	queue_mutex.lock();
	if (tasks_queue.size() > MAX_REQUESTS)
	{
		queue_mutex.unlock();
		return false;
	}
	tasks_queue.push(request);
	queue_mutex.unlock();
	condition.notify_one(); /*线程池添加进去了任务，自然要通知等待的线程*/
	return true;
}

template <typename T>
void *threadPool<T>::worker(void *arg)
{
	threadPool *pool = (threadPool *)arg;
	pool->run();
	return pool;
}

template <typename T>
void threadPool<T>::run()
{
	while (!stop)
	{
		std::unique_lock<std::mutex> lk(this->queue_mutex);
		this->condition.wait(lk, [this] { return !this->tasks_queue.empty(); });
		/*如果任务队列不为空，就停下来等待唤醒*/
		if (this->tasks_queue.empty())
		{
			queue_mutex.unlock();
			continue;
		}
		T *request = tasks_queue.front();
		tasks_queue.pop();
		queue_mutex.unlock();
		if (!request)
			continue;
		request->process();
	}
}
#endif
