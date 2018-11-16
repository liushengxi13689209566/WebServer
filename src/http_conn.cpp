
#include "./http_conn.h"

void http_conn::init()
{
    http_check_state = CHECK_STATE_REQUESTLINE;
    http_keep_connect = false;
    http_method = GET;
    http_url = nullptr;
    http_version = nullptr;
    http_content_length = 0;
    http_host = nullptr;
    http_start_line = 0;
    http_checked_index = 0;
    http_end_index = 0;
    http_header_index = 0;
    memset(http_read_buf, '\0', READ_BUFFERSIZE);
    memset(http_header_buf, '\0', HEADER_BUFFERSIZE);
    memset(http_real_file, '\0', FILENAME_LEN);
}
void http_conn::modfd(int ev)
{
    epoll_event event;
    event.data.fd = http_sockfd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    Epoll_ctl(http_epollfd, EPOLL_CTL_MOD, http_sockfd, &event);
}
/*循环读取客户数据，直到无数据可读或者对方关闭连接*/
bool http_conn::read()
{
    //printf("进入　read 函数 \n");
    //printf("http_end_index ==%d\n", http_end_index);

    if (http_end_index >= READ_BUFFERSIZE)
        return false;

    int readed_bytes = 0;
    while (true)
    {
        //printf("开始　recv \n");

        readed_bytes = recv(http_sockfd, http_read_buf + http_end_index, READ_BUFFERSIZE - http_end_index, 0);
        if (readed_bytes == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return true; /*无数据可读，返回　true */
            else
                return false;
        }
        else if (readed_bytes == 0)
            return false;
        http_end_index += readed_bytes;
    }
    //printf("出 read 函数　\n");
    return true;
}

/*
触发　写事件　，进行写的操作．
先发送头部（头部信息已经构建好了［在http_header_buf中］）过去，然后调用 sendfile 将请求所对应的文件发送过去　
考虑是否保存　连接
*/
bool http_conn::send_header()
{
    //printf("进入http_conn::send_header 函数\n");

    if (Sendlen(http_sockfd, http_header_buf, strlen(http_header_buf), 0) == -1)
        return false;
    else
        return true;
}
/*通过所有检测，只是单纯的发送文件，响应请求*/
bool http_conn::write()
{
    //printf(" 进入http_conn::write函数\n");

    send_header();
    //printf("出 send_header 函数\n ");
    /*打开文件*/
    int http_real_file_fd = open(http_real_file, O_RDONLY);
    int ret = 0;
    // 双"//"都表示测试语句
    //sendfile(http_sockfd, http_real_file_fd, NULL, http_file_stat.st_size);
    ssize_t bytes_have_send = 0;

    while (bytes_have_send != http_file_stat.st_size)
    {
        ret = sendfile(http_sockfd, http_real_file_fd,
                       &bytes_have_send, http_file_stat.st_size - bytes_have_send);
        if (ret == -1)
        {
            if (errno == EAGAIN)
                continue;
            else
            {
                Close(http_real_file_fd);
                return false;
            }
        }
        else
        {
            bytes_have_send += ret;
        }
    }
    Close(http_real_file_fd);
    //printf("出 sendfile响应 函数 结束　\n ");

    if (http_keep_connect)
    {
        init();
        modfd(EPOLLIN);
        return true;
    }
    else
    {
        modfd(EPOLLIN);
        return false;
    }
}

