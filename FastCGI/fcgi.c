#include "fastcgi.h"
#include "fcgi.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>

static const int PARAMS_BUFF_LEN = 1024;  //环境参数buffer的大小
static const int CONTENT_BUFF_LEN = 1024; //内容buffer的大小


static char *getHtmlFromContent(FastCgi_t *c, char *content);

void FastCgi_init(FastCgi_t *c)
{
    c->sockfd_ = 0;
    c->flag_ = 0;
    c->requestId_ = 0;
}

void FastCgi_finit(FastCgi_t *c)
{
    close(c->sockfd_);
}

void setRequestId(FastCgi_t *c, int requestId)
{
    c->requestId_ = requestId;
}

FCGI_Header makeHeader(int type, int requestId,
                       int contentLength, int paddingLength)
{
    FCGI_Header header;

    header.version = FCGI_VERSION_1;

    header.type = (unsigned char)type;

    /* 两个字段保存请求ID */
    header.requestIdB1 = (unsigned char)((requestId >> 8) & 0xff);
    header.requestIdB0 = (unsigned char)(requestId & 0xff);

    /* 两个字段保存内容长度 */
    header.contentLengthB1 = (unsigned char)((contentLength >> 8) & 0xff);
    header.contentLengthB0 = (unsigned char)(contentLength & 0xff);

    /* 填充字节的长度 */
    header.paddingLength = (unsigned char)paddingLength;

    /* 保存字节赋为 0 */
    header.reserved = 0;

    return header;
}

FCGI_BeginRequestBody makeBeginRequestBody(int role, int keepConnection)
{
    FCGI_BeginRequestBody body;

    /* 两个字节保存期望 php-fpm 扮演的角色 */
    body.roleB1 = (unsigned char)((role >> 8) & 0xff);
    body.roleB0 = (unsigned char)(role & 0xff);

    /* 大于0长连接，否则短连接 */
    body.flags = (unsigned char)((keepConnection) ? FCGI_KEEP_CONN : 0);

    bzero(&body.reserved, sizeof(body.reserved));

    return body;
}

int makeNameValueBody(char *name, int nameLen,
                      char *value, int valueLen,
                      unsigned char *bodyBuffPtr, int *bodyLenPtr)
{
    /* 记录 body 的开始位置 */
    unsigned char *startBodyBuffPtr = bodyBuffPtr;

    /* 如果 nameLen 小于128字节 */
    if (nameLen < 128)
    {
        *bodyBuffPtr++ = (unsigned char)nameLen; //nameLen用1个字节保存
    }
    else
    {
        /* nameLen 用 4 个字节保存 */
        *bodyBuffPtr++ = (unsigned char)((nameLen >> 24) | 0x80);
        *bodyBuffPtr++ = (unsigned char)(nameLen >> 16);
        *bodyBuffPtr++ = (unsigned char)(nameLen >> 8);
        *bodyBuffPtr++ = (unsigned char)nameLen;
    }

    /* valueLen 小于 128 就用一个字节保存 */
    if (valueLen < 128)
    {
        *bodyBuffPtr++ = (unsigned char)valueLen;
    }
    else
    {
        /* valueLen 用 4 个字节保存 */
        *bodyBuffPtr++ = (unsigned char)((valueLen >> 24) | 0x80);
        *bodyBuffPtr++ = (unsigned char)(valueLen >> 16);
        *bodyBuffPtr++ = (unsigned char)(valueLen >> 8);
        *bodyBuffPtr++ = (unsigned char)valueLen;
    }

    /* 将 name 中的字节逐一加入body中的buffer中 */
    for (int i = 0; i < strlen(name); i++)
    {
        *bodyBuffPtr++ = name[i];
    }

    /* 将 value 中的值逐一加入body中的buffer中 */
    for (int i = 0; i < strlen(value); i++)
    {
        *bodyBuffPtr++ = value[i];
    }

    /* 计算出 body 的长度 */
    *bodyLenPtr = bodyBuffPtr - startBodyBuffPtr;
    return 1;
}

