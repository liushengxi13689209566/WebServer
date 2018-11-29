
#include"./Http_conn.h"

/*循环读取客户数据，直到无数据可读或者对方关闭连接*/
bool HttpConn::HttpRead()
{
    //printf("进入　read 函数 \n");
    //printf("http_end_index ==%d\n", http_end_index);
    if (http_end_index >= READ_BUFFERSIZE)
        return false;
    bool ret = Sockfd.RecvAll(http_read_buf, READ_BUFFERSIZE, http_end_index, SOCK_NONBLOCK);
    return ret;
    //printf("%s", http_read_buf);
    //printf("http_end_index == %d\n", http_end_index);
}
