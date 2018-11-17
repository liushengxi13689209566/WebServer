#include <future>
#include <iostream>


int is_prime(int x)
{
  for (int i=0; i<x; i++)
  {
    if (x % i == 0)
      return -1  ;
  }
  return  666 ;
}

int main()
{
  std::future<int> fut = std::async(is_prime, 7000);
  std::cout << "please wait";
  std::chrono::milliseconds span(100);
  while (fut.wait_for(span) != std::future_status::ready)
    std::cout << ".";
  std::cout << std::endl;

  int ret = fut.get();
  std::cout << "final result: " <<  ret << std::endl;
  return 0;
}