/*
 * 如果有配置文件的话，可以将一些信息，比如IP 从配置文件里读出来
 *
char *getIpFromConf()
{
    return getMessageFromFile("IP");
}
*/

void startConnect(FastCgi_t *c)
{
    int rc;
    int sockfd;
    struct sockaddr_in server_address;

    /* 固定 */
    char ip[] = "127.0.0.1";

    /* 获取配置文件中的ip地址 */
    //ip = getIpFromConf();

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd > 0);

    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(9000);

    rc = connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
    assert(rc >= 0);

    c->sockfd_ = sockfd;
}
int sendStartRequestRecord(FastCgi_t *c)
{
    int rc;
    FCGI_BeginRequestRecord beginRecord;

    beginRecord.header = makeHeader(FCGI_BEGIN_REQUEST, c->requestId_, sizeof(beginRecord.body), 0);
    beginRecord.body = makeBeginRequestBody(FCGI_RESPONDER, 0);

    rc = write(c->sockfd_, (char *)&beginRecord, sizeof(beginRecord));
    assert(rc == sizeof(beginRecord));

    return 1;
}

int sendParams(FastCgi_t *c, char *name, char *value)
{
    int rc;

    unsigned char bodyBuff[PARAMS_BUFF_LEN];

    bzero(bodyBuff, sizeof(bodyBuff));

    /* 保存 body 的长度 */
    int bodyLen;

    /* 生成 PARAMS 参数内容的 body */
    makeNameValueBody(name, strlen(name), value, strlen(value), bodyBuff, &bodyLen);

    FCGI_Header nameValueHeader;
    nameValueHeader = makeHeader(FCGI_PARAMS, c->requestId_, bodyLen, 0);

    int nameValueRecordLen = bodyLen + FCGI_HEADER_LEN;
    char nameValueRecord[nameValueRecordLen];

    /* 将头和body拷贝到一块buffer 中只需调用一次write */
    memcpy(nameValueRecord, (char *)&nameValueHeader, FCGI_HEADER_LEN);
    memcpy(nameValueRecord + FCGI_HEADER_LEN, bodyBuff, bodyLen);

    rc = write(c->sockfd_, nameValueRecord, nameValueRecordLen);
    assert(rc == nameValueRecordLen);

    return 1;
}

int sendEndRequestRecord(FastCgi_t *c)
{
    int rc;

    FCGI_Header endHeader;
    endHeader = makeHeader(FCGI_PARAMS, c->requestId_, 0, 0);

    rc = write(c->sockfd_, (char *)&endHeader, FCGI_HEADER_LEN);
    assert(rc == FCGI_HEADER_LEN);

    return 1;
}

