
#include "thread_pool.h"


using namespace std;
void fun(void)
{
    std::cout<<"hello"<<std::endl;
}

int main(int argc,char **argv)
{
  netlib::ThreadPool pool(10);
  pool.start();
  while(1)
  {
    pool.append(fun);
  }  
  return 0;
}