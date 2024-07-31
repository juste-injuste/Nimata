#include <chrono>
#include <iostream>
#include <atomic>

#define NIMATA_DEBUGGING
#include "../include/Nimata.hpp"
#include "../include/Chronometro.hpp"


static std::atomic_uint work_count;

int  work_1()    { return ++work_count; }
void work_2()    {        ++work_count; }
void work_3(int) {        ++work_count; }

void threadpool_demo()
{
  std::cout << "\033[H\033[J";

  static unsigned iterations = 0;
  mtz::Pool pool;
  
  std::future<int> future;
  CHZ_MEASURE()
  {
    CHZ_LOOP_FOR(10000)
    {
      future = pool.push(work_1);
      pool.push(work_2);
      pool.push(work_3, 1);
    }
    
    pool.wait();
  }

  std::cout << "iteration:          " << ++iterations << '\n';
  std::cout << "completed work:     " <<   work_count << '\n';
  std::cout << "press enter to continue...\n";

  std::cin.get();
}

void cyclic_demo()
{
  using namespace mtz::_literals;

  MTZ_CYCLIC(1_Hz) // clear screen every second
  {
    static unsigned frame = 0;
    std::cout << "\033[H\033[J";
    std::cout << "press enter to continue...\n";
    std::cout << "frame: " << ++frame << "\n";
    std::cout << "0%                        100%\n";
  };

  std::this_thread::sleep_for(std::chrono::nanoseconds(1));

  MTZ_CYCLIC(5_Hz)
  {
    std::cout << '='; // add 5 '=' to progress bar every second
  };
  
  MTZ_CYCLIC(25_Hz)
  {
    std::cout << '-'; // add 25 '-' to progress bar every second
  };
  
  std::cin.get();
}

void parfor_demo()
{
  std::cout << "\033[H\033[J";

  static auto vector = std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  CHZ_MEASURE(", sequential iteration took: %ms")
  for (int& value : vector)
  {
    chz::sleep(2);
    std::cout << value;
  }
  
  CHZ_MEASURE(", sequential indexing took:  %ms")
  for (size_t k = 0; k < vector.size(); ++k)
  {
    chz::sleep(2);
    std::cout << vector[k];
  }

  mtz::Pool pool;

  CHZ_MEASURE(", parallel iteration took:   %ms")
  pool.parfor(int& value, vector)
  {
    chz::sleep(2);
    std::cout << value;
  };

  CHZ_MEASURE(", parallel indexing took:    %ms")
  pool.parfor(size_t k, 0, vector.size())
  {
    chz::sleep(2);
    std::cout << vector[k];
  };

  std::cout << "press enter to continue...\n";
  std::cin.get();
}

int main()
{
  while (true)
  {
    threadpool_demo();

    cyclic_demo();

    parfor_demo();
  }
}
