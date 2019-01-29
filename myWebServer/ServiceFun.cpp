/*************************************************************************
	> File Name: ServiceFun.cpp
	> Author: Liu Shengxi
	> Mail: 13689209566@163.com
	> Created Time: 2018年07月23日 星期一 22时54分52秒
 ************************************************************************/

#include "http_conn.h"
#include "ThreadPool.h"
using namespace std;
WebServer::WebServer(const string ip, const int port)
{
	start(ip, port);
}
WebServer::~WebServer()
{
	close(conn_fd);
}

void WebServer::addfd(int epollfd, int fd, bool oneshot)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	if (oneshot)
	{
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}
int WebServer::setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

int WebServer::start(const string ip, const int port)
{
	struct sockaddr_in address, client_address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
	address.sin_port = htons(port);

	int socketfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(socketfd >= 0); //  right  expression

	int optval = 1; //设置该套接字使之可以重新绑定端口
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (int *)&optval, sizeof(int)) < 0)
		printf("setsocketopt failed \n ");

	int ret = bind(socketfd, (struct sockaddr *)&address, sizeof(address));
	assert(ret != -1);
	/*将该套接字和套接字对应的连接队列长度告诉 Linux 内核,不会阻塞 */
	/*服务器进程不能随便指定一个数值，内核有一个许可的范围。这个范围是实现相关的。很难有某种统一，一般这个值会小30以内*/
	ret = listen(socketfd, 30); //
	assert(ret != -1);

	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(30);
	assert(epollfd != -1);

	addfd(epollfd, socketfd, false);

	threadpool pool(30);

	while (1)
	{
		int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if (ret < 0)
		{
			throw "epoll_wait";
			break;
		}
		for (int i = 0; i < ret; i++)
		{
			conn_fd = events[i].data.fd;
			if (conn_fd == socketfd)
			{
				socklen_t client_addrlength = sizeof(client_address);
				int temp_fd = accept(socketfd, (struct sockaddr *)&client_address, &client_addrlength);
				if (temp_fd < 0)
					throw " accept ";
				addfd(epollfd, temp_fd, true);
			}
			else if (events[i].events & EPOLLIN) //有数据来到
			{
				//向线程池添加任务
				pool.commit(WebServer::worker, conn_fd, epollfd);
			}
			else
			{
				throw " Epoll Other Error ";
			}
		}
	}
	close(socketfd);
	return 0;
}
void WebServer::reset_oneshot(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int WebServer::worker(int fd, int epollfd)
{
	//cout << "线程函数 " << endl;
	char temp_buffer[BUFFER_SIZE];
	memset(temp_buffer, 0, BUFFER_SIZE);
	while (1)
	{
		int size = recv(fd, temp_buffer, BUFFER_SIZE - 1, 0);
		if (size == 0)
		{
			cout << "client have been closed !!!! " << endl;
			break;
		}
		else if (size < 0)
		{
			if (errno == EAGAIN)
			{
				reset_oneshot(epollfd, fd);
				cout << "read later !!! " << endl;
				break; //相当于退出该线程
			}
		}
		else //size > 0
		{
			string recv_buffer = temp_buffer;
			stringstream ss{recv_buffer};
			string method, filename;
			ss >> method >> filename;
			/*GET请求*/
			if (method == "GET")
			{
				reponse_GET(filename, fd);
				break;
			}
			/*POST请求*/
			else if (method == "POST")
			{
				string data = Analysis_POST(recv_buffer);
				reponse_POST(filename, data, fd);
				break;
			}
			/*未知的 method */
			else
			{
				reponse_noKnow(fd);
				break;
			}
		}
	}
	close(fd);
}
/**/
string WebServer::Analysis_POST(std::string recv_buffer)
{
	auto start_pos = recv_buffer.find("Content-Length:");
	auto end_pos = recv_buffer.find("\r\n", start_pos);
	string size = recv_buffer.substr(start_pos + 15, end_pos);
	int data_size = stoi(size);
	string tmp;
	for (int i = recv_buffer.size() - data_size; i < recv_buffer.size(); i++)
		tmp += recv_buffer[i];
	return tmp;
}
/*生成http响应报头部*/
int WebServer::make_headers(const int statue, const int fd, std::string filename)
{
	string http_head = "HTTP/1.1 " + to_string(statue) + " OK\r\n";
	if (filename.find(".html") != string::npos)
		http_head += "Content-Type: text/html\r\n;charset=utf-8\r\n\r\n";
	if (filename.find(".jpg") != string::npos)
		http_head += "Content-Type: image/jpg\r\n;charset=utf-8\r\n\r\n";
	if (filename.find(".png") != string::npos)
		http_head += "Content-Type: image/png\r\n;charset=utf-8\r\n\r\n";
	if (filename.find(".ico") != string::npos)
		http_head += "Content-Type: image/x-icon\r\n;charset=utf-8\r\n\r\n";
	if (filename.find(".mp3") != string::npos)
		http_head += "Content-Type: audio/mp3\r\n;charset=utf-8\r\n\r\n";
	if (filename.find(".mp4") != string::npos)
		http_head += "Content-Type: video/mpeg4\r\n;charset=utf-8\r\n\r\n";
	/*
	HTTP/1.1  200(状态码)     OK
	Content-type: text/html或者是 image/jpg
	*/
	int ret = send(fd, http_head.c_str(), http_head.size(), 0);
	if (ret < 0)
		throw " send ";
	return 0;
}
int WebServer::not_found(const int fd, std::string filename = PATH_404) //404.html
{
	make_headers(404, fd, ".html");
	send_file(fd, filename);
	return 0;
}
int WebServer::send_file(const int fd, std::string file) //简单的向浏览器发送文件
{
	int filefd = open(file.c_str(), O_RDONLY); //open 只打开文件 ,文件存在才让她发送哦，所以直接发送就行了
	struct stat filestat;
	int ret = stat(file.c_str(), &filestat);
	if (ret < 0)
		throw "stat";

	sendfile(fd, filefd, NULL, filestat.st_size);

	// int sum = 0, file_len = 0;
	// char temp[CHAR_MAX_SIZE];
	// while (sum != filestat.st_size)
	// {
	// 	memset(temp, 0, CHAR_MAX_SIZE);
	// 	file_len = read(filefd, temp, CHAR_MAX_SIZE-1);
	// 	send(fd, temp, file_len, 0);
	// 	sum += file_len;
	// }
	// sleep(1);

	close(filefd);
	return 0;
}
/*不知道的 method*/
int WebServer::reponse_noKnow(const int fd)
{
	make_headers(501, fd, ".html");
	send_file(fd, PATH_501);
	return 0;
}
int WebServer::reponse_GET(string filename, const int fd)
{
	string file = ROOT_PATH;
	if (filename == "/")
		file += "/index.html";
	else
		file += filename;

	if (filename.find("404.png") != string::npos)
		file = PATH_404_PNG;
	if (filename.find("666.ico") != string::npos)
		file = PATH_ICO;

	struct stat filestat;
	int ret = stat(file.c_str(), &filestat);  // 执行成功则返回0，失败返回-1，错误代码存于errno
	if (ret < 0 || S_ISDIR(filestat.st_mode)) // 处理文件不存在的情况
	{
		not_found(fd);
		return 0;
	}
	//只要从这里下来，所请求的文件就一定会存在

	/*可能会有bug，待看下一步*/
	bool is_dynamic = false;
	string argv;
	auto pos = filename.find("?");
	if (pos != string::npos)
	{ //存在“？”号  动态请求
		//auto end = filename.find("\r\n");
		argv = filename.substr(pos);
		cout << " argv ==  " << argv << endl;
		is_dynamic = true;
	}
	else
	{
		// 静态请求
		//cout << "INTO 静态请求" << endl;
		//cout << "file == " << file << endl;
		make_headers(200, fd, file);
		send_file(fd, file);
	}
	return 0;
}
int WebServer::reponse_POST(std::string filename, std::string data, const int fd)
{
	string file = ROOT_PATH;
	file += filename; //******/realWebServer/cgi/adder
	struct stat filestat;
	int ret = stat(file.c_str(), &filestat); //文件不存在
	if (ret < 0 || S_ISDIR(filestat.st_mode))
	{
		not_found(fd);
		return 0;
	}
	char argv[CHAR_MAX_SIZE] = {0};
	int a, b;
	ret = sscanf(data.c_str(), "a=%d&b=%d", &a, &b); //参数错误的情况
	if (ret < 0 || ret != 2)
	{
		make_headers(666, fd, ".html");
		send_file(fd, PATH_666);
		return 0;
	}
	memset(argv, 0, sizeof(argv));
	sprintf(argv, "%d-%d", a, b);

	char tmp[1024]; //      rvbiv/cgi/adder
	int k;
	for (k = 0; k < file.size(); k++)
	{
		tmp[k] = file[k];
	}
	tmp[k] = '\0';
	if (fork() == 0)
	{
		dup2(fd, STDOUT_FILENO);
		execl(tmp, argv);
		//tmp是一个可执行文件，接受argv参数，他里面的输出就会直接到 socketed 中
	}
	while (1)
	{
		//wait()函数的返回值是子进程的pid
		ret = wait(NULL);
		if (ret == -1)
		{
			//父进程 wait()函数阻塞过程中，有可能被别的信号中断，需要做异常处理
			if (errno == EINTR)
			{
				continue;
			}
			break;
		}
	}
}
