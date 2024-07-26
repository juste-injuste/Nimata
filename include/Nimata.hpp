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

-----inclusion guard--------------------------------------------------------------------------------------------------*/
#ifndef _nimata_hpp
#define _nimata_hpp
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
#include <utility>    // for std::declval
//---conditionally necessary standard libraries-------------------------------------------------------------------------
#if defined(MTZ_DEBUGGING)
# include <cstdio>    // for std::sprintf
#endif
//---Nimata library-----------------------------------------------------------------------------------------------------
namespace mtz
{
  const unsigned MAX_THREADS = std::thread::hardware_concurrency();

  class Pool;

// # define MTZ_PARFOR(POOL, ...)
// # define MTZ_PARFOR_IDX

# define MTZ_CYCLIC(NS)

  inline namespace _literals
  {
    constexpr auto operator""_mHz(long double frequency)        -> std::chrono::nanoseconds::rep;
    constexpr auto operator""_mHz(unsigned long long frequency) -> std::chrono::nanoseconds::rep;
    constexpr auto operator""_Hz(long double frequency)         -> std::chrono::nanoseconds::rep;
    constexpr auto operator""_Hz(unsigned long long frequency)  -> std::chrono::nanoseconds::rep;
    constexpr auto operator""_kHz(long double frequency)        -> std::chrono::nanoseconds::rep;
    constexpr auto operator""_kHz(unsigned long long frequency) -> std::chrono::nanoseconds::rep;
  }

  namespace _io
  {
    static std::ostream dbg(std::clog.rdbuf()); // debugging
  }

  namespace _version
  {
    constexpr long MAJOR  = 000;
    constexpr long MINOR  = 001;
    constexpr long PATCH  = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
//---Nimata library: backend--------------------------------------------------------------------------------------------
  namespace _impl
  {
# if defined(__clang__)
#   define _mtz_impl_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#   define _mtz_impl_CLANG_IGNORE(WARNING, ...)          \
      _mtz_impl_PRAGMA(clang diagnostic push)            \
      _mtz_impl_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                        \
      _mtz_impl_PRAGMA(clang diagnostic pop)
# endif

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define _mtz_impl_LIKELY _mtz_impl_CLANG_IGNORE("-Wc++20-extensions", [[likely]])
# else
#   define _mtz_impl_LIKELY [[likely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _mtz_impl_LIKELY [[likely]]
# else
#   define _mtz_impl_LIKELY
# endif

// support from clang 3.9.0 and GCC 4.7.3 onward
# if defined(__clang__)
#   define _mtz_impl_NODISCARD           __attribute__((warn_unused_result))
#   define _mtz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _mtz_impl_LIKELY
# elif defined(__GNUC__)
#   define _mtz_impl_NODISCARD           __attribute__((warn_unused_result))
#   define _mtz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _mtz_impl_LIKELY
# else
#   define _mtz_impl_NODISCARD
#   define _mtz_impl_EXPECTED(CONDITION) (CONDITION) _mtz_impl_LIKELY
# endif

// support from clang 10.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 10)
# if __cplusplus < 202002L
#   define _mtz_impl_NODISCARD_REASON(REASON) _mtz_impl_CLANG_IGNORE("-Wc++20-extensions", [[nodiscard(REASON)]])
# else
#   define _mtz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _mtz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# else
#   define _mtz_impl_NODISCARD_REASON(REASON) _mtz_impl_NODISCARD
# endif

# if defined(MTZ_DEBUGGING)
    static thread_local char _dbg_buf[128];
    static std::mutex _dbg_mtx;
    
#   define _mtz_impl_DEBUG(...)                                      \
      [&](const char* const caller_){                                \
        std::sprintf(_impl::_dbg_buf, __VA_ARGS__);                  \
        std::lock_guard<std::mutex> _lock{_impl::_dbg_mtx};          \
        _io::dbg << caller_ << ": " << _impl::_dbg_buf << std::endl; \
      }(__func__)
# else
#   define _mtz_impl_DEBUG(...) void(0)
# endif

# if __cplusplus >= 201402L
#   define _mtz_impl_CONSTEXPR_CPP14 constexpr
# else
#   define _mtz_impl_CONSTEXPR_CPP14
# endif

