#include "Nimata.hpp"
#include <iostream>
#include <random>
#include <chrono>
#include <sstream>

std::random_device device;
std::mt19937 generator(device());
std::uniform_int_distribution<> uniform_distribution(1, 10);

void some_work()
{
  int sleep_for_sec = uniform_distribution(generator);

  std::stringstream temp1;
  temp1 << "sleeping for: " << sleep_for_sec << "\n";
  std::cout << temp1.str();

  std::this_thread::sleep_for(std::chrono::seconds{sleep_for_sec});

  std::stringstream temp2;
  temp2 << "done sleeping for: " << sleep_for_sec << "\n";
  std::cout << temp2.str();
}

int main()
{
  Nimata::Pool<1> pool;

  pool.do_work(some_work);

  pool.join_all();
  
  std::cout << "done\n";
}
