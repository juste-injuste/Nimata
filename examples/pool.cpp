#include "../include/Nimata.hpp"
#include <thread>
#include <chrono>
#include <iostream>

Nimata::Pool pool(4);

void work()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main()
{
  using namespace std::chrono;

  auto start = steady_clock::now();
  for (unsigned n = 100; n; --n)
  {
    pool.push(work);
  }
  pool.wait();
  auto end = steady_clock::now();
  auto elapsed = duration_cast<milliseconds>(end - start).count()*0.001;

  std::cout << "elapsed time: " << elapsed << " s\n";
}