http_conn::LINE_STATUS http_conn::parse_line()
{
    /*要分析的字节范围是(http_checked_index～(http_read_index-1))　*/
    char temp;
    for (; http_checked_index < http_end_index; ++http_checked_index)
    {
        temp = http_read_buf[http_checked_index];
        if (temp == '\r')
        {
            if ((http_checked_index + 1) == http_end_index)
                return LINE_NOT_ENOUGH;
            else if (http_read_buf[http_checked_index + 1] == '\n')
            {
                /*［xxx］＝＇＼0＇，xxx+1 */
                http_read_buf[http_checked_index++] = '\0';
                http_read_buf[http_checked_index++] = '\0';
                return LINE_OK;
            }
            else
                return LINE_BAD;
        }
        else if (temp == '\n')
        {
            if ((http_checked_index > 1) && http_read_buf[http_checked_index - 1] == '\r')
            {
                http_read_buf[http_checked_index - 1] = '\0';
                http_read_buf[http_checked_index++] = '\0';
                return LINE_OK;
            }
            else
                return LINE_BAD;
        }
    }
    return LINE_NOT_ENOUGH;
}
/*分析请求行*/
http_conn::HTTP_CODE http_conn::parse_request_line(char *line)
{
    http_url = strpbrk(line, " \t");
    if (http_url == NULL)
    {
        return BAD_REQUEST;
    }
    *http_url++ = '\0';

    char *method = line;
    if (strcasecmp(method, "GET") == 0)
    {
        http_method = GET;
    }
    else
    {
        return BAD_REQUEST;
    }
    http_url += strspn(http_url, " \t"); /*跳过分隔符*/

    http_version = strpbrk(http_url, " \t");
    if (http_version == NULL)
    {
        return BAD_REQUEST;
    }
    *http_version++ = '\0';
    http_version += strspn(http_version, " \t");

    /*这里可以处理一些http版本的一些信息*/

    /*检查url是否合法*/
    if (strncasecmp(http_url, "http://", 7) == 0)
    {
        http_url += 7;
        http_url = strchr(http_url, '/');
    }
    if (!http_url || http_url[0] != '/')
        return BAD_REQUEST;
    //printf("http_url ==%s\n", http_url);
    http_check_state = CHECK_STATE_HEADER;
    return REQUEST_NOT_ENOUGH;
}
/*一行一行的分析头部字段*/
http_conn::HTTP_CODE http_conn::parse_headers(char *line)
{
    if (line[0] == '\0')
    {
        if (http_method == HEAD)
        {
            return GET_REQUEST;
        }
        if (http_content_length != 0)
        {
            http_check_state = CHECK_STATE_CONTENT;
            return REQUEST_NOT_ENOUGH;
        }
        return GET_REQUEST;
    }
    else if (strncasecmp(line, "Connection:", 11) == 0)
    {
        line += 11;
        line += strspn(line, " \t");
        if (strcasecmp(line, "keep-alive") == 0)
        {
            http_keep_connect = true;
        }
    }
    else if (strncasecmp(line, "Content-Length:", 15) == 0)
    {
        line += 15;
        line += strspn(line, " \t");
        http_content_length = atol(line);
    }
    else if (strncasecmp(line, "Host:", 5) == 0)
    {
        line += 5;
        line += strspn(line, " \t");
        http_host = line;
    }
    else
    {
        ////printf("unknow head :%s\n", line);
    }
    return REQUEST_NOT_ENOUGH;
}
/*分析消息体*/
http_conn::HTTP_CODE http_conn::parse_content(char *line)
{
    if (http_end_index >= (http_content_length + http_checked_index))
    {
        line[http_content_length] = '\0';
        return GET_REQUEST;
    }
    return REQUEST_NOT_ENOUGH;
}
http_conn::HTTP_CODE http_conn::do_get_request()
{
    std::string tmp = http_url;
    if (tmp == "/")
        strcpy(http_real_file, doc_root);
    else
    {

        int len = strlen(doc_root);
        strncpy(http_real_file + len, http_url, FILENAME_LEN - 1 - len);
    }

    //printf("do_get_request ::http_real_file == %s\n", http_real_file);

    if (stat(http_real_file, &http_file_stat) < 0)
    {
        return NO_RESOURCE;
    }
    if (!(http_file_stat.st_mode & S_IROTH))
    {
        return FORBIDDEN_REQUEST;
    }
    if (S_ISDIR(http_file_stat.st_mode))
    {
        return BAD_REQUEST;
    }
    //printf("出 do_get_request 函数　\n");
    return FILE_REQUEST;
}
http_conn::HTTP_CODE http_conn::process_read()
{
    LINE_STATUS line_status = LINE_OK;      /*记录当前行的状态*/
    HTTP_CODE retcode = REQUEST_NOT_ENOUGH; /*记录 http 请求的处理结果*/
    char *line = nullptr;
    /*主状态机，用于从　http_read_buf 中取出所有完整的行,并对应进行分析　*/
    while (((http_check_state == CHECK_STATE_CONTENT) && (line_status == LINE_OK)) || ((line_status = parse_line()) == LINE_OK))
    {
        line = http_read_buf + http_start_line;
        http_start_line = http_checked_index; /*下一行的起始位置*/
        ////printf("get a line :%s \n", line);

        switch (http_check_state)
        {
            /*分析请求行*/
        case CHECK_STATE_REQUESTLINE:
        {
            retcode = parse_request_line(line);
            if (retcode == BAD_REQUEST)
                return BAD_REQUEST;
            break;
        }
            /*分析头部字段*/
        case CHECK_STATE_HEADER:
        {
            retcode = parse_headers(line);
            if (retcode == BAD_REQUEST)
                return BAD_REQUEST;
            else if (retcode == GET_REQUEST)
                return do_get_request();
            else if (retcode == POST_REQUEST)
                //return do_post_request();
                ;
            break;
        }
        case CHECK_STATE_CONTENT:
        {
            retcode = parse_content(line);
            if (retcode == GET_REQUEST)
            {
                return do_get_request();
            }
            line_status = LINE_NOT_ENOUGH;
            break;
        }
        default:
        {
            return SERVER_ERROR;
        }
        }
    }

    //printf("出 process_read() 函数\n");

    return REQUEST_NOT_ENOUGH;
}
void http_conn::http_close_conn()
{
    if (http_sockfd != -1)
    {
        Epoll_ctl(http_epollfd, EPOLL_CTL_DEL, http_sockfd, 0);
        Close(http_sockfd);
        http_sockfd = -1;
        //WebServer::sum_user_count--; /*必须解决的问题 */
    }
}

