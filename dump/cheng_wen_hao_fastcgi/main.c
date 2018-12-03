#include <stdio.h>
#include <stdlib.h>
#include "fcgi.h"

int main(void)
{
    FastCgi_t *c;
    c = (FastCgi_t *)malloc(sizeof(FastCgi_t));

    FastCgi_init(c);
    setRequestId(c, 1);
    startConnect(c);
    sendStartRequestRecord(c);
    sendParams(c, "SCRIPT_FILENAME", "/home/Shengxi-Liu/1.php");
    sendParams(c, "REQUEST_METHOD", "GET");
    sendEndRequestRecord(c);
    int n = 10000;
    readFromPhp(c, &n);
    FastCgi_finit(c);
    return 0;
}
