/*---author-----------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Nimata

-----licence----------------------------------------------------------------------------------------

MIT License

Copyright (c) 2023 Justin Asselin (juste-injuste)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-----versions---------------------------------------------------------------------------------------

-----description------------------------------------------------------------------------------------

-----inclusion guard------------------------------------------------------------------------------*/
#ifndef NIMATA_HPP
#define NIMATA_HPP
// --necessary standard libraries-------------------------------------------------------------------
#include <thread>     // for std::thread
#include <mutex>      // for std::mutex, std::lock_guard
#include <atomic>     // for std::atomic_bool
#include <deque>      // for std::deque
#include <functional> // for std::function
#include <chrono>     // for std::chrono::high_resolution_clock, std::chrono::nanoseconds
#if defined(NIMATA_LOGGING)
# include <cstdio>    // for std::sprintf
#endif
#include <ostream>    // for std::ostream
#include <iostream>   // for std::clog
#include <sstream>
// --Nimata library---------------------------------------------------------------------------------
namespace Nimata
{
  namespace Version
  {
    const long MAJOR = 000;
    const long MINOR = 001;
    const long PATCH = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }

  using Work = std::function<void()>;

  template<unsigned N>
  class Pool;

# define NIMATA_CYCLIC(period_us)
  
  using Period = std::chrono::nanoseconds::rep;

  inline namespace Literals
  {
    constexpr Period operator ""_mHz(long double frequency);
    constexpr Period operator ""_mHz(unsigned long long frequency);
    constexpr Period operator ""_Hz(long double frequency);
    constexpr Period operator ""_Hz(unsigned long long frequency);
    constexpr Period operator ""_kHz(long double frequency);
    constexpr Period operator ""_kHz(unsigned long long frequency);
  }

  namespace Global
  {
    std::ostream log{std::clog.rdbuf()}; // logging ostream
  }
// --Nimata library: backend forward declaration----------------------------------------------------
  namespace Backend
  {
    class Worker;

    template<Period period>
    class CyclicWorker;

# if defined(NIMATA_LOGGING)
#   define NIMATA_LOG(...)                          \
      [&](const char* caller){                      \
        static char buffer[255];                    \
        sprintf(buffer, __VA_ARGS__);               \
        Nimata::Global::log << caller << ": ";      \
        Nimata::Global::log << buffer << std::endl; \
      }(__func__)
# else
#   define NIMATA_LOG(...)
# endif
  }
// --Nimata library: frontend struct and class definitions------------------------------------------
  template<unsigned N>
  class Pool final
  {
  public:
    inline Pool() noexcept;
    inline ~Pool() noexcept;
    inline void execute(Work work_to_do) noexcept;
    inline void wait() noexcept;
    const unsigned size = N;
  private:
    inline void async_assignation() noexcept;
    Backend::Worker  workers[N];
    std::mutex       mutex;
    std::deque<Work> work_queue;
    std::atomic_bool alive = {true};
    std::thread      assigning_thread{async_assignation, this};
  };
// --Nimata library: backend  struct and class definitions------------------------------------------
  namespace Backend
  {
    class Worker final
    {
    public:
      inline ~Worker() noexcept;
      inline void assign(Work& work) noexcept;
      inline bool working() const noexcept;
      inline void body();
    private:
      Work             work          = nullptr;
      std::atomic_bool work_is_valid = {false};
      volatile bool    alive         = true;
      std::thread      worker_thread{body, this};
    };

    template<Period period>
    class CyclicWorker final
    {
    static_assert(period >= 0, "period must be greater than 0");
    public:
      inline CyclicWorker(Work work_to_do) noexcept;
      inline CyclicWorker(const CyclicWorker&) noexcept {}
      inline ~CyclicWorker() noexcept;
    private:
      inline void body();
      Work          work  = nullptr;
      volatile bool alive = true;
      std::thread   worker_thread{body, this};
    };
  }
// --Nimata library: frontend definitions-----------------------------------------------------------
  template<unsigned N>
  Pool<N>::Pool() noexcept
  {
    NIMATA_LOG("%u threads aquired", N);
  }