    inline _mtz_impl_CONSTEXPR_CPP14
    auto _compute_number_of_threads(signed N_) noexcept -> unsigned
    {
      if (N_ <= 0)
      {
        N_ += MAX_THREADS;
      }
      
      if (N_ < 1)
      {
        _mtz_impl_DEBUG("%d threads is not possible, 1 used instead", N_);
        N_ = 1;
      }

      if (N_ > static_cast<signed>(MAX_THREADS - 2))
      {
        _mtz_impl_DEBUG("MAX_THREADS - 2 is the recommended maximum amount of threads, %d used", N_);
      }
      
      return static_cast<unsigned>(N_);
    }

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

      void _task(_work_t task_) noexcept
      {
        _work = task_;
        _work_available = true;
      }

      bool _busy() const noexcept
      {
        return _work_available;
      }

    private:
      void _loop()
      {
        while _mtz_impl_EXPECTED(_alive)
        {
          if _mtz_impl_EXPECTED(_work_available)
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

    template<std::chrono::nanoseconds::rep period>
    class _cyclic final
    {
      static_assert(period >= 0, "MTZ_CYCLIC: period must be greater than 0");
    public:
      _cyclic(_work_t task) noexcept :
        _work(task ? task : (_mtz_impl_DEBUG("task is invalid"), nullptr))
      {
        _mtz_impl_DEBUG("thread spawned");
      }

      _cyclic(const _cyclic&) noexcept {}

      ~_cyclic() noexcept
      {
        _alive = false;
        _worker_thread.join();
        _mtz_impl_DEBUG("thread joined");
      }
    private:
      inline void _loop();
      volatile bool _alive = true;
      _work_t       _work;
      std::thread   _worker_thread{_loop, this};
    };
    
    template<std::chrono::nanoseconds::rep period>
    void _cyclic<period>::_loop()
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
    void _cyclic<0>::_loop()
    {
      if (_work)
      {
        while (_alive)
        {
          _work();
        }
      }
    }

    template<typename T, typename R = void>
    using _if_boollike = typename std::enable_if<std::is_convertible<T, bool>::value == true, R>::type;

    template<typename T, typename R = void>
    using _no_boollike = typename std::enable_if<std::is_convertible<T, bool>::value != true, R>::type;

    template<typename F>
    constexpr
    auto _is_valid(const F& function) noexcept -> _if_boollike<F, bool>
    {
      return static_cast<bool>(function);
    }

    template<typename F>
    constexpr
    auto _is_valid(const F&) noexcept -> _no_boollike<F, bool>
    {
      return true;
    }

    template<typename T>
    using _if_type = typename std::enable_if<not std::is_same<T, void>::value, T>::type;

    template<typename F, typename... A>
    using _void = typename std::enable_if<std::is_same<decltype(std::declval<F>()(A()...)), void>::value, void>::type;

    template<typename F, typename... A>
    using _future = std::future<_impl::_if_type<decltype(std::declval<F>()(A()...))>>;

    // class _parfor final
    // {
    // public:
    //   struct idx;

    //   inline
    //   _parfor(const size_t from, const size_t past) noexcept;

    //   inline
    //   _parfor(const size_t size) noexcept;

    //   inline
    //   auto operator=(_work_t&& body) noexcept -> _parfor*;

    //   friend
    //   void operator<<=(Pool& pool, _parfor* const parfor) noexcept;

    // private:
    //   _work_t      _work = nullptr;
    //   const size_t _from = 0;
    //   const size_t _past = {};
    // };

    // struct _parfor::idx final
    // {
    //   inline
    //   operator size_t() const noexcept;
    // };

    // static thread_local size_t _parfor_idx;

    template<typename T>
    struct _is_iterable final
    {
    private:
      template<typename T_>
      static
      auto _impl(int) -> decltype
      (
        void(begin(T_()) != end(T_())),
        void(++begin(T_())),
        void(*begin(T_())),
        std::true_type{}
      );

      template<typename T_>
      static
      auto _impl(...) -> std::false_type;

    public:
      static constexpr bool value = decltype(_impl<T>(0)){};
    };

    template<typename T, bool = _is_iterable<T>::value>
    struct _iter_type;
    
    template<typename T>
    struct _iter_type<T, true> final
    {
      using iter = decltype(begin(T()));
      using type = decltype(*begin(T()));
      
      static constexpr
      type _get(iter data) noexcept
      {
        return *data;
      }
    };
    
    template<typename T>
    struct _iter_type<T, false> final
    {
      using iter = T;
      using type = T;

      static constexpr
      type _get(iter data) noexcept
      {
        return data;
      }
    };

    template<typename... types>
    auto _parfor_iter(types...) -> typename _iter_type<typename std::common_type<types...>::type>::iter;

    template<typename... types>
    auto _parfor_type(types...) -> typename _iter_type<typename std::common_type<types...>::type>::type;

    template<typename type>
    struct _parfor;
  }
//---Nimata library: frontend struct and class definitions--------------------------------------------------------------
  class Pool final
  {
  public:
    inline // constructs pool
    Pool(signed number_of_threads = MAX_THREADS) noexcept;

    template<typename F, typename... A>
    _mtz_impl_NODISCARD_REASON("push: wrap in a lambda if you don't use the return value.")
    inline // add work that has a return value to queue
    auto push(F function, A... arguments) noexcept -> _impl::_future<F, A...>;

    template<typename F, typename... A>
    inline // add work that does not have a return value to queue
    auto push(F function, A... arguments) noexcept -> _impl::_void<F, A...>;

    inline // waits for all work to be done
    void wait() const noexcept;

    inline // query amount of workers
    auto size() const noexcept -> unsigned;

    //
    inline
    auto parfor(const size_t size_) noexcept -> _impl::_parfor<size_t>;
    
    //
    template<typename type>
    inline
    auto parfor(type from_, type past_) noexcept -> _impl::_parfor<type>;
    
    //
    template<typename iterable>
    inline
    auto parfor(iterable& iterable_) noexcept -> _impl::_parfor<iterable>;

    // variable to use as an index when using 'parfor'
    // static index idx;

    //
    void work() noexcept { _active = true;  }

    //
    void stop() noexcept { _active = false; }

    inline // waits for all work to be done then join threads
    ~Pool() noexcept;
    
  private:
    template<typename>
    friend struct _impl::_parfor;
    inline void _async_assign() noexcept;
    std::atomic<bool>                 _alive  = {true};
    std::atomic<bool>                 _active = {true};
    unsigned                          _size;
    std::unique_ptr<_impl::_worker[]> _workers;
    std::mutex                        _queue_mtx;
    std::queue<_impl::_work_t>        _queue;
    std::thread                       _assignation_thread{_async_assign, this};
  // public:
  //   auto push() noexcept -> Pool& { return *this; }
  };
//---Nimata library: backend--------------------------------------------------------------------------------------------
  namespace _impl
  {
    template<typename T>
    struct _parfor final
    {
      using iterator = decltype(_parfor_iter(std::declval<T&>()));

      _parfor(Pool* const pool_, iterator from_, iterator past_) noexcept :
        _pool(pool_),
        _from(from_),
        _past(past_)
      {}

      template<typename F>
      void operator=(F&& body) noexcept
      {
        {
          std::lock_guard<std::mutex> pool_queue_lock{_pool->_queue_mtx};

          for (auto k = _from; k != _past; ++k)
          {
            _pool->_queue.push([=]{ body(_iter_type<T>::_get(k)); });
          }
        }

        _pool->wait();
      }

      Pool* const _pool;
      iterator _from;
      const iterator _past;
    };

    // _parfor::_parfor(const size_t from_, const size_t past_) noexcept :
    //   _from(from_),
    //   _past(past_)
    // {}

    // _parfor::_parfor(const size_t size_) noexcept :
    //   _past(size_)
    // {}

    // auto _parfor::operator=(_work_t&& body_) noexcept -> _parfor*
    // {
    //   _work = std::move(body_);

    //   return this;
    // }

    // inline
    // void operator<<=(Pool& pool_, _parfor* const parfor) noexcept
    // {
    //   for (size_t k = parfor->_from; k < parfor->_past; ++k)
    //   {
    //     pool_.push(
    //       [&](const size_t idx)
    //       {
    //         _parfor_idx = idx;
    //         parfor->_work();
    //       },
    //       k
    //     );
    //   }

    //   pool_.wait();
    // }

    // _parfor::idx::operator size_t() const noexcept
    // {
    //   return _parfor_idx;
    // }
  }
//---Nimata library: frontend definitions-------------------------------------------------------------------------------
  Pool::Pool(signed N_) noexcept :
    _size(_impl::_compute_number_of_threads(N_)),
    _workers(new _impl::_worker[_size])
  {
    _mtz_impl_DEBUG("%u thread%s aquired.", _size, _size == 1 ? "" : "s");
  }
  
  template<typename F, typename... A>
  auto Pool::push(F function_, A... arguments_) noexcept -> _impl::_future<F, A...>
  {
    using R = decltype(function_(arguments_...));

    std::future<R> future;

    if _mtz_impl_EXPECTED(function_)
    {
      auto promise = new std::promise<R>;
      
      future = promise->get_future();
      
      std::lock_guard<std::mutex>{_queue_mtx}, _queue.push(
        // _impl::_as_work_t
        (
          [=]{ std::unique_ptr<std::promise<R>>(promise)->set_value(function_(arguments_...)); }
        )
      );

      _mtz_impl_DEBUG("pushed a task with return value.");
    }
    else _mtz_impl_DEBUG("null task pushed.");

    return future;
  }
  
  template<typename F, typename... A>
  auto Pool::push(F function_, A... arguments_) noexcept -> _impl::_void<F, A...>
  {
    if _mtz_impl_EXPECTED(_impl::_is_valid(function_))
    {
      std::lock_guard<std::mutex>{_queue_mtx}, _queue.push(
        // _impl::_as_work_t
        (
          [=]{ function_(arguments_...); }
        )
      );

      _mtz_impl_DEBUG("pushed a task with no return value.");
    }
    else _mtz_impl_DEBUG("null task pushed.");

    return;
  }

  void Pool::wait() const noexcept
  {
    if (_active == true)
    {
      while (_queue.empty() == false)
      {
        std::this_thread::sleep_for(std::chrono::nanoseconds{1});
      };

      for (unsigned k = 0; k < _size; ++k)
      {
        while (_workers[k]._busy())
        {
          std::this_thread::sleep_for(std::chrono::nanoseconds{1});
        }
      }

      _mtz_impl_DEBUG("all threads finished their work.");
    }
  }

  auto Pool::size() const noexcept -> unsigned
  {
    return _size;
  }
  
  template<typename type>
  auto Pool::parfor(type from_, type past_) noexcept -> _impl::_parfor<type>
  {
    return _impl::_parfor<type>(this, from_, past_);
  }

  auto Pool::parfor(const size_t size_) noexcept -> _impl::_parfor<size_t>
  {
    return _impl::_parfor<size_t>(this, 0, size_);
  }
  
  template<typename iterable>
  auto Pool::parfor(iterable& iterable_) noexcept -> _impl::_parfor<iterable>
  {
    return _impl::_parfor<iterable>(this, begin(iterable_), end(iterable_));
  }

// # define parfor(...) push() <<= mtz::_impl::_parfor(__VA_ARGS__) = [&]() -> void
# define parfor(PARFOR_VARIABLE, ...) \
    parfor(__VA_ARGS__) = [&](decltype(mtz::_impl::_parfor_type(__VA_ARGS__)) PARFOR_VARIABLE) -> void

  void Pool::_async_assign() noexcept
  {
    while (_alive)
    {
      if (_active == false) continue;

      for (unsigned k = 0; k < _size; ++k)
      {
        if (_workers[k]._busy()) continue;

        std::lock_guard<std::mutex> lock{_queue_mtx};
        if (_queue.empty() == false)
        {
          _workers[k]._task(std::move(_queue.front()));
          _queue.pop();
          _mtz_impl_DEBUG("assigned to worker thread #%02u", k);
        }
      }
    }
  }

  Pool::~Pool() noexcept
  {
    wait();

    _alive = false;
    _assignation_thread.join();

    _mtz_impl_DEBUG("all workers killed.");
  }
  
// # undef  MTZ_PARFOR
// # define MTZ_PARFOR(POOL, ...)                  _mtz_impl_PARFOR_PRXY(__LINE__, POOL, __VA_ARGS__)
// # define _mtz_impl_PARFOR_PRXY(LINE, POOL, ...) _mtz_impl_PARFOR_IMPL(LINE,     POOL, __VA_ARGS__)
// # define _mtz_impl_PARFOR_IMPL(LINE, POOL, ...) POOL <<= mtz::_impl::_parfor(__VA_ARGS__) = [&]

// # undef  MTZ_PARFOR_IDX
// # define MTZ_PARFOR_IDX mtz::Pool::idx

# undef  MTZ_CYCLIC
# define MTZ_CYCLIC(NS)                  _mtz_impl_CYCLIC_PRXY(__LINE__, NS)
# define _mtz_impl_CYCLIC_PRXY(LINE, NS) _mtz_impl_CYCLIC_IMPL(LINE,     NS)
# define _mtz_impl_CYCLIC_IMPL(LINE, NS)                                      \
    mtz::_impl::_cyclic<NS> _mtz_impl_cyclic##LINE = (mtz::_impl::_work_t)[&]

  inline namespace _literals
  {
    constexpr
    auto operator""_mHz(const long double freq_) -> std::chrono::nanoseconds::rep
    {
      return static_cast<std::chrono::nanoseconds::rep>(1000000000000/freq_);
    }

    constexpr
    auto operator""_mHz(const unsigned long long freq_) -> std::chrono::nanoseconds::rep
    {
      return static_cast<std::chrono::nanoseconds::rep>(1000000000000/freq_);
    }

    constexpr
    auto operator""_Hz(const long double freq_) -> std::chrono::nanoseconds::rep
    {
      return static_cast<std::chrono::nanoseconds::rep>(1000000000/freq_);
    }

    constexpr
    auto operator""_Hz(const unsigned long long freq_) -> std::chrono::nanoseconds::rep
    {
      return static_cast<std::chrono::nanoseconds::rep>(1000000000/freq_);
    }
    
    constexpr
    auto operator""_kHz(const long double freq_) -> std::chrono::nanoseconds::rep
    {
      return static_cast<std::chrono::nanoseconds::rep>(1000000/freq_);
    }

    constexpr
    auto operator""_kHz(const unsigned long long freq_) -> std::chrono::nanoseconds::rep
    {
      return static_cast<std::chrono::nanoseconds::rep>(1000000/freq_);
    }
  }
//----------------------------------------------------------------------------------------------------------------------
# undef _mtz_impl_PRAGMA
# undef _mtz_impl_CLANG_IGNORE
# undef _mtz_impl_LIKELY
# undef _mtz_impl_EXPECTED
# undef _mtz_impl_NODISCARD
# undef _mtz_impl_NODISCARD_REASON
# undef _mtz_impl_DEBUG
# undef _mtz_impl_CONSTEXPR_CPP14
}
#else
#error "mtz: Concurrent threads are required"
#endif
#else
#error "mtz: Support for ISO C++11 is required"
#endif
#endif