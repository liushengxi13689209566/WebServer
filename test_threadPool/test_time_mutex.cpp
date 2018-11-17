/*************************************************************************
	> File Name: test_mutex.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年07月19日 星期四 23时13分19秒
 ************************************************************************/

#include <iostream>       // std::cout
#include <chrono>         // std::chrono::milliseconds
#include <thread>         // std::thread
#include <mutex>          // std::timed_mutex

std::timed_mutex mtx ;

void fireworks() {
  // waiting to get a lock: each thread prints "-" every 200ms:
    //表示在这一段时间范围之内线程如果没有获得锁则被阻塞住
  while (!mtx.try_lock_for(std::chrono::milliseconds(200))) {
    std::cout << "-";
  }
  // got a lock! - wait for 1s, then this thread prints "*"
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  std::cout << "*\n";
  mtx.unlock();
}

int main ()
{
  std::thread threads[10];
  // spawn 10 threads:
  for (int i=0; i<10; ++i)
    threads[i] = std::thread(fireworks);

  for (auto& th : threads) th.join();

  return 0;
}
