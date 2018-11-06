/*************************************************************************
	> File Name: stdio.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年11月06日 星期二 22时03分42秒
 ************************************************************************/
#include "../system_call.h"

int Snprintf(char *str, size_t size, const char *format, ...)
{
	int ret = 0;
	if ((ret = snprintf(str, size, format)) == -1)
		throw __LINE__;
	else 
		return (ret);
}