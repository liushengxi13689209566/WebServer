CC = clang++ 
DEBUG = -g
OPT = -O2 #优化
LIBRARY= -lpthread
FILE = webserver

all:
	${CC} http_main.cpp  ./base_funtion/*.cpp -Wall -std=c++11 -o ${FILE} ${DEBUG} ${LIBRARY}

	${CC} testthreadPool.cpp   -Wall -std=c++11 -o pool ${DEBUG} ${LIBRARY}
