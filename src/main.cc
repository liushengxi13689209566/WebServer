/*************************************************************************
	> File Name: http_main.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月27日 星期六 10时21分56秒
 ************************************************************************/

#include "Web_server.h"
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
		ConfigServer::ServerInit Init;
		WebServer my(ip, port);
		my.run();
	}
	catch (CallFailed &failed)
	{
		std::cout << failed.ErrString << failed.LineNo << std::endl;
		perror("The reason is :");
	}
	catch (...)
	{
		perror("发生了不可见的异常\n");
	}
	return 0;
}