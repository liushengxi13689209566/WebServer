/*************************************************************************
	> File Name: test_mutex.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年07月19日 星期四 23时13分19秒
 ************************************************************************/

#include <iostream>       
#include <chrono>        
#include <thread>         
#include <mutex>          
using namespace std;
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include <chrono>
volatile int counter(0); // non-atomic counter
std::mutex mtx ;            // locks access to counter

void attempt_10k_increases() {
    for (int i=0; i<100 ; ++i) {
        if ( mtx.try_lock()) {   // only increase if currently not locked:
            ++counter;
            mtx.unlock();
        }
    }
}
int main (int argc, const char* argv[]) {
    std::thread threads[10] ;
    for (int i=0; i<10; ++i)
        threads[i] = std::thread(attempt_10k_increases);

    this_thread::sleep_for(chrono::seconds(4));
    for (auto& th : threads) 
    	th.join();
    
    std::cout << counter << " successful increases of the counter.\n";

    return 0;
}
