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

-----_versions-----------------------------------------------------------------------------------------------------------

-----description--------------------------------------------------------------------------------------------------------

mtz::Pool;

-----inclusion guard--------------------------------------------------------------------------------------------------*/
#ifndef _mtz_hpp
#define _mtz_hpp
#if defined(__cplusplus) and (__cplusplus >= 201103L)
#if defined(__STDCPP_THREADS__)
//---necessary standard libraries---------------------------------------------------------------------------------------
#include <thread>     // for std::thread, std::this_thread::yield, std::this_thread::sleep_for
#include <mutex>      // for std::mutex, std::lock_guard
#include <atomic>     // for std::atomic
#include <future>     // for std::future, std::promise
#include <functional> // for std::function
#include <queue>      // for std::queue
#include <chrono>     // for std::chrono::*
#include <ostream>    // for std::ostream
#include <iostream>   // for std::clog
#include <memory>     // for std::unique_ptr
//---conditionally necessary standard libraries-------------------------------------------------------------------------
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
    constexpr
    Period operator""_mHz(long double frequency);

    constexpr
    Period operator""_mHz(unsigned long long frequency);

    constexpr
    Period operator""_Hz(long double frequency);

    constexpr
    Period operator""_Hz(unsigned long long frequency);

    constexpr
    Period operator""_kHz(long double frequency);

    constexpr
    Period operator""_kHz(unsigned long long frequency);
  }

  namespace _io
  {
    static std::ostream log(std::clog.rdbuf()); // logging ostream
  }

  namespace _version
  {
    constexpr long MAJOR  = 000;
    constexpr long MINOR  = 001;
    constexpr long PATCH  = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
//---Nimata library: backend--------------------------------------------------------------------------------------------
  namespace _backend
  {
# if defined(__clang__)
#   define _mtz_impl_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#   define _mtz_impl_IGNORE(WARNING, ...)          \
      _mtz_impl_PRAGMA(clang diagnostic push)            \
      _mtz_impl_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                     \
      _mtz_impl_PRAGMA(clang diagnostic pop)
# endif

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define _mtz_impl_HOT _mtz_impl_IGNORE("-Wc++20-extensions", [[likely]])
# else
#   define _mtz_impl_HOT [[likely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _mtz_impl_HOT [[likely]]
# else
#   define _mtz_impl_HOT
# endif

// support from clang 3.9.0 and GCC 5.1 onward
# if defined(__clang__)
#   define _mtz_impl_NODISCARD __attribute__((warn_unused_result))
# elif defined(__GNUC__)
#   define _mtz_impl_NODISCARD __attribute__((warn_unused_result))
# else
#   define _mtz_impl_NODISCARD
# endif

// support from clang 10.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 10)
# if __cplusplus < 202002L
#   define _mtz_impl_NODISCARD_REASON(REASON) _mtz_impl_IGNORE("-Wc++20-extensions", [[nodiscard(REASON)]])
# else
#   define _mtz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _mtz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# else
#   define _mtz_impl_NODISCARD_REASON(REASON) _mtz_impl_NODISCARD
# endif

# if defined(NIMATA_LOGGING)
    static thread_local char _log_buffer[256];
    static std::mutex _log_mtx;
    
#   define _mtz_impl_LOG(...)                                                   \
      [&](const char* caller){                                               \
        std::sprintf(_backend::_log_buffer, __VA_ARGS__);                    \
        std::lock_guard<std::mutex> _lock{_backend::_log_mtx};               \
        _io::log << caller << ": " << _backend::_log_buffer << std::endl; \
      }(__func__)
# else
#   define _mtz_impl_LOG(...) void(0)
# endif

    using _work_t = std::function<void()>;
    // using _work_t = void(*)();
    
    // template <typename L, typename = void>
    // struct _is_work_t final :
    //   public std::false_type
    // {};

    // template <typename L>
    // struct _is_work_t<L, decltype(static_cast<_work_t>(std::declval<L>()), void())> final :
    //   public std::true_type
    // {};

    // template<typename L, typename T>
    // using _if_is_work_t = typename std::enable_if<_is_work_t<L>::value == true, T>::type;

    // template<typename L, typename T>
    // using _if_no_work_t = typename std::enable_if<_is_work_t<L>::value != true, T>::type;

    // template<typename L>
    // inline
    // auto _as_work_t(L lambda) noexcept -> _if_is_work_t<L, _work_t>
    // {
    //   std::cout << "_action!\n";

    //   return static_cast<_work_t>(lambda);
    // }

    // template<typename L>
    // inline
    // auto _as_work_t(L&& lambda) noexcept -> _if_no_work_t<L, _work_t>
    // {
    //   // std::cout << "generic!\n";

    //   static auto action = std::move(lambda);

    //   return []{ action(); };
    // }

    // inline
    // _work_t _as_work_t(std::function<void()>&& function) noexcept
    // {
    //   // std::cout << "function, ";

    //   if (function == nullptr)
    //   {
    //     return []{};
    //   }
      
    //   return _as_work_t([function]{ function(); });
    // }

    class _worker final
    {
    public:
      ~_worker() noexcept
      {
        _alive = false;
        _worker_thread.join();
      }

      void _task(_work_t task) noexcept
      {
        _work = task;
        _work_available = true;
      }

      bool _busy() const noexcept
      {
        return _work_available;
      }
    private:
      void _loop()
      {
        while (_alive) _mtz_impl_HOT
        {
          if (_work_available) _mtz_impl_HOT
          {
            _work();
            _work_available = false;
          }
          
          std::this_thread::yield();
        }
      }
      volatile bool     _alive          = true;
      _work_t           _work           = nullptr;
      std::atomic<bool> _work_available = {false};
      std::thread       _worker_thread{_loop, this};
    };

    template<Period period>
    class _cyclicexecuter final
    {
    static_assert(period >= 0, "NIMATA_CYCLIC: period must be greater than 0");
    public:
      _cyclicexecuter(std::function<void()> task) noexcept :
        _work(task ? task : (_mtz_impl_LOG("task is invalid"), nullptr))
      {
        _mtz_impl_LOG("thread spawned");
      }

      _cyclicexecuter(const _cyclicexecuter&) noexcept {}

      ~_cyclicexecuter() noexcept
      {
        _alive = false;
        _worker_thread.join();
        _mtz_impl_LOG("thread joined");
      }
    private:
      inline void _loop();
      volatile bool         _alive = true;
      std::function<void()> _work;
      std::thread           _worker_thread{_loop, this};
    };
    
    template<Period period>
    inline
    void _cyclicexecuter<period>::_loop()
    {
      if (_work)
      {
        std::chrono::high_resolution_clock::time_point previous = {};
        std::chrono::high_resolution_clock::time_point now;
        std::chrono::nanoseconds::rep                  elapsed;

        while (_alive)
        {
          now     = std::chrono::high_resolution_clock::now();
          elapsed = std::chrono::nanoseconds{now - previous}.count();
          if (elapsed >= period)
          {
            previous = now;
            _work();
          }
        }
      }
    }

    template<>
    inline
    void _cyclicexecuter<0>::_loop()
    {
      if (_work)
      {
        while (_alive)
        {
          _work();
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

    template<typename F, typename... A>
    _mtz_impl_NODISCARD_REASON("push: wrap in a lambda if you don't use the return value")
    inline // add work that has a return value to queue
    auto push(F function, A... arguments) noexcept -> std::future<_backend::_if_type<decltype(function(arguments...))>>;

    template<typename F, typename... A>
    inline // add work that does not have a return value to queue
    auto push(F function, A... arguments) noexcept -> _backend::_if_void<decltype(function(arguments...))>;

    inline // waits for all work to be done
    void wait() const noexcept;

    // query amount of workers
    auto size() const noexcept -> unsigned { return _n_workers; }

    //
    void work() noexcept { _active = true;  }

    //
    void stop()  noexcept { _active = false; }
  private:
    static inline unsigned _compute_number_of_threads(signed N) noexcept;
    inline void _async_assign() noexcept;
    std::atomic<bool>                    _alive  = {true};
    std::atomic<bool>                    _active = {true};
    unsigned                             _n_workers;
    std::unique_ptr<_backend::_worker[]> _workers;
    std::mutex                           _queue_mtx;
    std::queue<_backend::_work_t>        _queue;
    std::thread                          _assignation_thread{_async_assign, this};
  };
//---Nimata library: frontend definitions-------------------------------------------------------------------------------
  Pool::Pool(signed N) noexcept :
    _n_workers(_compute_number_of_threads(N)),
    _workers(new _backend::_worker[_n_workers])
  {
    _mtz_impl_LOG("%u thread%s aquired", _n_workers, _n_workers == 1 ? "" : "s");
  }

  Pool::~Pool() noexcept
  {
    wait();

    _alive = false;
    _assignation_thread.join();

    _mtz_impl_LOG("all workers killed");
  }
  
  template<typename F, typename... A>
  auto Pool::push(F function, A... arguments) noexcept -> std::future<_backend::_if_type<decltype(function(arguments...))>>
  {
    using R = decltype(function(arguments...));

    std::future<R> future;

    if (function) _mtz_impl_HOT
    {
      auto promise = new std::promise<R>;
      
      future = promise->get_future();
      
      std::lock_guard<std::mutex>{_queue_mtx}, _queue.push(
        // _backend::_as_work_t
        (
          [=]{ std::unique_ptr<std::promise<R>>(promise)->set_value(function(arguments...)); }
        )
      );

      _mtz_impl_LOG("pushed a task with return value");
    }
    else _mtz_impl_LOG("null task pushed");

    return future;
  }
  
  template<typename F, typename... A>
  auto Pool::push(F function, A... arguments) noexcept -> _backend::_if_void<decltype(function(arguments...))>
  {
    if (function) _mtz_impl_HOT
    {
      std::lock_guard<std::mutex>{_queue_mtx}, _queue.push(
        // _backend::_as_work_t
        (
          [=]{ function(arguments...); }
        )
      );

      _mtz_impl_LOG("pushed a task with no return value");
    }
    else _mtz_impl_LOG("null task pushed");

    return;
  }

  void Pool::wait() const noexcept
  {
    if (_active == true)
    {
      while (not _queue.empty())
      {
        std::this_thread::sleep_for(std::chrono::microseconds{1});
      };

      for (unsigned k = 0; k < _n_workers; ++k)
      {
        while (_workers[k]._busy())
        {
          std::this_thread::sleep_for(std::chrono::microseconds{1});
        }
      }

      _mtz_impl_LOG("all threads finished their work");
    }
  }

  void Pool::_async_assign() noexcept
  {
    while (_alive)
    {
      if (not _active) continue;

      for (unsigned k = 0; k < _n_workers; ++k)
      {
        if (_workers[k]._busy()) continue;

        std::lock_guard<std::mutex> lock{_queue_mtx};
        if (_queue.empty() == false)
        {
          _workers[k]._task(std::move(_queue.front()));
          _queue.pop();
          _mtz_impl_LOG("assigned to worker thread #%02u", k);
        }
      }
    }
  }

  unsigned Pool::_compute_number_of_threads(signed N) noexcept
  {
    if (N <= 0)
    {
      N += MAX_THREADS;
    }
    
    if (N < 1)
    {
      _mtz_impl_LOG("%d threads is not possible, 1 used instead", N);
      N = 1;
    }

    if (N > static_cast<signed>(MAX_THREADS - 2))
    {
      _mtz_impl_LOG("MAX_THREADS - 2 is the recommended maximum amount of threads, %d used", N);
    }
    
    return static_cast<unsigned>(N);
  }
  
# undef  NIMATA_CYCLIC
# define NIMATA_CYCLIC(period_us)   _mtz_impl_CYCLIC_PROX(__LINE__, period_us)
# define _mtz_impl_CYCLIC_PROX(...) _mtz_impl_CYCLIC_IMPL(__VA_ARGS__)
# define _mtz_impl_CYCLIC_IMPL(line, period_us)                                                   \
    Nimata::_backend::_cyclicexecuter<period_us> cyclic_worker_##line = (std::function<void()>)[&]

  inline namespace Literals
  {
    constexpr
    Period operator ""_mHz(long double frequency)
    {
      return static_cast<Period>(1000000000000/frequency);
    }

    constexpr
    Period operator ""_mHz(unsigned long long frequency)
    {
      return 1000000000000/frequency;
    }

    constexpr
    Period operator ""_Hz(long double frequency)
    {
      return static_cast<Period>(1000000000/frequency);
    }

    constexpr
    Period operator ""_Hz(unsigned long long frequency)
    {
      return 1000000000/frequency;
    }
    
    constexpr
    Period operator ""_kHz(long double frequency)
    {
      return static_cast<Period>(1000000/frequency);
    }

    constexpr
    Period operator ""_kHz(unsigned long long frequency)
    {
      return 1000000/frequency;
    }
  }
//----------------------------------------------------------------------------------------------------------------------
# undef _mtz_impl_PRAGMA
# undef _mtz_impl_IGNORE
# undef _mtz_impl_HOT
# undef _mtz_impl_NODISCARD
# undef _mtz_impl_NODISCARD_REASON
# undef _mtz_impl_LOG
}
#else
#error "Nimata: Concurrent threads are required"
#endif
#else
#error "Nimata: Support for ISO C++11 is required"
#endif
#endif