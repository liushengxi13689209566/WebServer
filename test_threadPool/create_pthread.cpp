/*************************************************************************
	> File Name: test_pthread.cpp
	> Author: Liu Shengxi
	> Mail: 13689209566@163.com
	> Created Time: 2018年07月19日 星期四 22时23分59秒
 ************************************************************************/

#include<iostream>
#include<vector>
using namespace std;
#include <thread> 
#include <chrono> // time 
void hello(int i, int j, int k) {
	std::cout << "Hello from thread :" << i << j << k << std::endl;
}


class thread_c
{
public:
	void operator() ()
	{
		cout << "hello from class member function" << endl ;
	}
private:
};

class Myclass{
public:
	void worker(int i,int j,int k ){
		cout << "liushengxi ~~~~~~~~~~"<< i << j << k << endl ;
	}
private:
};
int main() {
	/*1. 通过全局函数创建线程

	std::thread t1(hello, 1, 2, 3);
	t1.join() ;
	//只能调用join去结束线程，或者detach去转成后台线程执行。否则当thread被析构时，程序会被abort。*/




	/*2. 通过伪函数创建线程
	thread_c c; 
	thread t(c);
	this_thread::sleep_for(chrono::seconds(1));
	t.join() ;*/





	/*3. 通过lambda表达式创建线程
	vector<thread> threads ;
	for (int i = 0; i < 5 ; ++i)
	{
		threads.push_back(thread([](){
			cout << "通过lambda表达式创建线程 "<< this_thread::get_id() << endl ;
		}));
	}
	for(auto &i :threads ) //必须加&，目前不知道是为什么？
		i.join();*/

	/* 4. 通过成员函数创建线程
	Myclass my ;
	thread t1(&Myclass::worker,my,4,5,6);//在这里想给线程函数传几个参数就传几个参数
	t1.join();
	this_thread::sleep_for(chrono::seconds(3));*/





	return 0 ;
}
