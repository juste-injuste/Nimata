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
#if not defined(NIMATA_HPP)
#define NIMATA_HPP
#if defined(__cplusplus) and (__cplusplus >= 201103L)
#if defined(__STDCPP_THREADS__)
//---necessary libraries------------------------------------------------------------------------------------------------
#include <thread>     // for std::thread, std::this_thread::yield, std::this_thread::sleep_for
#include <mutex>      // for std::mutex, std::lock_guard
#include <atomic>     // for std::atomic
#include <future>     // for std::future, std::promise
#include <functional> // for std::function
#include <queue>      // for std::queue
#include <chrono>     // for std::chrono::*
#include <ostream>    // for std::ostream
#include <iostream>   // for std::clog
//---supplementary libraries--------------------------------------------------------------------------------------------
#if defined(NIMATA_LOGGING)
# include <cstdio>    // for std::sprintf
#endif
//---Nimata library-----------------------------------------------------------------------------------------------------
namespace Nimata
{
  const unsigned MAX_THREADS = std::thread::hardware_concurrency();

  class Pool;

  using Period = std::chrono::nanoseconds::rep;

# define NIMATA_CYCLIC(period_us)

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
    static std::ostream log{std::clog.rdbuf()}; // logging ostream
  }

  namespace Version
  {
    constexpr long MAJOR  = 000;
    constexpr long MINOR  = 001;
    constexpr long PATCH  = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
//---Nimata library: backend--------------------------------------------------------------------------------------------
  namespace _backend
  {
#   define NIMATA_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#   define NIMATA_CLANG_IGNORE(WARNING, ...)          \
      NIMATA_PRAGMA(clang diagnostic push)            \
      NIMATA_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                     \
      NIMATA_PRAGMA(clang diagnostic pop)

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define NIMATA_HOT  NIMATA_CLANG_IGNORE("-Wc++20-extensions", [[likely]])
# else
#   define NIMATA_HOT  [[likely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define NIMATA_HOT  [[likely]]
# else
#   define NIMATA_HOT
# endif

// support from clang 3.9.0 and GCC 7.1 onward
# if defined(__clang__) and ((__clang_major__ > 3) or ((__clang_major__ == 3) and (__clang_minor__ >= 9)))
# if __cplusplus < 201703L
#   define NIMATA_NODISCARD NIMATA_CLANG_IGNORE("-Wc++1z-extensions", [[nodiscard]])
# else
#   define NIMATA_NODISCARD [[nodiscard]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 7)
#   define NIMATA_NODISCARD [[nodiscard]]
# else
#   define NIMATA_NODISCARD
# endif

// support from clang 10.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 10)
# if __cplusplus < 202002L
#   define NIMATA_NODISCARD_REASON(REASON) NIMATA_CLANG_IGNORE("-Wc++20-extensions", [[nodiscard(REASON)]])
# else
#   define NIMATA_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define NIMATA_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# else
#   define NIMATA_NODISCARD_REASON(REASON) NIMATA_NODISCARD
# endif

# if defined(NIMATA_LOGGING)
    static thread_local char _log_buffer[256];
    static std::mutex _log_mtx;
    
#   define NIMATA_LOG(...)                                                   \
      [&](const char* caller){                                               \
        std::sprintf(_backend::_log_buffer, __VA_ARGS__);                    \
        std::lock_guard<std::mutex> _lock{_backend::_log_mtx};               \
        Global::log << caller << ": " << _backend::_log_buffer << std::endl; \
      }(__func__)
# else
#   define NIMATA_LOG(...) void(0)
# endif

    class _worker final
    {
    public:
      ~_worker() noexcept
      {
        alive = false;
        worker_thread.join();
      }

      void task(std::function<void()>&& task) noexcept
      {
        work = std::move(task);
        work_available = true;
      }

      bool busy() const noexcept
      {
        return work_available;
      }

      void loop()
      {
        while (alive) NIMATA_HOT
        {
          if (work_available) NIMATA_HOT
          {
            work();
            work_available = false;
          }
          
          std::this_thread::yield();
        }
      }
    private:
      std::atomic<bool>     work_available = {false};
      std::function<void()> work  = nullptr;
      volatile bool         alive = true;
      std::thread           worker_thread{loop, this};
    };

    template<Period period>
    class _cyclicexecuter final
    {
    static_assert(period >= 0, "NIMATA_CYCLIC: period must be greater than 0");
    public:
      _cyclicexecuter(std::function<void()> task) noexcept :
        work{task ? task : (NIMATA_LOG("task is invalid"), nullptr)}
      {
        NIMATA_LOG("thread spawned");
      }

      _cyclicexecuter(const _cyclicexecuter&) noexcept {}

      ~_cyclicexecuter() noexcept
      {
        alive = false;
        worker_thread.join();
        NIMATA_LOG("thread joined");
      }
    private:
      inline void loop();
      std::function<void()> work;
      volatile bool alive = true;
      std::thread   worker_thread{loop, this};
    };
    
    template<Period period> inline
    void _cyclicexecuter<period>::loop()
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

    template<> inline
    void _cyclicexecuter<0>::loop()
    {
      if (work)
      {
        while (alive)
        {
          work();
        }
      }
    }

    template<typename T>
    using _if_type = typename std::enable_if<not std::is_same<T, void>::value, T>::type;

    template<typename T>
    using _if_void = typename std::enable_if<std::is_same<T, void>::value, T>::type;
  }
//---Nimata library: frontend struct and class definitions--------------------------------------------------------------
  class Pool final
  {
  public:
    inline // constructs pool
    Pool(signed number_of_threads = MAX_THREADS) noexcept;

    inline // waits for all work to be done then join threads
    ~Pool() noexcept;

    // add work that has a return value to queue
    template<typename F, typename... A>
    NIMATA_NODISCARD_REASON("push: consider wrapping in a lambda if you don't use the return value") inline
    auto push(F function, A... arguments) noexcept -> std::future<_backend::_if_type<decltype(function(arguments...))>>;

    // add work that does not have a return value to queue
    template<typename F, typename... A> inline
    auto push(F function, A... arguments) noexcept -> _backend::_if_void<decltype(function(arguments...))>;

    inline // waits for all work to be done
    void wait() const noexcept;

    // query amount of workers
    auto size() const noexcept -> unsigned { return n_workers; }
  private:
    unsigned           n_workers;
    std::atomic<bool>  active = {true};
    _backend::_worker* workers;
    std::mutex         mtx;
    std::queue<std::function<void()>> queue;
    inline void async_assign() noexcept;
    std::thread assignation_thread{async_assign, this};
    static inline unsigned compute_number_of_threads(signed N) noexcept;
  };
//---Nimata library: frontend definitions-------------------------------------------------------------------------------
  Pool::Pool(signed N) noexcept :
    n_workers{compute_number_of_threads(N)},
    workers{new _backend::_worker[n_workers]}
  {
    NIMATA_LOG("%u thread%s aquired", n_workers, n_workers == 1 ? "" : "s");
  }

  Pool::~Pool() noexcept
  {
    wait();

    active = false;
    assignation_thread.join();

    delete[] workers;

    NIMATA_LOG("all workers killed");
  }
  
  template<typename F, typename... A>
  auto Pool::push(F function, A... arguments) noexcept -> std::future<_backend::_if_type<decltype(function(arguments...))>>
  {
    using R = decltype(function(arguments...));

    std::future<R> future;

    if (function) NIMATA_HOT
    {
      std::promise<R>* promise = new std::promise<R>;
      
      future = promise->get_future();
      
      {
        std::lock_guard<std::mutex> lock{mtx};
        queue.push([=]{
          promise->set_value(function(arguments...));
          delete promise;
        });
      }

      NIMATA_LOG("pushed a task with return value");
    }
    else NIMATA_LOG("invalid task pushed");

    return future;
  }
  
  template<typename F, typename... A>
  auto Pool::push(F function, A... arguments) noexcept -> _backend::_if_void<decltype(function(arguments...))>
  {
    if (function) NIMATA_HOT
    {      
      {
        std::lock_guard<std::mutex> lock{mtx};
        queue.push([=]{
          function(arguments...);
        });
      }

      NIMATA_LOG("pushed a task with no return value");
    }
    else NIMATA_LOG("invalid task pushed");

    return;
  }

  void Pool::wait() const noexcept
  {
    while (queue.empty() == false)
    {
      std::this_thread::sleep_for(std::chrono::microseconds{1});
    };

    for (unsigned k = 0; k < n_workers; ++k)
    {
      while (workers[k].busy())
      {
        std::this_thread::sleep_for(std::chrono::microseconds{1});
      }
    }

    NIMATA_LOG("all threads finished their work");
  }

  void Pool::async_assign() noexcept
  {
    while (active)
    {
      for (unsigned k = 0; k < n_workers; ++k)
      {
        if (workers[k].busy() == false)
        {
          std::lock_guard<std::mutex> lock{mtx};
          if (queue.empty() == false)
          {
            workers[k].task(std::move(queue.front()));
            queue.pop();
            NIMATA_LOG("assigned to worker thread #%02u", k);
          }
        }
      }
    }
  }

  unsigned Pool::compute_number_of_threads(signed N) noexcept
  {
    if (N <= 0)
    {
      N += MAX_THREADS;
    }
    
    if (N < 1)
    {
      NIMATA_LOG("%d threads is not possible, 1 used instead", N);
      N = 1;
    }

    if (N > static_cast<signed>(MAX_THREADS - 2))
    {
      NIMATA_LOG("MAX_THREADS - 2 is the recommended maximum amount of threads, %d used", N);
    }
    
    return static_cast<unsigned>(N);
  }
  
# undef  NIMATA_CYCLIC
# define NIMATA_CYCLIC(period_us) NIMATA_CYCLIC_PROX(__LINE__, period_us)
# define NIMATA_CYCLIC_PROX(...)  NIMATA_CYCLIC_IMPL(__VA_ARGS__)
# define NIMATA_CYCLIC_IMPL(line, period_us)                                                       \
    Nimata::_backend::_cyclicexecuter<period_us> cyclic_worker_##line = (std::function<void()>)[&]

  inline namespace Literals
  {
    constexpr Period operator ""_mHz(long double frequency)
    {
      return static_cast<Period>(1000000000000/frequency);
    }

    constexpr Period operator ""_mHz(unsigned long long frequency)
    {
      return 1000000000000/frequency;
    }

    constexpr Period operator ""_Hz(long double frequency)
    {
      return static_cast<Period>(1000000000/frequency);
    }

    constexpr Period operator ""_Hz(unsigned long long frequency)
    {
      return 1000000000/frequency;
    }
    
    constexpr Period operator ""_kHz(long double frequency)
    {
      return static_cast<Period>(1000000/frequency);
    }

    constexpr Period operator ""_kHz(unsigned long long frequency)
    {
      return 1000000/frequency;
    }
  }
//----------------------------------------------------------------------------------------------------------------------
# undef NIMATA_PRAGMA
# undef NIMATA_CLANG_IGNORE
# undef NIMATA_HOT
# undef NIMATA_NODISCARD
# undef NIMATA_NODISCARD_REASON
# undef NIMATA_LOG
}
#else
#error "Nimata: Concurrent threads are required"
#endif
#else
#error "Nimata: Support for ISO C++11 is required"
#endif
#endif