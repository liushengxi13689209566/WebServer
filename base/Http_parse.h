/*************************************************************************
	> File Name: http_parse.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年11月20日 星期二 13时50分06秒
 ************************************************************************/

#ifndef _HTTP_PARSE_HPP
#define _HTTP_PARSE_HPP
/* http 请求方法，目前只支持 get */
#include "../base/File.h"
#include "../base/base.hpp"

enum METHOD
{
    GET,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATCH
};
/*解析客户的请求时，主状态机所处的几种状态*/
enum CHECK_STATE
{
    CHECK_REQUESTLINE,
    CHECK_HEADER,
    CHECK_CONTENT
};
/*服务器处理　http 请求的结果 */
enum HTTP_CODE
{
    RE_NOT_ENOUGH,    /*请求不完整，继续读取客户端*/
    ENOUGH_RE,        /*读取到一个完整请求*/
    BAD_RE,           /*请求有语法错误*/
    FORBIDDEN_RE,     /*客户对该资源无权限*/
    NO_RESOURCE,      /*无此资源，发送404页面*/
    FILE_RE,          /*文件请求*/
    SERVER_ERROR,     /*服务器出错*/
    CLOSED_CONNECTION /*客户端关闭了连接*/
};
/*行的读取状态：有可能一行数据都没有一次性传输完毕，需要继续进行传输*/
enum LINE_STATUS
{
    LINE_OK,
    LINE_BAD,
    LINE_NOT_ENOUGH
};
/*文件名最大长度*/
static const int FILENAME_LEN = 200;

class HttpParse
{
  public:
    /*sizeof(会退化成指针)，strlen(会取出实际字节数目)  */
    HttpParse() : file() { Init(); }
    HttpParse(const HttpParse &) = delete;
    HttpParse &operator=(const HttpParse &) = delete;
    ~HttpParse() {}

