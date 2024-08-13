#include "Nimata.hpp"
#include <thread>
#include <chrono>
#include <iostream>

stz::Pool pool(4);

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
  auto end     = steady_clock::now();
  auto elapsed = static_cast<float>(duration_cast<milliseconds>(end - start).count())*0.001f;

  // would take 10 seconds if it were single-threaded
  std::cout << "elapsed time: " << elapsed << " s\n";
}