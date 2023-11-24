/*---author-------------------------------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Nimata

-----licence------------------------------------------------------------------------------------------------------------

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

-----versions-----------------------------------------------------------------------------------------------------------

-----description--------------------------------------------------------------------------------------------------------

-----inclusion guard--------------------------------------------------------------------------------------------------*/
#if not defined(NIMATA_HPP) and defined(__STDCPP_THREADS__)
#define NIMATA_HPP
// --necessary standard libraries---------------------------------------------------------------------------------------
#include <thread>     // for std::thread
#include <mutex>      // for std::mutex, std::lock_guard
#include <atomic>     // for std::atomic
#include <functional> // for std::function
#include <chrono>     // for std::chrono::steady_clock, std::chrono::nanoseconds
#include <utility>    // for std::move
#if defined(NIMATA_LOGGING)
# include <cstdio>    // for std::sprintf
#endif
#include <ostream>    // for std::ostream
#include <iostream>   // for std::clog
// --Nimata library-----------------------------------------------------------------------------------------------------
namespace Nimata
{
  namespace Version
  {
    constexpr long MAJOR  = 000;
    constexpr long MINOR  = 001;
    constexpr long PATCH  = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }

  const unsigned MAX_THREADS = std::thread::hardware_concurrency();

  template<typename T>
  class Queue;

  using Work = std::function<void()>;

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
// --Nimata library: backend forward declaration------------------------------------------------------------------------
  namespace Backend
  {
    class Worker;

    template<Period period>
    class CyclicExecuter;

# if defined(NIMATA_LOGGING)
    void log(const char* caller, const char* message) noexcept
    {
      static std::mutex mtx;
      std::lock_guard<std::mutex> lock{mtx};
      Nimata::Global::log << caller << ": " << message << std::endl;
    }

#   define NIMATA_LOG(...)                    \
      [&](const char* caller) -> void         \
      {                                       \
        static char buffer[255];              \
        sprintf(buffer, __VA_ARGS__);         \
        Nimata::Backend::log(caller, buffer); \
      }(__func__)
# else
#   define NIMATA_LOG(...) ((void)0)
# endif
  }
// --Nimata library: frontend struct and class definitions--------------------------------------------------------------
  template<typename Type>
  class Queue final
  {
  private:
    struct Node { Type data; Node* next; };
  public:
    void push(const Type& data) noexcept
    {
      Node* node = new Node{data, nullptr};

      std::lock_guard<std::mutex> head_lock{head_mtx}, tail_lock{tail_mtx};
      tail = (tail ? tail->next : head) = node;
    }

    Type grab() noexcept
    {
      Type data = head ? std::move(head->data) : Type{};

      drop();

      return data;
    }

    void drop() noexcept
    {
      Node* temp = nullptr;

      {
        std::lock_guard<std::mutex> head_lock{head_mtx};
        if (head)
        {
          temp = head;
          head = head->next;
        }
      }

      {
        std::lock_guard<std::mutex> tail_lock{tail_mtx};
        if (tail == temp)
        {
          tail = nullptr;
        }
      }

      delete temp;
    }

    auto view() const noexcept -> const Type&
    {
      return head ? head->data : Type{};
    }

    operator bool() const noexcept
    { 
      std::lock_guard<std::mutex> head_lock{head_mtx};
      return head != nullptr;
    }

    ~Queue()
    {
      Node* temp;
      while (head)
      {
        temp = head;
        head = head->next;
        delete temp;
      }
    }
  private:
    mutable std::mutex head_mtx;
    mutable std::mutex tail_mtx;
    Node* head = nullptr;
    Node* tail = nullptr;
  };