    inline void Init()
    {
        retcode = RE_NOT_ENOUGH;
        buffer = NULL;
        http_read_index = 0;
        http_checked_index = 0;
        http_start_line = 0;
        http_check_state = CHECK_REQUESTLINE;
        http_method = GET;
        memset(http_real_file, '\0', FILENAME_LEN);
        http_url = NULL;
        http_version = NULL;
        http_host = NULL;
        http_content_length = 0;
        http_keep_connect = false;
        query_string = NULL;
    }
    /*正式读取一个未知　http　报文的接口，所以必须在这里进行初始化*/
    HTTP_CODE HttpDataRead(char *http_read_buf, int &temp)
    {
        Init();

        buffer = http_read_buf;
        http_read_index = temp;

        printf("http_read_index== %d\n", http_read_index);

        LINE_STATUS line_status = LINE_OK; /*记录当前行的状态*/
        retcode = RE_NOT_ENOUGH;           /*记录 http 请求的处理结果*/
        char *line = NULL;

        /*主状态机，用于从　buffer 中取出所有完整的行,并对应进行分析　*/
        while (((http_check_state == CHECK_CONTENT) && (line_status == LINE_OK)) || ((line_status = ParseLine()) == LINE_OK))
        {
            line = buffer + http_start_line;
            http_start_line = http_checked_index; /*下一行的起始位置*/

            printf("get a line :%s \n", line);
            switch (http_check_state)
            {
                /*分析请求行*/
            case CHECK_REQUESTLINE:
            {
                retcode = ParseRequestLine(line);
                if (retcode == BAD_RE)
                    return BAD_RE;
                break;
            }
                /*分析头部字段*/
            case CHECK_HEADER:
            {
                retcode = ParseHeaders(line);
                if (retcode == BAD_RE)
                    return BAD_RE;
                else if (retcode == ENOUGH_RE)
                    return DoRequest();
                break;
            }
            case CHECK_CONTENT:
            {
                retcode = ParseContent(line);
                if (retcode == ENOUGH_RE)
                {
                    return DoRequest();
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
        return RE_NOT_ENOUGH;
    }
    inline char *GetFileName()
    {
        return http_real_file;
    }
    inline METHOD GetMethod()
    {
        return http_method;
    }
    inline int GetContentLength()
    {
        return http_content_length;
    }
    inline bool IsKeep()
    {
        return http_keep_connect;
    }
    inline bool IsDynamic()
    {
        std::string tmp = http_url;
        if (tmp.find("?") != std::string::npos)
            return true;
        else
            return false;
    }
    inline bool IsPhp()
    {
        std::string tmp = http_real_file;
        std::string file_type;
        auto idx = tmp.rfind('.');

        if (idx != std::string::npos)
        {
            file_type = tmp.substr(idx);
        }
        return file_type == ".php";
    }
    inline HTTP_CODE CheckFile()
    {
        File file(http_real_file, O_RDONLY);
        /*有三种情况，1（html）　2　(php)　3　(url中带有参数)*/
        // 在这里进行处理
        if (file.GetFileFd() < 0)
        {
            return NO_RESOURCE;
        }
        else if (file.IsForbid())
        {
            return FORBIDDEN_RE;
        }
        else if (file.IsDir())
        {
            return BAD_RE;
        }
        //printf("出 do_get_request 函数　\n");
        else
        {
            return FILE_RE;
        }
    }
    inline char *GetQueryString()
    {
        return query_string;
    }

  private:
    HTTP_CODE DoRequest()
    {
        printf("http_url ==  %s\n", http_url);
        int len = strlen(doc_root);
        strncpy(http_real_file, doc_root, len);

        if (IsDynamic())
        {
            printf("{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{\n");
            // 含有 ？ 号
            return DoDynamicRequest();
        }
        else /*get 没有参数,构建文件，检查文件*/
        {
            if (!strcmp(http_url, "/"))
            {
                strncpy(http_real_file + len, "/html/index.html", FILENAME_LEN - 1 - len);
            }
            else
            {
                /*单纯的请求　php 文件就在这里处理了 */
                strncpy(http_real_file + len, http_url, FILENAME_LEN - 1 - len);
            }
            printf("http_real_file==%s\n", http_real_file);

            return CheckFile();
        }
    }
    HTTP_CODE DoDynamicRequest()
    {
        /*解析参数和文件名 http://127.0.0.1:10000/php/Operation.php?a=8&b=1&c=3&d=2*/
        int i, k;
        for (i = 0, k = strlen(http_real_file); i < strlen(http_url); i++, k++)
        {
            if (*(http_url + i) == '?')
                break;
            else
                http_real_file[k] = *(http_url + i);
        }
        http_real_file[k] = '\0';
        printf("http_real =%s\n", http_real_file);

        query_string = http_url + i + 1;
        printf("query_string =%s\n", query_string);

        return CheckFile();
    }
    /*process_read 所使用的函数*/
    LINE_STATUS ParseLine()
    {
        /*要分析的字节范围是(http_checked_index～(http_read_index-1))　*/
        char temp;
        for (; http_checked_index < http_read_index; ++http_checked_index)
        {
            temp = buffer[http_checked_index];
            if (temp == '\r')
            {
                if ((http_checked_index + 1) == http_read_index)
                {
                    return LINE_NOT_ENOUGH;
                }
                else if (buffer[http_checked_index + 1] == '\n')
                {
                    buffer[http_checked_index++] = '\0';
                    buffer[http_checked_index++] = '\0';
                    return LINE_OK;
                }

                return LINE_BAD;
            }
            else if (temp == '\n')
            {
                if ((http_checked_index > 1) && (buffer[http_checked_index - 1] == '\r'))
                {
                    buffer[http_checked_index - 1] = '\0';
                    buffer[http_checked_index++] = '\0';
                    return LINE_OK;
                }
                return LINE_BAD;
            }
        }
        return LINE_NOT_ENOUGH;
    }

    HTTP_CODE ParseRequestLine(char *line)
    {
        http_url = strpbrk(line, " \t");
        if (!http_url)
        {
            return BAD_RE;
        }
        *http_url++ = '\0';

        char *method = line;
        if (strcasecmp(method, "GET") == 0)
            http_method = GET;
        else if (strcasecmp(method, "POST") == 0)
            http_method = POST;
        else
            return BAD_RE;

        http_url += strspn(http_url, " \t"); /*跳过分隔符*/
        http_version = strpbrk(http_url, " \t");
        if (!http_version)
        {
            return BAD_RE;
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
            return BAD_RE;
        //printf("http_url ==%s\n", http_url);
        http_check_state = CHECK_HEADER;
        return RE_NOT_ENOUGH;
    }
    /*一行一行的分析头部字段*/
    HTTP_CODE ParseHeaders(char *line)
    {
        if (line[0] == '\0')
        {
            if (http_method == HEAD)
            {
                return ENOUGH_RE;
            }
            if (http_content_length != 0)
            {
                http_check_state = CHECK_CONTENT;
                return RE_NOT_ENOUGH;
            }
            else
                return ENOUGH_RE;
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
        return RE_NOT_ENOUGH;
    }
    HTTP_CODE ParseContent(char *line)
    {
        if (http_read_index >= (http_content_length + http_checked_index))
        {
            line[http_content_length] = '\0';
            query_string = line; /*将查询字符串指向消息内容*/
            return ENOUGH_RE;
        }
        return RE_NOT_ENOUGH;
    }

  private:
    File file;
    /*返回码*/
    HTTP_CODE retcode = RE_NOT_ENOUGH;
    /*指向http报文地址*/
    char *buffer = NULL;
    /*http报文尾部下一个字节*/
    int http_read_index;
    /*正在分析的字符在读缓冲区中的位置*/
    int http_checked_index = 0;
    /*正在解析的行的起始位置*/
    int http_start_line = 0;
    CHECK_STATE http_check_state = CHECK_REQUESTLINE;
    /*请求的方法*/
    METHOD http_method = GET;
    char *query_string = NULL;
    /*客户请求的目标文件的完整的路径，＝　root+url */
    char http_real_file[FILENAME_LEN] = {0};
    /*url*/
    char *http_url = NULL;
    /*版本*/
    char *http_version = NULL;
    /*主机名*/
    char *http_host = NULL;
    /*http请求的消息体的长度*/
    int http_content_length = 0;
    /*是否需要保持连接*/
    bool http_keep_connect = false;
};
#endif
