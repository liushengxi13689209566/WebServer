## fastcgi使用（支持php)
### GET
main.c是一个很简单get的例子</br>
参数就是需要发送文件的名字，使用get方法
sendParams(c, "SCRIPT_FILENAME","/home/cwh/FastCgi/index.php");
sendParams(c, "REQUEST_METHOD","GET");

### POST
post需要注意的是还要发body，所以需要加几个东西</br>
auto t = makeHeader(5, 1, req.GetBodySize(), 0);//制造头为了发body</br>
send(c->sockfd_, &t, sizeof(t), 0);</br>
send(c->sockfd_, req.GetBody(), </br>
req.GetBodySize(), 0);</br>
t = makeHeader(5, 1, 0, 0);</br>
//说明body发完了</br>
send(c->sockfd_, &t, sizeof(t), 0);

