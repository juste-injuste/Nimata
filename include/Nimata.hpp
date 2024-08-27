/*---author-------------------------------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Nimata

-----licence------------------------------------------------------------------------------------------------------------

MIT License

Copyright (c) 2023 Justin Asselin (juste-injuste)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

-----versions-----------------------------------------------------------------------------------------------------------

-----description--------------------------------------------------------------------------------------------------------

-----inclusion guard--------------------------------------------------------------------------------------------------*/
#ifndef _nimata_hpp
#define _nimata_hpp
#if defined(__cplusplus) and (__cplusplus >= 201103L)
#if defined(__STDCPP_THREADS__)
//---necessary standard libraries---------------------------------------------------------------------------------------
#include <thread>      // for std::thread, std::this_thread::yield, std::this_thread::sleep_for
#include <mutex>       // for std::mutex, std::lock_guard
#include <atomic>      // for std::atomic
#include <future>      // for std::future, std::promise
#include <functional>  // for std::function
#include <queue>       // for std::queue
#include <chrono>      // for std::chrono::nanoseconds, std::chrono::high_resolution_clock, std::chrono::steady_clock
#include <ostream>     // for std::ostream
#include <iostream>    // for std::clog
#include <memory>      // for std::unique_ptr
#include <utility>     // for std::declval
#include <type_traits> // for std::is_function, std::is_same, std::enable_if, std::conditional, std:: true_type, std::false_type
//---conditionally necessary standard libraries-------------------------------------------------------------------------
#if defined(STZ_DEBUGGING)
# include <cstdio>     // for std::sprintf
#endif
//---Nimata library-----------------------------------------------------------------------------------------------------
namespace stz
{
inline namespace mtz
//----------------------------------------------------------------------------------------------------------------------
{
  const unsigned MAX_THREADS = std::thread::hardware_concurrency();

  class Pool;

  void cyclic_async(size_t period, ...);

  namespace _io
  {
    static std::ostream dbg(std::clog.rdbuf()); // debugging
  }

  inline namespace _literals
  {
    constexpr auto operator""_mHz(long double        frequency) -> std::chrono::nanoseconds;
    constexpr auto operator""_mHz(unsigned long long frequency) -> std::chrono::nanoseconds;
    constexpr auto operator""_Hz (long double        frequency) -> std::chrono::nanoseconds;
    constexpr auto operator""_Hz (unsigned long long frequency) -> std::chrono::nanoseconds;
    constexpr auto operator""_kHz(long double        frequency) -> std::chrono::nanoseconds;
    constexpr auto operator""_kHz(unsigned long long frequency) -> std::chrono::nanoseconds;
  }

  namespace _version
  {
#   define MTZ_VERSION_MAJOR  000
#   define MTZ_VERSION_MINOR  000
#   define MTZ_VERSION_PATCH  000
#   define MTZ_VERSION_NUMBER ((MTZ_VERSION_MAJOR  * 1000 + MTZ_VERSION_MINOR) * 1000 + MTZ_VERSION_PATCH)

    constexpr long MAJOR  = MTZ_VERSION_MAJOR;
    constexpr long MINOR  = MTZ_VERSION_MINOR;
    constexpr long PATCH  = MTZ_VERSION_PATCH;
    constexpr long NUMBER = MTZ_VERSION_NUMBER;
  }
//---Nimata library: backend--------------------------------------------------------------------------------------------
  namespace _impl
  {
#   define _mtz_impl_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
# if defined(__clang__)
#   define _mtz_impl_CLANG_IGNORE(WARNING, ...)          \
      _mtz_impl_PRAGMA(clang diagnostic push)            \
      _mtz_impl_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                        \
      _mtz_impl_PRAGMA(clang diagnostic pop)

#   define _mtz_impl_GCC_IGNORE(WARNING, ...)   __VA_ARGS__
# elif defined(__GNUC__)
#   define _mtz_impl_CLANG_IGNORE(WARNING, ...) __VA_ARGS__

#   define _mtz_impl_GCC_IGNORE(WARNING, ...)          \
      _mtz_impl_PRAGMA(GCC diagnostic push)            \
      _mtz_impl_PRAGMA(GCC diagnostic ignored WARNING) \
      __VA_ARGS__                                      \
      _mtz_impl_PRAGMA(GCC diagnostic pop)
# else
#   define _mtz_impl_CLANG_IGNORE(WARNING, ...) __VA_ARGS__
#   define _mtz_impl_GCC_IGNORE(WARNING, ...)   __VA_ARGS__
#endif


// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define _mtz_impl_LIKELY   _mtz_impl_CLANG_IGNORE("-Wc++20-extensions", [[likely]])
#   define _mtz_impl_UNLIKELY _mtz_impl_CLANG_IGNORE("-Wc++20-extensions", [[unlikely]])
# else
#   define _mtz_impl_LIKELY   [[likely]]
#   define _mtz_impl_UNLIKELY [[unlikely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _mtz_impl_LIKELY   [[likely]]
#   define _mtz_impl_UNLIKELY [[unlikely]]
# else
#   define _mtz_impl_LIKELY
#   define _mtz_impl_UNLIKELY
# endif

// support from clang 3.9.0 and GCC 4.7.3 onward
# if defined(__clang__)
#   define _mtz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _mtz_impl_LIKELY
#   define _mtz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _mtz_impl_UNLIKELY
#   define _mtz_impl_NODISCARD           __attribute__((warn_unused_result))
# elif defined(__GNUC__)
#   define _mtz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _mtz_impl_LIKELY
#   define _mtz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _mtz_impl_UNLIKELY
#   define _mtz_impl_NODISCARD           __attribute__((warn_unused_result))
# else
#   define _mtz_impl_EXPECTED(CONDITION) (CONDITION) _mtz_impl_LIKELY
#   define _mtz_impl_ABNORMAL(CONDITION) (CONDITION) _mtz_impl_UNLIKELY
#   define _mtz_impl_NODISCARD
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

# if defined(STZ_DEBUGGING)
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

    class _worker final
    {
    public:
      ~_worker() noexcept
      {
        _alive = false;
        _worker_thread.join();
      }

      void _task(const std::function<void()>& task_) noexcept
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
      volatile bool         _alive          = true;
      std::function<void()> _work           = nullptr;
      std::atomic_bool      _work_available = {false};
      std::thread           _worker_thread{_loop, this};
    };

    template<std::chrono::nanoseconds::rep PERIOD>
    class _cyclic_async final
    {
      static_assert(PERIOD >= 0, "cyclic_async: 'PERIOD' must be greater or equal to 0.");
    public:
      template<typename Work>
      _cyclic_async(Work work_) noexcept :
        _work(work_)
      {
        _mtz_impl_DEBUG("thread spawned.");
      }

      _cyclic_async(const _cyclic_async&) noexcept {}

      ~_cyclic_async() noexcept
      {
        _alive = false;
        _worker_thread.join();
        _mtz_impl_DEBUG("thread joined.");
      }
    private:
      inline void _loop();
      volatile bool         _alive = true;
      std::function<void()> _work;
      std::thread           _worker_thread{_loop, this};
    };
    
    template<std::chrono::nanoseconds::rep PERIOD>
    void _cyclic_async<PERIOD>::_loop()
    {
      using clock = std::conditional<
        std::chrono::high_resolution_clock::is_steady,
        std::chrono::high_resolution_clock,
        std::chrono::steady_clock
      >::type;

      clock::time_point last = {},  time;
      std::chrono::nanoseconds::rep span;

      while (_alive)
      {
        time = clock::now();
        _mtz_impl_GCC_IGNORE("-Wstrict-overflow", _mtz_impl_CLANG_IGNORE("-Wstrict-overflow",
        span = std::chrono::nanoseconds{time - last}.count();
        ))

        if (span >= PERIOD)
        {
          last = time;
          _work();
        }
      }
    }

    template<>
    inline
    void _cyclic_async<0>::_loop()
    {
      while (_alive)
      {
        _work();
      }
    }

    template<typename T>
    struct _can_convert_to_bool final
    {
    private:
      template<typename T_>
      static
      auto _impl(int) -> decltype
      (
        void(static_cast<bool>(std::declval<T_&>())),
        std::true_type{}
      );

      template<typename T_>
      static
      auto _impl(...) -> std::false_type;

    public:
      static constexpr bool value = decltype(_impl<T>(0)){};
    };

    template<typename T, typename R = bool>
    using _if_can_validate_callable = typename std::enable_if<
      _can_convert_to_bool<T>::value == true
      and std::is_function<T>::value != true, R
    >::type;

    template<typename T, typename R = bool>
    using _no_can_validate_callable = typename std::enable_if<
      _can_convert_to_bool<T>::value != true
      or  std::is_function<T>::value == true, R
    >::type;

    template<typename F>
    constexpr
    auto _validate_callable(const F& function_) noexcept -> _if_can_validate_callable<F>
    {
      return static_cast<bool>(function_);
    }

    template<typename F>
    constexpr
    auto _validate_callable(const F& function_) noexcept -> _no_can_validate_callable<F>
    {
      return true;
    }

    template<typename type>
    using _if_type = typename std::enable_if<std::is_same<type, void>::value == false, type>::type;

    template<typename F, typename... A>
    using _future = std::future<_if_type<decltype(std::declval<F&>()(std::declval<A&>()...))>>;

    template<typename type>
    using _if_void = typename std::enable_if<std::is_same<type, void>::value != false, void>::type;

    template<typename F, typename... A>
    using _void = _if_void<decltype(std::declval<F&>()(std::declval<A&>()...))>;

    template<bool D, typename F, typename... A>
    using _type = typename std::conditional<
      D,
      void,
      std::future<decltype(std::declval<F&>()(std::declval<A&>()...))>
    >::type;

    template<typename T>
    struct _has_iter_meths final
    {
    private:
      template<typename T_>
      static
      auto _impl(int) -> decltype
      (
        void(  std::declval<T_&>().begin() != std::declval<T_&>().end()),
        void(++std::declval<T_&>().begin()),
        void( *std::declval<T_&>().begin()),
        std::true_type{}
      );

      template<typename T_>
      static
      auto _impl(...) -> std::false_type;

    public:
      static constexpr bool value = decltype(_impl<T>(0)){};
    };

    template<typename T>
    struct _has_iter_funcs final
    {
    private:
      template<typename T_>
      static
      auto _impl(int) -> decltype
      (
        void(  begin(std::declval<T_&>()) != end(std::declval<T_&>())),
        void(++begin(std::declval<T_&>())),
        void( *begin(std::declval<T_&>())),
        std::true_type{}
      );

      template<typename T_>
      static
      auto _impl(...) -> std::false_type;

    public:
      static constexpr bool value = decltype(_impl<T>(0)){};
    };

    template<typename T>
    struct _is_iterable final
    {
    public:
      static constexpr bool value = _has_iter_funcs<T>::value || _has_iter_meths<T>::value;
    };

    template<typename T>
    auto _begin(T&& iterable_) noexcept -> typename std::enable_if<
      _has_iter_meths<T>::value
      and not _has_iter_funcs<T>::value,
      decltype(std::declval<T&>().begin())
    >::type
    {
      return iterable_.begin();
    }

    template<typename T>
    auto _begin(T&& iterable_) noexcept -> typename std::enable_if<
      _has_iter_funcs<T>::value,
      decltype(begin(std::declval<T&>()))
    >::type
    {
      return begin(std::forward<T>(iterable_));
    }

    template<typename T>
    auto _end(T&& iterable_) noexcept -> typename std::enable_if<
      _has_iter_meths<T>::value
      and not _has_iter_funcs<T>::value,
      decltype(std::declval<T&>().end())
    >::type
    {
      return iterable_.end();
    }

    template<typename T>
    auto _end(T&& iterable_) noexcept -> typename std::enable_if<
      _has_iter_funcs<T>::value,
      decltype(end(std::declval<T&>()))
    >::type
    {
      return end(std::forward<T>(iterable_));
    }

    template<typename T, bool = _is_iterable<T>::value>
    struct _iter_type;
    
    template<typename T>
    struct _iter_type<T, true> final
    {
      using iter = decltype( _begin(std::declval<T&>()));
      using type = decltype(*_begin(std::declval<T&>()));
      
      static constexpr
      type _dereference(iter& data) noexcept
      {
        return *data;
      }
      
      static constexpr
      const type _dereference(const iter& data) noexcept
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
      type _dereference(const iter data) noexcept
      {
        return data;
      }
    };

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
    inline // add work that has a return value
    auto push(F&& function, A&&... arguments) noexcept -> _impl::_future<F, A...>;

    template<typename F, typename... A>
    inline // add work that does not have a return value
    auto push(F&& function, A&&... arguments) noexcept -> _impl::_void<F, A...>;

    template<bool D, typename F, typename... A>
    inline // add work and specify if you want it detached or not
    auto push(F&& function, A&&... arguments) noexcept -> _impl::_type<D, F, A...>;

    inline // waits for all work to be done
    void wait() const noexcept;

    inline // enable workers
    void work() noexcept;

    inline // disable workers
    void stop() noexcept;

    inline // parallel for-loop with index range = [0, 'size')
    auto parfor(size_t size) noexcept -> _impl::_parfor<size_t>;
    
    inline // parallel for-loop with index range = ['from', 'past')
    auto parfor(size_t from, size_t past) noexcept -> _impl::_parfor<size_t>;
    
    template<typename iterable>
    inline // parallel for-loop over iterable
    auto parfor(iterable&& thing) noexcept -> _impl::_parfor<iterable>;

    inline // get amount of workers
    auto size() const noexcept -> unsigned;

    inline // set amount of workers
    void size(signed number_of_threads) noexcept;

    inline // waits for all work to be done then join threads
    ~Pool() noexcept;
    
  private:
    template<typename>
    friend struct _impl::_parfor;
    inline void _assign() noexcept;
    std::atomic_bool                  _alive  = {true};
    std::atomic_bool                  _active = {true};
    std::atomic_uint                  _size;
    std::atomic<_impl::_worker*>      _workers;
    std::mutex                        _queue_mtx;
    std::queue<std::function<void()>> _queue;
    std::thread                       _assignation_thread{_assign, this};
    template<typename F, typename... A>
    void push(std::true_type,  F&& function, A&&... arguments) noexcept;
    template<typename F, typename... A>
    auto push(std::false_type, F&& function, A&&... arguments) noexcept -> _impl::_future<F, A...>;
  };
//---Nimata library: backend--------------------------------------------------------------------------------------------
  namespace _impl
  {
    template<typename T>
    struct _parfor final
    {
      using iterator = typename _iter_type<T>::iter;

      _parfor(Pool* const pool_, const iterator& from_, const iterator& past_) noexcept :
        _pool(pool_),
        _from(from_),
        _past(past_)
      {}

      template<typename F>
      void operator=(F&& body) noexcept
      {
        {
          std::lock_guard<std::mutex> pool_queue_lock{_pool->_queue_mtx};

          for (iterator iter = _from; iter != _past; ++iter)
          {
            _pool->_queue.push([=]{ body(_iter_type<T>::_dereference(iter)); });
          }
        }

        _pool->wait();
      }

      Pool* const    _pool;
      iterator       _from;
      const iterator _past;
    };
  }
//---Nimata library: frontend definitions-------------------------------------------------------------------------------
  Pool::Pool(signed N_) noexcept :
    _size(_impl::_compute_number_of_threads(N_)),
    _workers(new _impl::_worker[_size])
  {
    _mtz_impl_DEBUG("%u thread%s aquired.", _size, _size == 1 ? "" : "s");
  }
  
  template<typename F, typename... A>
  auto Pool::push(F&& function_, A&&... arguments_) noexcept -> _impl::_future<F, A...>
  {
    return push(
      std::false_type(),
      std::forward<F>(function_),
      std::forward<A>(arguments_)...);
  }
  
  template<typename F, typename... A>
  auto Pool::push(F&& function_, A&&... arguments_) noexcept -> _impl::_void<F, A...>
  {
    return push(
      std::true_type(),
      std::forward<F>(function_),
      std::forward<A>(arguments_)...);
  }

  template<typename F, typename... A>
  void Pool::push(std::true_type, F&& function_, A&&... arguments_) noexcept
  {
    if _mtz_impl_EXPECTED(_impl::_validate_callable(function_) == true)
    {
      std::lock_guard<std::mutex>{_queue_mtx}, _queue.push(
        [=]{ function_(arguments_...); }
      );

      _mtz_impl_DEBUG("pushed a task with no return value.");
    }
    else _mtz_impl_DEBUG("null task pushed.");

    return;
  }

  template<typename F, typename... A>
  auto Pool::push(std::false_type, F&& function_, A&&... arguments_) noexcept -> _impl::_future<F, A...>
  {
    using R = decltype(function_(arguments_...));

    std::future<R> future;

    if _mtz_impl_EXPECTED(_impl::_validate_callable(function_) == true)
    {
      auto promise = new std::promise<R>;
      
      future = promise->get_future();
      
      std::lock_guard<std::mutex>{_queue_mtx}, _queue.push(
        [=]{ std::unique_ptr<std::promise<R>>(promise)->set_value(function_(arguments_...)); }
      );

      _mtz_impl_DEBUG("pushed a task with return value.");
    }
    else _mtz_impl_DEBUG("null task pushed.");

    return future;
  }

  void Pool::wait() const noexcept
  {
    if (_active == true)
    {
      while (_queue.empty() == false)
      {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
      };

      for (unsigned k = 0; k < _size; ++k)
      {
        while (_workers[k]._busy())
        {
          std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }
      }

      _mtz_impl_DEBUG("all threads finished their work.");
    }
  }
  
  void Pool::work() noexcept
  {
    _active = true;
  }

  void Pool::stop() noexcept
  {
    _active = false;
  }

  void Pool::size(const signed N_) noexcept
  {
    wait();

    _size = _impl::_compute_number_of_threads(N_);
    
    delete[] _workers;
    _workers = new _impl::_worker[_size];    
  }

  auto Pool::size() const noexcept -> unsigned
  {
    return _size;
  }
  
  auto Pool::parfor(const size_t from_, const size_t past_) noexcept -> _impl::_parfor<size_t>
  {
    return _impl::_parfor<size_t>(this, from_, past_);
  }

  auto Pool::parfor(const size_t size_) noexcept -> _impl::_parfor<size_t>
  {
    return _impl::_parfor<size_t>(this, 0, size_);
  }
  
  template<typename iterable>
  auto Pool::parfor(iterable&& thing_) noexcept -> _impl::_parfor<iterable>
  {
    return _impl::_parfor<iterable>(this, _impl::_begin(thing_), _impl::_end(thing_));
  }
# undef  parfor_continue
# define parfor_continue return
# define parfor(PARFOR_VARIABLE_DECLARATION, ...) \
    parfor(__VA_ARGS__) = [&](PARFOR_VARIABLE_DECLARATION) -> void


  Pool::~Pool() noexcept
  {
    wait();

    _alive = false;
    _assignation_thread.join();

    delete[] _workers;

    _mtz_impl_DEBUG("all workers killed.");
  }

  void Pool::_assign() noexcept
  {
    while _mtz_impl_EXPECTED(_alive)
    {
      if _mtz_impl_ABNORMAL(_active == false) continue;

      for (unsigned k = 0; k < _size; ++k)
      {
        if (_workers[k]._busy()) continue;

        std::lock_guard<std::mutex> lock{_queue_mtx};
        if (_queue.empty() == false)
        {
          _workers[k]._task(std::move(_queue.front()));
          _queue.pop();
          _mtz_impl_DEBUG("assigned to worker thread #%02u.", k);
        }
      }
    }
  }

# define cyclic_async(DURATION)                      _mtz_impl_cyclic_async_PRXY(__LINE__, DURATION)
# define _mtz_impl_cyclic_async_PRXY(LINE, DURATION) _mtz_impl_cyclic_async_IMPL(LINE,     DURATION)
# define _mtz_impl_cyclic_async_IMPL(LINE, DURATION) \
    mtz::_impl::_cyclic_async<(DURATION).count()> _mtz_impl_cyclic_async##LINE = [&]() -> void

  inline namespace _literals
  {
# if not defined(STZ_LITERALS_FREQUENCY)
#   define STZ_LITERALS_FREQUENCY
    constexpr
    auto operator""_mHz(const long double frequency_) -> std::chrono::nanoseconds
    {
      return std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(1000000000000/frequency_));
    }

    constexpr
    auto operator""_mHz(const unsigned long long frequency_) -> std::chrono::nanoseconds
    {
      return std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(1000000000000/frequency_));
    }

