#include <chrono>
#include <iostream>
#include <atomic>

#define NIMATA_DEBUGGING
#include "../include/Nimata.hpp"
#include "../include/Chronometro.hpp"


static std::atomic_uint work_count;
int some_work()
{
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
  mtz::Pool pool;
  pool.work();

  work_count = 0;
  std::cout << "\033[H\033[J";
  auto total_start = steady_clock::now();
  for (unsigned k = 5000; k; --k)
  {
    auto f1 = pool.push(some_work);
    pool.push(other_work);
    auto f2 = pool.push(more_work, 1);
    pool.push(hihi_work, 1);
  }
  pool.wait();
  auto total_end     = steady_clock::now();
  auto total_elapsed = duration_cast<milliseconds>(total_end - total_start);
  ++iterations;
  std::cout << "iteration:          " << iterations << '\n';
  std::cout << "total elapsed time: " << total_elapsed.count() << " ms\n";
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

  std::this_thread::sleep_for(std::chrono::milliseconds(1));

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

  //-----------------------
  for (size_t k = 0; k < 10; ++k)
  {
    std::cout << k;
  }
  //-----------------------


  std::cout << '\n';

  //-----------------------
  mtz::Pool pool1(3);
  // pool1.parfor(0, 10)
  // {
  //   std::cout << mtz::Pool::idx;
  // };
  //-----------------------

  std::cout << '\n';

  //-----------------------
  mtz::Pool pool2(3);
  // MTZ_PARFOR(pool2, 10)
  // {
  //   std::cout << MTZ_PARFOR_IDX;
  // };
  //-----------------------

  std::cout << '\n';

  //-----------------------
  // mtz::Pool(10).parfor(10)
  // {
  //   std::cout << mtz::Pool::idx;
  // };
  //-----------------------

  std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7};
  pool1.parfor(val, vec)
  {
    chz::sleep(10);
    std::cout << val;
  };

  std::cout << '\n';

  pool1.parfor(idx, 'A', 'Q')
  {
    chz::sleep(10);
    std::cout << idx;
  };


  std::cout << '\n';

  pool1.parfor(idx, 'a', 'k')
  {
    std::cout << idx;
  };
  
  std::cin.get();
}

int main()
{
  while (true)
  {
    // threadpool_demo();

    // cyclic_demo();

    parfor_demo();
  }
}
