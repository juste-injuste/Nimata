#include "Nimata.hpp"
#include <iostream>

void some_work()
{
    std::cout << "some work\n";
}


int main()
{
  Nimata::Pool<10> pool;

  pool.workers_[2].assign_work(some_work);

  pool.wait();
  std::cout << "done\n";
}
