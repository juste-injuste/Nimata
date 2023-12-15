#include <chrono>
#include <iostream>
#include <atomic>

#define NIMATA_LOGGING
#include "../include/Nimata.hpp"

static std::atomic_uint work_count;
int some_work()
{
  // std::this_thread::sleep_for(std::chrono::milliseconds(10));
  return ++work_count;
}

int more_work(int i)
{
  ++work_count;
  return i;
}

void other_work()
{
  ++work_count;
}

void hihi_work(int)
{
  ++work_count;
}

void threadpool_demo()
{
  static unsigned iterations = 0;
  using namespace std::chrono;
  Nimata::Pool pool;

  work_count = 0;
  std::cout << "\033[H\033[J";
  auto total_start = steady_clock::now();
  for (unsigned k = 5000; k; --k)
  {
    pool.push(some_work);
    pool.push(other_work);
    pool.push(more_work, 1);
    pool.push(hihi_work, 1);
  }
  pool.wait();
  auto total_end     = steady_clock::now();
  auto total_elapsed = duration_cast<milliseconds>(total_end - total_start);
  iterations++;
  std::cout << "iteration:          " << iterations << '\n';
  std::cout << "total elapsed time: " << total_elapsed.count() << " ms\n";
  std::cout << "completed work:     " << work_count << '\n';
  std::cout << "press enter to continue...\n";
  std::cin.get();
}

void cyclic_demo()
{
  using namespace Nimata::Literals;

  NIMATA_CYCLIC(1_Hz) // clear screen every second
  {
    static unsigned frame = 0;
    std::cout << "\033[H\033[J";
    std::cout << "press enter to continue...\n";
    std::cout << "frame: " << ++frame << "\n";
    std::cout << "0%                        100%\n";
  };

  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  NIMATA_CYCLIC(5_Hz)
  {
    std::cout << '='; // add 5 '=' to progress bar every second
  };
  
  NIMATA_CYCLIC(25_Hz)
  {
    std::cout << '-'; // add 25 '-' to progress bar every second
  };
  
  std::cin.get();
}

int main()
{ 
  Nimata::Pool p(-123124);
  // while (true)
  // {
  //   threadpool_demo();

  //   // cyclic_demo();
  // }
}
