/*************************************************************************
	> File Name: test_1.c
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月25日 星期四 20时40分58秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>

int main(void)
{
    char *str = "GET /    HTTP/1.0 ";

    unsigned int cnt = 0;

    char *tt = strpbrk(str, " \t"); // 寻找分隔符
    if (tt)

        printf("str == %s\n", tt);
}
