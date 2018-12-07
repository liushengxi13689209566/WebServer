
#include <stdio.h>
#include <stdlib.h>
#include "fcgi.h"

int main()
{
    FastCgi_t *c;
    c = (FastCgi_t *)malloc(sizeof(FastCgi_t));
    FastCgi_init(c);
    setRequestId(c, 1); /*1 用来表明此消息为请求开始的第一个消息*/
    startConnect(c);

    printf("链接是成功的\n");
    int ret = 0;
    sendStartRequestRecord(c);
    ret = sendParams(c, "SCRIPT_FILENAME", "/home/Shengxi-Liu/WebServer/wwwroot/php/Operation.php");
    if (ret == 1)
        printf("成功\n");

    ret = sendParams(c, "REQUEST_METHOD", "GET");
    if (ret == 1)
        printf("成功\n");

    ret = sendParams(c, "QUERY_STRING", "a=20&b=10&c=5&d=6");
    if (ret == 1)
        printf("成功\n");

    sendEndRequestRecord(c);

    printf("end-----\n");
    readFromPhp(c);

    FastCgi_finit(c);
    return 0;
}
