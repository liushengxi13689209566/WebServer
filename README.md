<div align="center">

                    WebServer


A C++ Web Server 

支持静态请求和 PHP 文件的简易 Web 服务器，实现了 GET、POST 等基本请求的解析与响应

![](https://img.shields.io/badge/release-v1.0-blue.svg)
![](https://img.shields.io/badge/build-passing-green.svg)
![](https://img.shields.io/badge/dependencies-up%20to%20date-green.svg)
![](https://img.shields.io/badge/license-MIT-blue.svg)

</div>
--------
##### 快速使用指南:

```cpp


```
##### 运行结果:

--------

## Introduction  

C++ 语言开发，采用 Reactor+线程池 的结构，维护状态机解析 HTTP 请求，使用 RapidJSON 解析 JSON 配置文件，通过 FastCGI 协议与 PHP 后台引擎进行通讯。

## Envoirment  

* OS: Linux 
* Complier: g++ / clang++ 

## Build

	cd /WebServer/src && make 

## Usage

	./WebServer ip port 

## Technical points

* 使用Epoll边沿触发的IO多路复用技术，非阻塞IO，使用Reactor模式
* 使用多线程充分利用多核CPU，并使用线程池避免线程频繁创建销毁的开销
* 使用基于小根堆的定时器关闭超时请求
* 主线程只负责accept请求，并以Round Robin的方式分发给其它IO线程(兼计算线程)，锁的争用只会出现在主线程和某一特定线程中
* 使用eventfd实现了线程的异步唤醒
* 使用双缓冲区技术实现了简单的异步日志系统
* 为减少内存泄漏的可能，使用智能指针等RAII机制
* 使用状态机解析了HTTP请求,支持管线化
* 支持优雅关闭连接
 
## Model

并发模型为Reactor+非阻塞IO+线程池，新连接Round Robin分配，详细介绍请参考[并发模型](https://github.com/linyacool/WebServer/blob/master/并发模型.md)
![并发模型](https://github.com/linyacool/WebServer/blob/master/datum/model.png)



## Others
除了项目基本的代码，进服务器进行压测时，对开源测试工具Webbench增加了Keep-Alive选项和测试功能: 改写后的[Webbench](https://github.com/linyacool/WebBench)

## 
明日继续更新．．．