    constexpr
    auto operator""_Hz(const long double frequency_) -> std::chrono::nanoseconds
    {
      return std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(1000000000/frequency_));
    }

    constexpr
    auto operator""_Hz(const unsigned long long frequency_) -> std::chrono::nanoseconds
    {
      return std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(1000000000/frequency_));
    }
    
    constexpr
    auto operator""_kHz(const long double frequency_) -> std::chrono::nanoseconds
    {
      return std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(1000000/frequency_));
    }

    constexpr
    auto operator""_kHz(const unsigned long long frequency_) -> std::chrono::nanoseconds
    {
      return std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(1000000/frequency_));
    }
# endif
  }
}
}
//----------------------------------------------------------------------------------------------------------------------
# undef _mtz_impl_PRAGMA
# undef _mtz_impl_GCC_IGNORE
# undef _mtz_impl_CLANG_IGNORE
# undef _mtz_impl_LIKELY
# undef _mtz_impl_UNLIKELY
# undef _mtz_impl_EXPECTED
# undef _mtz_impl_ABNORMAL
# undef _mtz_impl_NODISCARD
# undef _mtz_impl_NODISCARD_REASON
# undef _mtz_impl_DEBUG
# undef _mtz_impl_CONSTEXPR_CPP14
//----------------------------------------------------------------------------------------------------------------------
#else
#error "mtz: Concurrent threads are required"
#endif
#else
#error "mtz: Support for ISO C++11 is required"
#endif
#endif