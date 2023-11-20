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
  using namespace Nimata;

  Pool<1> pool;

  pool.do_work(some_work);

  pool.join_all();

  // NIMATA_ASYNC(1_Hz)
  // {
  //   std::cout << "1 Hz\n";
  // };

  // NIMATA_ASYNC(3_Hz)
  // {
  //   std::cout << "3 Hz\n";
  // };

  std::cin.get();
  
  std::cout << "done\n";
}
