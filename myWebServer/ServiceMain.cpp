/*************************************************************************
	> File Name: main.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年07月23日 星期一 19时25分02秒
 ************************************************************************/


#include"http_conn.h"
using namespace std;
int main(int argc,char *argv[]){
	if (argc <= 2 ) {
		cout << "usage : " << basename(argv[0]) << "   ip_address    port_number   " << endl ;
		return 1 ;
	}
	const string  ip = argv[1] ;
	const int port = atoi(argv[2]);
	
	try{
		WebServer my(ip,port) ;
	}catch(const char *&str){
		cout << str << " 调用失败！！！"  << endl ;
	}catch(...){
		cout << " 其他错误！！！"  << endl ;
	}
	return 0;
}