  class Pool final
  {
  public:
    inline          Pool(signed number_of_threads = MAX_THREADS - 2) noexcept;
    inline         ~Pool() noexcept;
    inline void     push(Work work) noexcept;
    inline void     wait() noexcept;
    inline unsigned size() noexcept;
  private:
    unsigned int     worker_count;
    std::atomic_bool active = {true};
    Backend::Worker* workers;
    Queue<Work>      queue;
    inline void async_assign() noexcept;
    std::thread assignation_thread{async_assign, this};
  };
// --Nimata library: backend  struct and class definitions--------------------------------------------------------------
  namespace Backend
  {
    class Worker final
    {
    public:
      ~Worker() noexcept
      {
        alive = false;
        worker_thread.join();
      }
      
      void task(Work work) noexcept
      {
        this->work     = std::move(work);
        work_available = true;
      }

      bool busy() const noexcept
      {
        return work_available;
      }
    private:
      std::atomic_bool work_available = {false};
      Work             work  = nullptr;
      volatile bool    alive = true;
      std::thread worker_thread{[this]{
        while (alive)
        {
          if (work_available)
          {
            work();
            work_available = false;
          }
          else std::this_thread::yield();
        }
      }};
    };

    template<Period period>
    class CyclicExecuter final
    {
    static_assert(period >= 0, "period must be greater than 0");
    public:
      inline CyclicExecuter(Work work) noexcept;
      inline ~CyclicExecuter() noexcept;
      CyclicExecuter(const CyclicExecuter&) noexcept {}
    private:
      inline void   loop();
      Work          work;
      volatile bool alive = true;
      std::thread   worker_thread{loop, this};
    };
  }
// --Nimata library: frontend definitions-------------------------------------------------------------------------------
  Pool::Pool(signed number_of_threads) noexcept
  {
    if (number_of_threads <= 0)
    {
      number_of_threads += MAX_THREADS;
    }

    if (number_of_threads < 1)
    {
      NIMATA_LOG("%d thread%s is not possible, 1 used instead", number_of_threads, number_of_threads == -1 ? "" : "s");
      number_of_threads = 1;
    }

    worker_count = number_of_threads;
    workers      = new Backend::Worker[worker_count];
    NIMATA_LOG("%u thread%s aquired", worker_count, worker_count == 1 ? "" : "s");
  }

  Pool::~Pool() noexcept
  {
    wait();

    active = false;
    assignation_thread.join();

    delete[] workers;

    NIMATA_LOG("all workers killed");
  }

  void Pool::push(Work work) noexcept
  {
    if (work)
    {
      queue.push(work);
    }
  }

  void Pool::wait() noexcept
  {
    while (queue);

    for (unsigned k = 0; k < worker_count; ++k)
    {
      while (workers[k].busy());
    }

    NIMATA_LOG("all workers finished their work");
  }

  unsigned Pool::size() noexcept
  {
    return worker_count;
  }

  void Pool::async_assign() noexcept
  {
    while (active)
    {
      for (unsigned k = 0; k < worker_count; ++k)
      {
        if (queue and not workers[k].busy())
        {
          workers[k].task(queue.grab());
          NIMATA_LOG("assigned to worker #%02u", k);
        }
      }
    }
  }
  
# undef  NIMATA_CYCLIC
# define NIMATA_CYCLIC(period_us) NIMATA_CYCLIC_PROX(__LINE__, period_us)
# define NIMATA_CYCLIC_PROX(...)  NIMATA_CYCLIC_IMPL(__VA_ARGS__)
# define NIMATA_CYCLIC_IMPL(line, period_us)                                            \
    Nimata::Backend::CyclicExecuter<period_us> cyclic_worker_##line = (Nimata::Work)[&]

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
// --Nimata library: backend definitions--------------------------------------------------------------------------------
  namespace Backend
  {
    template<Period period>
    CyclicExecuter<period>::CyclicExecuter(Work work) noexcept :
      work{work ? NIMATA_LOG("work is null"), nullptr : work}
    {
      NIMATA_LOG("thread spawned");
    }

    template<Period period>
    CyclicExecuter<period>::~CyclicExecuter() noexcept
    {
      alive = false;
      worker_thread.join();
      NIMATA_LOG("thread joined");
    }

    template<Period period>
    void CyclicExecuter<period>::loop()
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
    void CyclicExecuter<0>::loop()
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
