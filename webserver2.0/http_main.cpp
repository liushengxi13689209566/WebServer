/*************************************************************************
	> File Name: http_main.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月27日 星期六 10时21分56秒
 ************************************************************************/

#include "WebServer.h"
using namespace std;

int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		cout << "usage : " << basename(argv[0]) << "   ip_address    port_number   " << endl;
		return 1;
	}
	const char *ip = argv[1];
	const int port = atoi(argv[2]);
	try
	{
		WebServer my;
		my.run(ip,port);
	}
	catch (const int &line_number)
	{
		cout << line_number << endl;
		perror("　对应的系统调用失败！！！");
	}
	catch (...)
	{
		cout << " 其他错误！！！" << endl;
	}
	return 0;
}