char *readFromPhp(FastCgi_t *c, int *len)
{
    FCGI_Header responderHeader;
    // char content[CONTENT_BUFF_LEN];＆＆＆＆＆＆＆＆＆＆＆＆

    int contentLen;
    char tmp[512]; //用来暂存padding字节
    int ret;
    char *result;
    *len = 0;
    /* 先将头部 8 个字节读出来 */
    /*while*/ if (read(c->sockfd_, &responderHeader, FCGI_HEADER_LEN) > 0)
    {
        if (responderHeader.type == FCGI_STDOUT)
        {
            /* 获取内容长度 */
            contentLen = (responderHeader.contentLengthB1 << 8) + (responderHeader.contentLengthB0);
            (*len) = contentLen;
            if (contentLen == 0)
            {
                return NULL;
            }
            // fprintf(stderr, "BEFORE MALLOC, contentlen = %d\n", contentLen);
            char *content = (char *)malloc(contentLen); //＆＆＆＆＆＆

            // printf("contentlen is :%d\n", contentLen); //////////
            // bzero(content, CONTENT_BUFF_LEN);

            /* 读取获取内容 */
            // ret = read(c->sockfd_, content, contentLen);
            int readed = 0;
            while (readed != contentLen)
            {
                int ret = read(c->sockfd_, content + readed, contentLen - readed);
                if (ret == -1)
                {
                    //因为是非阻塞，所以需要过一会儿再重新读文件
                    //Resource temporarily unavailable
                    if (errno == EAGAIN)
                    {
                        continue;
                    }
                    perror("SENDFILE IN readphp ###");
                    break;
                }
                readed += ret;
            }
            // perror("read");
            // printf("已经发送完成!\n");
            // printf("%s", content);
            assert(readed == contentLen);

            // result = getHtmlFromContent(c, content);
            result = content;//如果没有返回原始指针，就不能随意的free
            /**************
             * ^        原来
             * |
             *  
             *      ^
             *      |    移动之后 ，此时free，会发生段错误
             * 
            */
            // free(content);
            // printf("\nafter gethtmlfrom content\n"); ///////////////////

            /* 跳过填充部分 */
            if (responderHeader.paddingLength > 0)
            {
                ret = read(c->sockfd_, tmp, responderHeader.paddingLength);
                assert(ret == responderHeader.paddingLength);
            }
        } //end of type FCGI_STDOUT
        else if (responderHeader.type == FCGI_STDERR)
        {
            contentLen = (responderHeader.contentLengthB1 << 8) + (responderHeader.contentLengthB0);
            // bzero(content, CONTENT_BUFF_LEN);&&&&&&
            *len = contentLen;
            char *content = (char *)malloc(contentLen); //＆＆＆＆＆＆
            ret = read(c->sockfd_, content, contentLen);
            assert(ret == contentLen);

            // fprintf(stdout, "error:%s\n", content);

            /* 跳过填充部分 */
            if (responderHeader.paddingLength > 0)
            {
                ret = read(c->sockfd_, tmp, responderHeader.paddingLength);
                assert(ret == responderHeader.paddingLength);
            }
            free(content);
        } // end of type FCGI_STDERR
        else if (responderHeader.type == FCGI_END_REQUEST)
        {
            FCGI_EndRequestBody endRequest;

            ret = read(c->sockfd_, &endRequest, sizeof(endRequest));
            assert(ret == sizeof(endRequest));
        }
    }
    // fprintf(stderr, "############RETURN FROM FCGI.c\n");
    return result;
}

char *findStartHtml(char *p)
{
    enum {
        S_NOPE,
        S_CR,
        S_CRLF,
        S_CRLFCR,
        S_CRLFCRLF
    } state = S_NOPE;

    for (char *content = p; *content != '\0'; content++)//状态机
    {
        switch (state) {
        case S_NOPE:
            if (*content == '\r') state = S_CR;
            break;
        case S_CR:
            state = (*content == '\n') ? S_CRLF : S_NOPE;
            break;
        case S_CRLF:
            state = (*content == '\r') ? S_CRLFCR : S_NOPE;
            break;
        case S_CRLFCR:
            state = (*content == '\n') ? S_CRLFCRLF : S_NOPE;
            break;
        case S_CRLFCRLF:
            return content;
        }
    }
    // fprintf(stderr, "%%%%%%%%%%RETURN NULL!!!!!\n");
    return p;
}

char *getHtmlFromContent(FastCgi_t *c, char *content)
{
    return findStartHtml(content);
    // /* 保存html内容开始位置 */
    // char *pt;

    // /* 读取到的content是html内容 */
    // if (c->flag_ == 1)
    // {
    //     //printf("%s",content);
    //     return findStartHtml(content);
    // }
    // else
    // {
    //     char *p = (char *)malloc(strlen(content));
    //     char *t = p;

    //     if ((pt = findStartHtml(content)) != NULL)
    //     {
    //         // if ((pt = content) != NULL) {
    //         c->flag_ = 1;
    //         for (char *i = pt; *i != '\0'; i++)
    //         {
    //             *t++ = *i;
    //             // printf("%c",*i);
    //         }
    //         *t = '\0';
    //     }
    //     return p;
    // }
}
