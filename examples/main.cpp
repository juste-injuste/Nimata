#include <iostream>
#include <atomic>

#include <iosfwd>
// #define NIMATA_DEBUGGING
#include "../include/Paleta.hpp"
#include "../include/Chronometro.hpp"
#include "../include/Nimata.hpp"

static std::atomic_uint work_count;

void work_1()    {        ++work_count; }
void work_2(int) {        ++work_count; }
int  work_3()    { return ++work_count; }
int  work_4(int) { return ++work_count; }

void pool_demo()
{
  std::cout << stz::clear;

  static unsigned  iteratings = 0;
  static stz::Pool pool;

  std::future<void> future_1;
  std::future<void> future_2;
  std::future<int>  future_3;
  std::future<int>  future_4;
  stz::measure_block()
  {
    stz::loop_n_times(10000)
    {
      /* void */ pool.push(work_1   );
      /* void */ pool.push(work_2, 0);
      future_3 = pool.push(work_3   );
      future_4 = pool.push(work_4, 0);
      
      future_1 = pool.push<stz::bound>(work_1   );
      future_2 = pool.push<stz::bound>(work_2, 0);
      future_3 = pool.push<stz::bound>(work_3   );
      future_4 = pool.push<stz::bound>(work_4, 0);
      
      /* void */ pool.push<stz::stray>(work_1   );
      /* void */ pool.push<stz::stray>(work_2, 0);
      /* void */ pool.push<stz::stray>(work_3   );
      /* void */ pool.push<stz::stray>(work_4, 0);
    };
    
    pool.wait();
  };

  std::cout << "iterating:          " << ++iteratings << '\n';
  std::cout << "completed work:     " <<   work_count << '\n';
  std::cout << "press enter to continue...\n";

  std::cin.get();
}

void cyclic_async_demo()
{
  stz::cyclic_async(std::chrono::seconds(1)) // clear screen every second
  {
    static unsigned frame = 0;
    std::cout << stz::clear;
    std::cout << "press enter to continue...\n";
    std::cout << "frame: " << ++frame << "\n";
    std::cout << "0%                        100%\n";
  };
  
  stz::sleep(1);

  stz::cyclic_async(200) // 200 milliseconds (aka 5 Hz)
  {
    std::cout << '='; // add 5 '=' to progress bar every second
  };

  stz::sleep(1);
  
  using namespace stz::_literals;
  stz::cyclic_async(25_Hz)
  {
    std::cout << '-'; // add 25 '-' to progress bar every second
  };
  
  std::cin.get();
}

template<typename, typename>
struct index
{
  size_t operator=(size_t k) { idx = k; return idx; }
  size_t idx;
  operator size_t() { return idx; }
};

void parfor_demo()
{
  std::cout << stz::clear;
  static int              array[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  static std::vector<int> vector  = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  static stz::Pool pool;

  stz::measure_block(", vector iterating sequential took: %ms")
  {
    for (int& value : vector)
    {
      stz::sleep(2);
      std::cout << value;
    }
  };

  stz::measure_block(", vector iterating parallel   took: %ms")
  {
    pool.parfor(int& value, vector)
    {
      stz::sleep(2);
      std::cout << value;
    };
  };

  stz::measure_block(", vector indexing  sequential took: %ms")
  {
    for (size_t k = 0; k < vector.size(); ++k)
    {
      stz::sleep(2);
      std::cout << vector[k];
    }
  };

  stz::measure_block(", vector indexing  parallel   took: %ms")
  {
    pool.parfor(size_t k, 0, vector.size())
    {
      stz::sleep(2);
      std::cout << vector[k];
    };
  };

  stz::measure_block(", array  iterating sequential took: %ms")
  {
    for (int& value : array)
    {
      stz::sleep(2);
      std::cout << value;
    }
  };

  stz::measure_block(", array  iterating parallel   took: %ms")
  {
    pool.parfor(int& value, array)
    {
      stz::sleep(2);
      std::cout << value;
    };
  };

  stz::measure_block(", array  indexing  sequential took: %ms")
  {
    for (size_t k = 0; k < (sizeof(array)/sizeof(*array)); ++k)
    {
      stz::sleep(2);
      std::cout << array[k];
    }
  };

  stz::measure_block(", array  indexing  parallel   took: %ms")
  {
    pool.parfor(size_t k, sizeof(array)/sizeof(*array))
    {
      stz::sleep(2);
      std::cout << array[k];
    };
  };

  std::cout << "press enter to continue...\n";
  std::cin.get();
}


int main()
{
  while (true)
  {
    pool_demo();
    
    cyclic_async_demo();

    parfor_demo();
  }
}
