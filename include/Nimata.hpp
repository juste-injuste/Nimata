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
#include <thread>             // for std::thread
#include <atomic>             // for std::atomic
#include <mutex>              // for std::mutex, std::unique_lock
#include <condition_variable> // for std::conditional_variable
#include <array>              // for std::array
#include <ostream>            // for std::ostream
#include <deque>
#include <functional>
#include <chrono>             // for std::chrono::high_resolution_clock, std::chrono::nanoseconds
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

  template<size_t N>
  class Pool;

# define NIMATA_ASYNC(period_us)
  
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
// --Nimata library: backend forward declaration----------------------------------------------------
  namespace Backend
  {
    class Worker;

    template<Period P>
    class AsyncWorker;
  }
// --Nimata library: frontend struct and class definitions------------------------------------------
  template<size_t N>
  class Pool final
  {
  static_assert(N > 0, "N in Pool<N> must be greater than 0");
  public:
    inline Pool() noexcept;
    inline ~Pool() noexcept;
    inline void do_work(Work work) noexcept;
    inline void join_all() noexcept;
    Backend::Worker workers_[N];
  private:
    std::condition_variable conditional_;
    std::mutex queue_mutex;
    std::deque<Work> work_queue;
  };
// --Nimata library: backend  struct and class definitions------------------------------------------
  namespace Backend
  {
    class Worker final
    {
    public:
      inline explicit Worker() noexcept;
      inline ~Worker() noexcept;
      inline void body() noexcept;
      inline void assign_work(Work work) noexcept;
      inline bool vitality() { return alive; }
      inline void kill() noexcept;
      inline bool busy() const noexcept;
    private:
      std::condition_variable* notifier;
      std::thread thread;
      std::mutex mtx;
      bool working;
      bool alive;
      Work work;
    };

    template<Period P>
    class AsyncWorker final
    {
    static_assert(P >= 0, "period must be a positive or zero number");
    public:
      AsyncWorker(Work work) noexcept;
      AsyncWorker(const AsyncWorker&) noexcept {}
      ~AsyncWorker() noexcept;
    private:
      std::thread async_work;
      bool        do_work = true;
    };
  }
// --Nimata library: frontend definitions-----------------------------------------------------------
  template<size_t N>
  Pool<N>::Pool() noexcept :
    workers_()
  {}

  template<size_t N>
  Pool<N>::~Pool() noexcept
  {
    join_all();
  }

  template<size_t N>
  void Pool<N>::do_work(Work work) noexcept
  {
    {
      std::unique_lock lock(queue_mutex);
      work_queue.push_back(work);
    }

    // for (Backend::Worker& worker : workers_)
    // {
    //   if (worker.busy() == false)
    //   {
    //     worker.assign_work(work);
    //     return;
    //   }
    // }
  }

  template<size_t N>
  void Pool<N>::join_all() noexcept
  {
    for (Backend::Worker& worker : workers_)
    {
      if (worker.vitality())
      {
        worker.kill();
      }
    }
  }
  
# undef  NIMATA_ASYNC
# define NIMATA_ASYNC_IMPL(period_us, line)                                                   \
    Nimata::Backend::AsyncWorker<period_us> async_worker_##line = (Nimata::Work)[&]() -> void
# define NIMATA_ASYNC_PROX(...)  NIMATA_ASYNC_IMPL(__VA_ARGS__)
# define NIMATA_ASYNC(period_us) NIMATA_ASYNC_PROX(period_us, __LINE__)

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
    Worker::Worker() noexcept :
      thread(body, this),
      working(false),
      alive(true)
    {}

    Worker::~Worker() noexcept
    {
      if (alive)
      {
        kill();
      }
    }

    void Worker::body() noexcept
    {
    loop:
      working = false;

      std::unique_lock<std::mutex> lock{mtx};
      notifier->wait(lock, [this]{return work != nullptr;});

      working = true;

      if (alive == false)
        return;

      work();

      goto loop;
    }

    void Worker::assign_work(Work work) noexcept
    {
      work = work;

      std::unique_lock<std::mutex> lock{mtx};
      notifier->notify_one();
    }

    void Worker::kill() noexcept
    {
      alive = false;
      
      if (thread.joinable())
      {
        // std::cout << "  joining thread "<< thread.get_id() << "...\n";
        notifier->notify_one();
        thread.join();
        // std::cout << "  joined...\n";
      }
      else {}//std::cout << " not joinable\n";
    }

    bool Worker::busy() const noexcept
    {
      return working;
    }

    template<Period P>
    AsyncWorker<P>::AsyncWorker(Work work) noexcept :
      async_work{[&]{
        while (do_work)
        {
          static std::chrono::high_resolution_clock::time_point previous = {};
          auto now     = std::chrono::high_resolution_clock::now();
          auto elapsed = std::chrono::nanoseconds(now - previous).count();
          
          if (elapsed >= P)
          {
            previous = now;
            work();
          }
        }
      }}
    {}

    template<>
    AsyncWorker<0>::AsyncWorker(Work work) noexcept :
      async_work{[&]{
        while (do_work)
        {
          work();
        }
      }}
    {}

    template<Period P>
    AsyncWorker<P>::~AsyncWorker() noexcept
    {
      do_work = false;
      async_work.join();
    }
  }
}
#endif