void http_conn::add_status_line(int status, const char *title)
{
    http_header_index = 0;
    //printf("1.addd_status_line : http_header_buf ==%s\n", http_header_buf);
    int ret = snprintf(http_header_buf, HEADER_BUFFERSIZE - 1, "HTTP/1.1 %d %s\r\n", status, title);
    http_header_index += ret;
    //printf("2.addd_status_line : http_header_buf ==%s\n", http_header_buf);
}
void http_conn::add_header()
{
    int &index = http_header_index;
    int ret = snprintf(http_header_buf + index,
                       HEADER_BUFFERSIZE - 1 - index,
                       "Connection: %s\r\n",
                       (http_keep_connect == true)
                           ? "keep-alive"
                           : "close");
    index += ret;
    /*添加文件类型*/
    std::string tmp = http_real_file;
    if (tmp.find(".html") != std::string::npos)
        ret = snprintf(http_header_buf + index,
                       HEADER_BUFFERSIZE - 1 - index,
                       "%s", "Content-Type: text/html\r\n;charset=utf-8\r\n");
    if (tmp.find(".jpg") != std::string::npos)
        ret = snprintf(http_header_buf + index,
                       HEADER_BUFFERSIZE - 1 - index,
                       "%s", "Content-Type: image/jpg\r\n;charset=utf-8\r\n");
    if (tmp.find(".png") != std::string::npos)
        ret = snprintf(http_header_buf + index,
                       HEADER_BUFFERSIZE - 1 - index,
                       "%s", "Content-Type: image/png\r\n;charset=utf-8\r\n");
    if (tmp.find(".ico") != std::string::npos)
        ret = snprintf(http_header_buf + index,
                       HEADER_BUFFERSIZE - 1 - index,
                       "%s", "Content-Type: image/x-icon\r\n;charset=utf-8\r\n");
    if (tmp.find(".mp3") != std::string::npos)
        ret = snprintf(http_header_buf + index,
                       HEADER_BUFFERSIZE - 1 - index,
                       "%s", "Content-Type: audio/mp3\r\n;charset=utf-8\r\n");
    if (tmp.find(".mp4") != std::string::npos)
        ret = snprintf(http_header_buf + index,
                       HEADER_BUFFERSIZE - 1 - index,
                       "%s", "Content-Type: video/mpeg4\r\n;charset=utf-8\r\n");

    index += ret;

    /*添加空白行*/
    ret = snprintf(http_header_buf + index,
                   HEADER_BUFFERSIZE - 1 - index,
                   "%s", "\r\n");
    index += ret;

    //printf("addd_header : http_header_buf ==%s\n", http_header_buf);
}

bool http_conn::process_write(HTTP_CODE ret)
{
    //printf("进入process_write 函数\n");

    switch (ret)
    {
    case FILE_REQUEST:
    {
        add_status_line(200, ok_200_title);
        add_header();
        /*从这里下来，就构造好了http的头部，但是因为没有修改事件类型，所以没有任何的发送的情况*/
        break;
    }
    case SERVER_ERROR:
    {
        break;
    }
    default:
    {
    }
    }

    //printf("出　process_write 函数\n");

    return true;
}
void http_conn::process()
{
    HTTP_CODE read_ret = process_read();
    if (read_ret == REQUEST_NOT_ENOUGH)
    {
        modfd(EPOLLIN);
        return;
    }
    bool write_ret = process_write(read_ret);
    if (!write_ret)
    {
        http_close_conn();
    }
    printf("修改事件类型\n");
    modfd(EPOLLOUT); /*这里触发写事件*/
}