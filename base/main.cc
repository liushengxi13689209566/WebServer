/*************************************************************************
	> File Name: main.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年11月30日 星期五 11时34分37秒
 ************************************************************************/

#include "./Server_init.h"
#include "./File.h"

using namespace std;
int main(void)
{
	int fd = open("/home/Shengxi-Liu/Webserver/wwwroot/php/Hello.php", O_RDONLY);
	if (fd < 0)
		printf("文件不存在\n");
}
