/*************************************************************************
	> File Name: test.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月04日 星期二 18时01分19秒
 ************************************************************************/

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace std;
int main(void)
{
    char *http_url = "/php/Operation.php?a=8&b=1&c=3&d=2";

    int a, b, c, d;
    cout << *(http_url + 18) << endl;

    sscanf(http_url + 19, "a=%d&b=%d&c=%d&d=%d", &a, &b, &c, &d);

    printf("a:%d,b:%d,d:%d,d:%d \n", a, b, c, d);
}
