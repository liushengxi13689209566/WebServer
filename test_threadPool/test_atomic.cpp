/*************************************************************************
	> File Name: test_atomic.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年07月19日 星期四 22时58分54秒
 ************************************************************************/

#include<iostream>
#include<vector>
using namespace std;
#include<thread>
#include<vector> 
#include<atomic>
void   func( std::atomic<int>&   counter)
{
	for( int   i=0;   i<1000;   ++i )
		++counter;
}
int main()
{
	std::atomic<int>   counter ;
	std::vector<std::thread>   threads;
	for( int i=0;   i< 10 ;   ++i )
		threads.push_back( std::thread{ func, std::ref(counter)} );
	for( auto &t:threads )
		t.join();
	cout << "Result="<<counter<<std::endl;
	return 0;
}