  template<unsigned N>
  Pool<N>::~Pool() noexcept
  {
    while (work_queue.empty() == false)
    {
      std::this_thread::sleep_for(std::chrono::microseconds{1});
    };

    for (unsigned k = 0; k < N; ++k)
    {
      while (workers[k].working())
      {
        std::this_thread::sleep_for(std::chrono::microseconds{1});
      }
    }

    alive = false;
    assigning_thread.join();
  }

  template<unsigned N>
  void Pool<N>::execute(Work work_to_do) noexcept
  {
    if (work_to_do)
    {
      std::lock_guard<std::mutex> lock{mutex};
      work_queue.push_back(work_to_do);
    }
  }

  template<unsigned N>
  void Pool<N>::async_assignation() noexcept
  {
    while (alive)
    {
      for (unsigned k = 0; k < N; ++k)
      {
        if (workers[k].working() == false)
        {
          std::lock_guard<std::mutex> lock{mutex};
          if (work_queue.empty() == false)
          {
            workers[k].assign(work_queue.front());
            work_queue.pop_front();
            NIMATA_LOG("assigned to worker thread #%02u", k);
          }
        }
      }
    }
  }

  template<unsigned N>
  void Pool<N>::wait() noexcept
  {
    while (work_queue.empty() == false)
    {
      std::this_thread::sleep_for(std::chrono::microseconds{1});
    };

    for (unsigned k = 0; k < N; ++k)
    {
      while (workers[k].working() == true)
      {
        std::this_thread::sleep_for(std::chrono::microseconds{1});
      }
    }

    NIMATA_LOG("all threads finished their work");
  }
  
# undef  NIMATA_CYCLIC
# define NIMATA_CYCLIC_IMPL(period_us, line)                                          \
    Nimata::Backend::CyclicWorker<period_us> cyclic_worker_##line = (Nimata::Work)[&]
# define NIMATA_CYCLIC_PROX(...)  NIMATA_CYCLIC_IMPL(__VA_ARGS__)
# define NIMATA_CYCLIC(period_us) NIMATA_CYCLIC_PROX(period_us, __LINE__)

  inline namespace Literals
  {
    constexpr Period operator ""_mHz(long double frequency)
    {
      return 1000000000000/frequency;
    }

    constexpr Period operator ""_mHz(unsigned long long frequency)
    {
      return 1000000000000/frequency;
    }

    constexpr Period operator ""_Hz(long double frequency)
    {
      return 1000000000/frequency;
    }

    constexpr Period operator ""_Hz(unsigned long long frequency)
    {
      return 1000000000/frequency;
    }
    
    constexpr Period operator ""_kHz(long double frequency)
    {
      return 1000000/frequency;
    }

    constexpr Period operator ""_kHz(unsigned long long frequency)
    {
      return 1000000/frequency;
    }
  }
// --Nimata library: backend definitions------------------------------------------------------------
  namespace Backend
  {
    Worker::~Worker() noexcept
    {
      alive = false;
      worker_thread.join();
    }

    void Worker::assign(Work& work_to_do) noexcept
    {
      work          = std::move(work_to_do);
      work_is_valid = true;
    }

    bool Worker::working() const noexcept
    {
      return work_is_valid;// || work;
    }

    void Worker::body()
    {
      while (alive)
      {
        if (work_is_valid)
        {
          work();
          work = nullptr;
          work_is_valid = false;
        }
      }
    }

    template<Period period>
    CyclicWorker<period>::CyclicWorker(Work work_to_do) noexcept :
      work{work_to_do}
    {
      NIMATA_LOG("thread spawned");
    }

    template<Period period>
    CyclicWorker<period>::~CyclicWorker() noexcept
    {
      alive = false;
      worker_thread.join();
    }

    template<Period period>
    void CyclicWorker<period>::body()
    {
      if (work)
      {
        std::chrono::high_resolution_clock::time_point previous = {};
        std::chrono::high_resolution_clock::time_point now;
        std::chrono::nanoseconds::rep                  elapsed;

        while (alive)
        {
          now     = std::chrono::high_resolution_clock::now();
          elapsed = std::chrono::nanoseconds{now - previous}.count();
          if (elapsed >= period)
          {
            previous = now;
            work();
          }
        }
      }
    }

    template<>
    void CyclicWorker<0>::body()
    {
      if (work)
      {
        while (alive)
        {
          work();
        }
      }
    }
  }
}
#endif
