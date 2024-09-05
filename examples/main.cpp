#include <iostream>
#include <atomic>

// #define NIMATA_DEBUGGING
#include "../include/Paleta.hpp"
#include "../include/Nimata.hpp"
#include "../include/Chronometro.hpp"

static std::atomic_uint work_count;

int  work_1()    { return ++work_count; }
void work_2()    {        ++work_count; }
void work_3(int) {        ++work_count; }

void threadpool_demo()
{
  std::cout << stz::clear;

  static unsigned  iterations = 0;
  static stz::Pool pool;

  std::future<int> future;
  STZ_MEASURE_BLOCK()
  {
    STZ_LOOP_FOR_N(10000)
    {
      future = pool.push(work_1);
      pool.push(work_2);
      pool.push(work_3, 1);
      // pool.push(std::function<void()>()); // would not be a issue as tasks are checked for validity.
    }
    
    pool.wait();
  }

  std::cout << "iteration:          " << ++iterations << '\n';
  std::cout << "completed work:     " <<   work_count << '\n';
  std::cout << "press enter to continue...\n";

  std::cin.get();
}

void cyclic_async_demo()
{
  using namespace stz::_literals;

  stz::cyclic_async(std::chrono::seconds(1)) // clear screen every second
  {
    static unsigned frame = 0;
    std::cout << stz::clear;
    std::cout << "press enter to continue...\n";
    std::cout << "frame: " << ++frame << "\n";
    std::cout << "0%                        100%\n";
  };
  
  stz::sleep(1);

  stz::cyclic_async(5_Hz)
  {
    std::cout << '='; // add 5 '=' to progress bar every second
  };

  stz::sleep(1);
  
  stz::cyclic_async(25_Hz)
  {
    std::cout << '-'; // add 25 '-' to progress bar every second
  };
  
  std::cin.get();
}

void parfor_demo()
{
  std::cout << stz::clear;

  static auto      vector = std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  static stz::Pool pool;

  STZ_MEASURE_BLOCK(", sequential iteration took: %ms")
  for (int& value : vector)
  {
    stz::sleep(2);
    std::cout << value;
  }
  
  STZ_MEASURE_BLOCK(", sequential indexing took:  %ms")
  for (size_t k = 0; k < vector.size(); ++k)
  {
    stz::sleep(2);
    std::cout << vector[k];
  }

  STZ_MEASURE_BLOCK(", parallel iteration took:   %ms")
  pool.parfor(int& value, vector)
  {
    stz::sleep(2);
    // std::cout << value;
  };

  STZ_MEASURE_BLOCK(", parallel indexing took:    %ms")
  pool.parfor(size_t k, 0, vector.size())
  {
    stz::sleep(2);
    // std::cout << vector[k];
  };

  STZ_MEASURE_BLOCK(", anonymous indexing took:    %ms")
  pool.parfor(..., 0, vector.size())
  {
    stz::sleep(2);
  };

  std::cout << "press enter to continue...\n";
  std::cin.get();
}


int main()
{
  while (true)
  {
    threadpool_demo();
    
    cyclic_async_demo();

    parfor_demo();
  }
}
