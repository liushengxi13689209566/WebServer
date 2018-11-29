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
#include <memory> //unique_ptr

const int MAX_THREADS = 1000; //最大线程数目

template <typename T>
class threadPool
{
  public:
	/*默认开一个线程*/
	threadPool(int number = 1);
	~threadPool();
	/*往请求队列＜task_queue＞中添加任务<T *>*/
	bool append(T *request);

  private:
	/*工作线程需要运行的函数,不断的从任务队列中取出并执行*/
	static void *worker(void *arg);
	void run();

  private:
	std::vector<std::thread> work_threads; /*工作线程*/
	std::queue<T *> tasks_queue;		   /*任务队列*/
	std::mutex queue_mutex;
	std::condition_variable condition; /*必须与unique_lock配合使用*/
	bool stop;
};

template <typename T>
threadPool<T>::threadPool(int number) : stop(false)
{
	if (number <= 0 || number > MAX_THREADS)
		throw std::exception();
	for (int i = 0; i < number; i++)
	{
		std::cout << "创建第" << i << "个线程 " << std::endl;
		/*
		std::thread temp(worker, this);
		不能先构造再插入
		 */
		work_threads.emplace_back(worker, this);
	}
}

template <typename T>
inline threadPool<T>::~threadPool()
{
	/*世上最大　bug 就是因为我写的这个．shit  */
	//work_threads.clear();
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
	tasks_queue.push(request);
	queue_mutex.unlock();
	condition.notify_one(); //线程池添加进去了任务，自然要通知等待的线程
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
		/*　unique_lock() 出作用域会自动解锁　*/
		this->condition.wait(lk, [this] { return !this->tasks_queue.empty(); });
		//如果任务队列不为空，就停下来等待唤醒
		if (this->tasks_queue.empty())
		{
			continue;
		}
		else
		{
			T *request = tasks_queue.front();
			tasks_queue.pop();
			if (request)
				request->process();
		}
	}
}
#endif
