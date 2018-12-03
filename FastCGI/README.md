### **Description**

C language through FastCGI protocol, through php-fpm, php file parsed into html files.

### **How to use**

**Preparation：**
	
please make sure you have installed [php-fpm](https://php-fpm.org/).

Testing environment：CentOS 7

configuration file：/usr/local/php/etc/php-fpm.conf
	
php file：my own user home directory ---/home/Tanswer/index.php
	
If you want to run, you need to change the  php  file path of main.c.
	
	
 1. Modify the configuration
	
	After the installation is complete, the default communication method for unix local domain socket communication, our example and php-fpm for **TCP** communication, so we should change the configuration, the ip address is set to **127.0.0.1**, listening port **9000**. As follows：

	![enter image description here](http://test-1252727452.costj.myqcloud.com/github/2017-12-23%2013-36-24%20%E7%9A%84%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE.png)
	
 
 2. restart php-fpm 
	
	`sudo service php-fpm restart`

 3. compiling and running
	```
	git clone git@github.com:Tanswer/FastCGI.git
	cd FastCGI && make
	./main
	```
	found an error after running：
	`error:Unable to open primary script: /home/Tanswer/index.php (No such file or directory)`

	This is the issue of permissions, the permissions of index.php are：
	`-rw-r--r-- 1 Tanswer Tanswer 64 12月 23 13:16 index.php`.  From the above default configuration file can be seen, the account that started the php-fpm process is www, and here we change it to Tanswer，as follows：

	![enter image description here](http://test-1252727452.costj.myqcloud.com/github/2017-12-23%2013-54-15%20%E7%9A%84%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE.png)

	run again `./main`, the result is as follows：

	![enter image description here](http://test-1252727452.costj.myqcloud.com/github/2017-12-23%2014-01-02%20%E7%9A%84%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE.png)


### **Reference material**

 - [fastcgi协议分析与实例](http://blog.csdn.net/shreck66/article/details/50355729)
 - [php-fpm - 启动参数及重要配置详解](http://www.4wei.cn/archives/1002061)
