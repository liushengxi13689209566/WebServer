<div align="center">

WebServer


支持静态请求和 PHP 文件的简易 Web 服务器，实现了 GET、POST 等基本请求的解析与响应

![](https://img.shields.io/badge/release-v1.0-green.svg)
![](https://img.shields.io/badge/Base%20On-FastCGI-green.svg)
![](https://img.shields.io/badge/dependencies-up%20to%20date-green.svg)
![](https://img.shields.io/badge/Base%20On-rapidJSON-green.svg)

</div>

--------

##### 快速使用指南:

```cpp
const int port = atoi(argv[2]);
const char *ip = argv[1];

ConfigServer::ServerInit Init;
WebServer server(ip, port);
server.run();

```
--------

## 概述

使用　C++ 语言开发，采用 Reactor+线程池 的结构，维护状态机解析 HTTP 请求，使用 RapidJSON 解析 JSON 配置文件，通过 FastCGI 协议与 PHP 后台引擎进行通讯。

## 特性

- 使用多线程充分利用多核CPU，并使用 线程池 避免线程频繁创建销毁的开销
- 使用 Epoll ET 的 IO多路复用 技术，非阻塞式 IO,采用 Reactor+线程池 的结构
- 维护　有限状态机　去解析　Http 请求，提高解析速率
- 使用配置文件，配置服务器参数，并使用　RapidJSON库 解析 JSON 配置文件
- 通过 FastCGI 协议与 PHP 后台引擎进行通讯,以支持 PHP 请求　
- 支持在浏览器页面播放　音频和视频

## 服务器模型（图片来自网络，但是与我的模型完全相同）

![并发模型](https://github.com/linyacool/WebServer/blob/master/datum/model.png)


## 运行环境

* OS: Linux 
* Complier: g++ / clang++ 

## 安装与使用
    $ git clone git@github.com:liushengxi13689209566/WebServer.git
	$ cd WebServer/src
    $ make 
    $ ./WebServer 127.0.0.1 9981

## 具体使用方法　
```cpp

#include "Web_server.h"
using namespace std;
int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		cout << "usage : " << basename(argv[0]) << "   ip_address    port_number   " << endl;
		return 1;
	}
	const char *ip = argv[1];
	const int port = atoi(argv[2]);
	try
	{
		ConfigServer::ServerInit Init;
		WebServer my(ip, port);
		my.run();
	}
	catch (CallFailed &failed)
	{
		std::cout << failed.ErrString << failed.LineNo << std::endl;
		perror("The reason is :");
	}
	catch (...)
	{
		perror("发生了不可见的异常\n");
	}
	return 0;
}
```
## 运行结果
##### (1) 普通GET请求

![](https://github.com/liushengxi13689209566/WebServer/blob/master/image/get.png)

----- 

##### (2) 带参数的GET请求
![](https://github.com/liushengxi13689209566/WebServer/blob/master/image/get_01.png)
----- 

##### (3) POST请求
![](https://github.com/liushengxi13689209566/WebServer/blob/master/image/post.png)
----- 

##### (4) 浏览器页面播放　音频和视频
![](https://github.com/liushengxi13689209566/WebServer/blob/master/image/music.png)

----- 

![](https://github.com/liushengxi13689209566/WebServer/blob/master/image/video.png)

------
　
## WebBench 测试

统一使用命令

    ./webbench http://127.0.0.1:<端口号>/ -2 --get -c <客户端数量> -t 6


| 测试      |   5个客户端   | 20个客户端     |　 50个客户端     | 200个客户端     | 5000个客户端     |
| ---------- | :-----------:  | :-----------: |  :-----------: |  :-----------: | :-----------: |
| bytes/sec大小     |    340414304  |   332031744   |254892816|   299588320   |279898240      |


![](https://github.com/liushengxi13689209566/WebServer/blob/master/image/webbench.png)


| 表格      | 第一列     | 第二列     |
| ---------- | :-----------:  | :-----------: |
| 第一行     | 第一列     | 第二列     |





