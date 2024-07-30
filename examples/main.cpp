#include <chrono>
#include <iostream>
#include <atomic>

#define NIMATA_DEBUGGING
#include "../include/Nimata.hpp"
#include "../include/Chronometro.hpp"


static std::atomic_uint work_count;

int some_work()
{
  chz::sleep<chz::Unit::us>(1);
  return ++work_count;
}

int more_work(int i)
{
  chz::sleep<chz::Unit::us>(1);
  ++work_count;
  return i;
}

void other_work()
{
  chz::sleep<chz::Unit::us>(1);
  ++work_count;
}

void hihi_work(int)
{
  chz::sleep<chz::Unit::us>(1);
  ++work_count;
}

void threadpool_demo()
{
  static unsigned iterations = 0;
  using namespace std::chrono;
  mtz::Pool pool;
  pool.work();

  std::cout << "\033[H\033[J";
  
  for (auto measurement : chz::Measure())
  {
    for (unsigned k = 10000; k; --k)
    {
      auto f1 = pool.push(some_work);
      pool.push(other_work);
      auto f2 = pool.push(more_work, 1);
      pool.push(hihi_work, 1);
    }
    
    pool.wait();
    measurement.avoid(), std::cout << "iteration:          " << ++iterations << '\n';
  }
  std::cout << "completed work:     " << work_count << '\n';
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

  auto vector = std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  std::cout << "sequential iteration:\n";
  CHZ_MEASURE()
  {
    for (int& value : vector)
    {
      chz::sleep(1);
      std::cout << value;
    }
    std::cout << '\n';
  }

  static mtz::Pool pool;

  std::cout << "\nparallel iteration:\n";
  CHZ_MEASURE()
  {
    pool.parfor(int& value, vector)
    {
      chz::sleep(1);
      std::cout << value;
    };
    std::cout << '\n';
  }
  
  std::cout << "\nsequential indexing:\n";
  CHZ_MEASURE()
  {
    for (size_t k = 0; k < vector.size(); ++k)
    {
      chz::sleep(2);
      std::cout << vector[k];
    }
    std::cout << '\n';
  }

  std::cout << "\nparallel indexing:\n";
  CHZ_MEASURE()
  {
    pool.parfor(size_t k, 0, vector.size())
    {
      chz::sleep(2);
      std::cout << vector[k];
    };
    std::cout << '\n';
  }

  std::cin.get();
}

int main()
{
  while (true)
  {
    threadpool_demo();

    // cyclic_demo();

    // parfor_demo();
  }
}
