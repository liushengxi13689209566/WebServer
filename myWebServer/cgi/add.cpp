/*************************************************************************
	> File Name: add.c
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年07月25日 星期三 15时40分54秒
 ************************************************************************/

#include <stdio.h>
#include <string>
#include<iostream>
using namespace std;
int main(int argc, char *argv[])
{ 
	int a, b;
	int ret = sscanf(argv[1], "%d-%d", &a,&b);
	string body = "<html>\r\n<head>\r\n<title> Tattoo_Welkin </title>\r\n";
	body += "<meta charset=""utf-8"">\r\n";
	body += "<link rel=""shortcut icon"" href=""666.ico"" />\r\n";
	body += "<link rel=""bookmark"" href=""666.ico"" />\r\n</head>\r\n";
	body +="<body >\r\n";
	body += " 200\r\n";
	body += "<p>GET: Add success \r\n" ;
	body += "<hr>\r\n<p>The answer of " + to_string(a) 
	+ " + " +  to_string(b) + " = " +  to_string(a+b) + "\r\n" ;
	body += " <h4>------ By Tattoo </h4></body>\r\n</html>\r\n";

	//设置请求头
	string http_head = "HTTP/1.1  200  OK\r\n";
	http_head += "Content-Type: text/html\r\n\r\n";

    cout << http_head << endl  ;
    cout  << body  <<  endl ;

    return 0;
}
