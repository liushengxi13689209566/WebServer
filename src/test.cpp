/*************************************************************************
	> File Name: test.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月04日 星期二 18时01分19秒
 ************************************************************************/

#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
using namespace std;
int main(void) {
    char *p="123456789";
    char t[1000]={0};
    for(int i=0;i< strlen(p);i++){
        t[i]=*(p+i);
    }
    printf("t == %s\n",t);
}

