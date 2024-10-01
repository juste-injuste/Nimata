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
#include <utility>     // for std::declval, std::move
#include <type_traits> // for std::is_function, std::is_same, std::enable_if, std::conditional, std:: true_type, std::false_type
//---conditionally necessary standard libraries-------------------------------------------------------------------------
#if defined(STZ_DEBUGGING)
# include <cstdio>     // for std::sprintf
#endif
//*///------------------------------------------------------------------------------------------------------------------
namespace stz
{
inline namespace nimata
//*///--summary---------------------------------------------------------------------------------------------------------
{
  const unsigned max_threads = std::thread::hardware_concurrency();

  class Pool;

# define cyclic_async(PERIOD)

  enum class Tracking : uint_fast8_t
  {
    bound, // return std::future
    stray, // return void
    infer  // return_type == void ? void : std::future
  };

  constexpr Tracking bound = Tracking::bound;
  constexpr Tracking stray = Tracking::stray;

  struct io
  {
    static std::ostream& out(); // output
    static std::ostream& dbg(); // debugging
    static std::ostream& wrn(); // warning
    static std::ostream& err(); // errors
  };

# define NIMATA_MAJOR   000
# define NIMATA_MINOR   000
# define NIMATA_PATCH   000
# define NIMATA_VERSION ((NIMATA_MAJOR  * 1000 + NIMATA_MINOR) * 1000 + NIMATA_PATCH)
//*///------------------------------------------------------------------------------------------------------------------
  namespace _nimata_impl
  {
#   define _stz_impl_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
# if defined(__clang__)
#   define _stz_impl_CLANG_IGNORE(WARNING, ...)          \
      _stz_impl_PRAGMA(clang diagnostic push)            \
      _stz_impl_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                        \
      _stz_impl_PRAGMA(clang diagnostic pop)

#   define _stz_impl_GCC_IGNORE(WARNING, ...)       __VA_ARGS__
#   define _stz_impl_GCC_CLANG_IGNORE(WARNING, ...) _stz_impl_CLANG_IGNORE(WARNING, __VA_ARGS__)
# elif defined(__GNUC__)
#   define _stz_impl_CLANG_IGNORE(WARNING, ...) __VA_ARGS__

#   define _stz_impl_GCC_IGNORE(WARNING, ...)          \
      _stz_impl_PRAGMA(GCC diagnostic push)            \
      _stz_impl_PRAGMA(GCC diagnostic ignored WARNING) \
      __VA_ARGS__                                      \
      _stz_impl_PRAGMA(GCC diagnostic pop)
#   define _stz_impl_GCC_CLANG_IGNORE(WARNING, ...) _stz_impl_GCC_IGNORE(WARNING, __VA_ARGS__)
# else
#   define _stz_impl_CLANG_IGNORE(WARNING, ...)     __VA_ARGS__
#   define _stz_impl_GCC_IGNORE(WARNING, ...)       __VA_ARGS__
#   define _stz_impl_GCC_CLANG_IGNORE(WARNING, ...) __VA_ARGS__
#endif

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define _stz_impl_LIKELY   _stz_impl_CLANG_IGNORE("-Wc++20-extensions", [[likely]])
#   define _stz_impl_UNLIKELY _stz_impl_CLANG_IGNORE("-Wc++20-extensions", [[unlikely]])
# else
#   define _stz_impl_LIKELY   [[likely]]
#   define _stz_impl_UNLIKELY [[unlikely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _stz_impl_LIKELY   [[likely]]
#   define _stz_impl_UNLIKELY [[unlikely]]
# else
#   define _stz_impl_LIKELY
#   define _stz_impl_UNLIKELY
# endif

// support from clang 3.9.0 and GCC 4.7.3 onward
# if defined(__clang__)
#   define _stz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _stz_impl_LIKELY
#   define _stz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _stz_impl_UNLIKELY
#   define _stz_impl_NODISCARD           __attribute__((warn_unused_result))
# elif defined(__GNUC__)
#   define _stz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _stz_impl_LIKELY
#   define _stz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _stz_impl_UNLIKELY
#   define _stz_impl_NODISCARD           __attribute__((warn_unused_result))
# else
#   define _stz_impl_EXPECTED(CONDITION) (CONDITION) _stz_impl_LIKELY
#   define _stz_impl_ABNORMAL(CONDITION) (CONDITION) _stz_impl_UNLIKELY
#   define _stz_impl_NODISCARD
# endif

// support from clang 10.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 10)
# if __cplusplus < 202002L
#   define _stz_impl_NODISCARD_REASON(REASON) _stz_impl_CLANG_IGNORE("-Wc++20-extensions", [[nodiscard(REASON)]])
# else
#   define _stz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _stz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# else
#   define _stz_impl_NODISCARD_REASON(REASON) _stz_impl_NODISCARD
# endif

# if defined(STZ_DEBUGGING)
#   if   (STZ_DEBUGGING + 0) >= 3
#     define _stz_impl_DBG_LVL_3(...) __VA_ARGS__
#     define _stz_impl_DBG_LVL_2(...) __VA_ARGS__
#     define _stz_impl_DBG_LVL_1(...) __VA_ARGS__
#     define _stz_impl_DBG_LVL_0(...) __VA_ARGS__
#   elif (STZ_DEBUGGING + 0) >= 2
#     define _stz_impl_DBG_LVL_3(...)
#     define _stz_impl_DBG_LVL_2(...) __VA_ARGS__
#     define _stz_impl_DBG_LVL_1(...) __VA_ARGS__
#     define _stz_impl_DBG_LVL_0(...) __VA_ARGS__
#   elif (STZ_DEBUGGING + 0) >= 1
#     define _stz_impl_DBG_LVL_3(...)
#     define _stz_impl_DBG_LVL_2(...)
#     define _stz_impl_DBG_LVL_1(...) __VA_ARGS__
#     define _stz_impl_DBG_LVL_0(...) __VA_ARGS__
#   elif (STZ_DEBUGGING + 0) >= 0
#     define _stz_impl_DBG_LVL_3(...)
#     define _stz_impl_DBG_LVL_2(...)
#     define _stz_impl_DBG_LVL_1(...)
#     define _stz_impl_DBG_LVL_0(...) __VA_ARGS__
#   endif
# else
#   define _stz_impl_DBG_LVL_3(...)
#   define _stz_impl_DBG_LVL_2(...)
#   define _stz_impl_DBG_LVL_1(...)
#   define _stz_impl_DBG_LVL_0(...)
# endif

# if defined(STZ_DEBUGGING)
    static thread_local char _dbg_buf[128];
    static std::mutex _dbg_mtx;

#   define _stz_impl_DEBUG_MESSAGE(...)                                          \
      [&](const char* const caller_){                                            \
        std::sprintf(_impl::_dbg_buf, __VA_ARGS__);                              \
        std::lock_guard<std::mutex> _lock{_impl::_dbg_mtx};                      \
        io::dbg() << "stz: " << caller_ << ": " << _impl::_dbg_buf << std::endl; \
      }(__func__)
# else
#   define _stz_impl_DEBUG_MESSAGE(...) void(0)
# endif

    inline
    auto _compute_number_of_threads(signed N_) noexcept -> unsigned
    {
      if (N_ <= 0)
      {
        N_ += max_threads;
      }

      if (N_ < 1)
      {
        _stz_impl_DBG_LVL_0(
        _stz_impl_DEBUG_MESSAGE("%d threads is not possible, 1 used instead", N_);)

        N_ = 1;
      }

      if (N_ > static_cast<signed>(max_threads - 2))
      {
        _stz_impl_DBG_LVL_0(
        _stz_impl_DEBUG_MESSAGE("max_threads - 2 is the recommended maximum amount of threads, %d used", N_);)
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
        while _stz_impl_EXPECTED(_alive)
        {
          if _stz_impl_EXPECTED(_work_available)
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
    
    template<typename Representation, typename Period>
    constexpr
    auto _to_ns(const std::chrono::duration<Representation, Period> duration_) -> std::chrono::nanoseconds::rep
    {
      return std::chrono::nanoseconds(duration_).count();
    }

    template<typename Type>
    constexpr
    auto _to_ns(const Type milliseconds_) -> std::chrono::nanoseconds::rep
    {
      return std::chrono::nanoseconds(std::chrono::milliseconds(milliseconds_)).count();
    }

    template<std::chrono::nanoseconds::rep PERIOD>
    class _cyclic_async final
    {
      static_assert(PERIOD >= 0, "stz: cyclic_async: 'PERIOD' must be greater or equal to 0.");
    public:
      template<typename Work>
      _cyclic_async(Work work_) noexcept
        : _work(work_)
      {
        _stz_impl_DBG_LVL_3(_stz_impl_DEBUG_MESSAGE("thread spawned.");)
      }

      _cyclic_async(const _cyclic_async&) noexcept
      {}

      ~_cyclic_async() noexcept
      {
        _alive = false;
        _worker_thread.join();
        
        _stz_impl_DBG_LVL_3(_stz_impl_DEBUG_MESSAGE("thread joined.");)
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

      clock::time_point last = clock::time_point(), time;
      std::chrono::nanoseconds::rep span;

      while (_alive)
      {
        time = clock::now();
        _stz_impl_GCC_CLANG_IGNORE("-Wstrict-overflow",
        span = std::chrono::nanoseconds(time - last).count();
        )

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

    template<typename Type>
    struct _can_convert_to_bool final
    {
    private:
      template<typename Type_>
      static
      auto _impl(int) -> decltype
      (
        void(static_cast<bool>(std::declval<Type_&>())),
        std::true_type()
      );

      template<typename Type_>
      static
      auto _impl(...) -> std::false_type;

    public:
      static constexpr bool value = decltype(_impl<Type>(0))();
    };

    template<typename Type, typename Result = bool>
    using _if_can_validate_callable = typename std::enable_if<
      _can_convert_to_bool<Type>::value == true
      and std::is_function<Type>::value != true, Result
    >::type;

    template<typename Type, typename Result = bool>
    using _no_can_validate_callable = typename std::enable_if<
      _can_convert_to_bool<Type>::value != true
      or  std::is_function<Type>::value == true, Result
    >::type;

    template<typename Callable>
    auto _validate_callable(const Callable& callable_) noexcept -> _if_can_validate_callable<Callable>
    {
      return static_cast<bool>(callable_);
    }

    template<typename Callable>
    auto _validate_callable(const Callable&) noexcept -> _no_can_validate_callable<Callable>
    {
      return true;
    }

    template<typename Type>
    struct _has_iter_meths final
    {
    private:
      template<typename Type_>
      static
      auto _impl(int) -> decltype
      (
        void(  std::declval<Type_&>().begin() != std::declval<Type_&>().end()),
        void(++std::declval<Type_&>().begin()),
        void( *std::declval<Type_&>().begin()),
        std::true_type()
      );

      template<typename Type_>
      static
      auto _impl(...) -> std::false_type;

    public:
      static constexpr bool value = decltype(_impl<Type>(0))();
    };

    template<typename Type>
    struct _has_iter_funcs final
    {
    private:
      template<typename Type_>
      static
      auto _impl(int) -> decltype
      (
        void(  begin(std::declval<Type_&>()) != end(std::declval<Type_&>())),
        void(++begin(std::declval<Type_&>())),
        void( *begin(std::declval<Type_&>())),
        std::true_type()
      );

      template<typename Type_>
      static
      auto _impl(...) -> std::false_type;

    public:
      static constexpr bool value = decltype(_impl<Type>(0))();
    };

    template<typename Type>
    struct _is_iterable final
    {
    public:
      static constexpr bool value =
           _has_iter_funcs<Type>::value
        || _has_iter_meths<Type>::value
        || std::is_pointer<Type>::value;
    };

    template<typename Type>
    using _if_iterable = typename std::enable_if<_is_iterable<Type>::value>::type;

    template<typename Type>
    auto _begin(Type&& iterable_) noexcept -> typename std::enable_if<
      _has_iter_meths<Type>::value and not _has_iter_funcs<Type>::value,
      decltype(std::declval<Type&>().begin())
    >::type
    {
      return iterable_.begin();
    }

    template<typename Type>
    auto _begin(Type&& iterable_) noexcept -> typename std::enable_if<
      _has_iter_funcs<Type>::value,
      decltype(begin(std::declval<Type&>()))
    >::type
    {
      return begin(std::forward<Type>(iterable_));
    }

    template<typename Type>
    auto _end(Type&& iterable_) noexcept -> typename std::enable_if<
      _has_iter_meths<Type>::value and not _has_iter_funcs<Type>::value,
      decltype(std::declval<Type&>().end())
    >::type
    {
      return iterable_.end();
    }

    template<typename Type>
    auto _end(Type&& iterable_) noexcept -> typename std::enable_if<
      _has_iter_funcs<Type>::value,
      decltype(end(std::declval<Type&>()))
    >::type
    {
      return end(std::forward<Type>(iterable_));
    }

    template<typename Type>
    auto _begin(Type* pointer_) noexcept -> Type*
    {
      return pointer_;
    }

    template<typename Type>
    auto _end(Type* pointer_) noexcept -> Type*
    {
      return pointer_;
    }

    template<typename Type, bool = _is_iterable<Type>::value>
    struct _iter_type;

    template<typename Type>
    struct _iter_type<Type, true> final
    {
      using iter = decltype( _begin(std::declval<Type&>()));
      using type = decltype(*_begin(std::declval<Type&>()));

      static
      type _deref(iter& data) noexcept
      {
        return *data;
      }

      static
      const type _deref(const iter& data) noexcept
      {
        return *data;
      }
    };

    template<typename Type>
    struct _iter_type<Type, false> final
    {
      using iter = Type;
      using type = Type;

      static
      type _deref(const iter data) noexcept
      {
        return data;
      }
    };

    template<typename Callable, typename... Arguments>
    using _result = decltype(std::declval<Callable&>()(std::declval<Arguments&>()...));

    template<typename Callable, typename... Arguments>
    using _auto = typename std::conditional<
      std::is_same<_result<Callable, Arguments...>, void>::value,
      void,
      std::future<_result<Callable, Arguments...>>
    >::type;

    template<typename Callable, typename... Arguments>
    using _future = typename std::future<_result<Callable, Arguments...>>;

    template<Tracking tracking, typename Callable, typename... Arguments>
    using _tracking = 
      typename std::conditional<tracking == Tracking::stray,
        void,
        typename std::conditional<tracking == Tracking::bound,
          std::future<_result<Callable, Arguments...>>,
          typename std::conditional<std::is_same<_result<Callable, Arguments...>, void>::value,
            void,
            std::future<_result<Callable, Arguments...>>
          >::type
        >::type
      >::type;

    using _attached = std::integral_constant<Tracking, Tracking::bound>;
    using _detached = std::integral_constant<Tracking, Tracking::stray>;
    using _inferred = std::integral_constant<Tracking, Tracking::infer>;

    template<typename Type>
    struct _parfor;

    template<typename Result>
    struct _push;
  }
//*///------------------------------------------------------------------------------------------------------------------
  class Pool
  {
  public:
    // constructs pool
    inline Pool(signed number_of_threads = max_threads) noexcept;

    // add work and specify if you want it detached or not
    template<Tracking tracking = Tracking::infer, typename Callable, typename... Arguments>
    inline auto push
    (
      Callable&&     callable,
      Arguments&&... arguments
    ) noexcept -> _nimata_impl::_tracking<tracking, Callable, Arguments...>;

    // waits for all work to be done
    inline void wait() const noexcept;

    // enable workers
    inline void work() noexcept;

    // disable workers
    inline void stop() noexcept;

    // parallel for-loop with index range = [0, 'size')
    inline auto parfor(size_t size) noexcept -> _nimata_impl::_parfor<size_t>;

    // parallel for-loop with index range = ['from', 'past')
    inline auto parfor(size_t from, size_t past) noexcept -> _nimata_impl::_parfor<size_t>;

    // parallel for-loop over iterable
    template<typename Iterable, typename = _nimata_impl::_if_iterable<Iterable>>
    inline auto parfor(Iterable&& thing) noexcept -> _nimata_impl::_parfor<Iterable>;

    // parallel for-loop over fixed-size array
    template<typename Type, size_t Size>
    auto parfor(Type (&array)[Size]) noexcept -> _nimata_impl::_parfor<Type*>;

    // get amount of workers
    inline auto size() const noexcept -> unsigned;

    // set amount of workers
    inline void size(signed number_of_threads) noexcept;

    // waits for all work to be done then join threads
    inline ~Pool() noexcept;

  private:
    template<typename> friend struct _nimata_impl::_parfor;
    template<typename> friend struct _nimata_impl::_push;
    inline void _assign() noexcept;
    std::atomic_bool                    _alive  = {true};
    std::atomic_bool                    _active = {true};
    std::atomic_uint                    _size;
    std::atomic<_nimata_impl::_worker*> _workers;
    std::mutex                          _queue_mtx;
    std::queue<std::function<void()>>   _queue;
    std::thread                         _assignation_thread{_assign, this};

    template<typename F, typename... A>
    auto push(_nimata_impl::_detached, F&& function, A&&... arguments) noexcept -> void;
    template<typename F, typename... A>
    auto push(_nimata_impl::_attached, F&& function, A&&... arguments) noexcept -> _nimata_impl::_future<F, A...>;
    template<typename F, typename... A>
    auto push(_nimata_impl::_inferred, F&& function, A&&... arguments) noexcept -> _nimata_impl::_auto<F, A...>;
  };
//*///------------------------------------------------------------------------------------------------------------------
  namespace _nimata_impl
  {
    template<typename Type>
    struct _parfor final
    {
      using iterator = typename _iter_type<Type>::iter;

      _parfor(Pool* const pool_, const iterator& from_, const iterator& past_) noexcept
        : _pool(pool_)
        , _from(from_)
        , _past(past_)
      {}

      template<typename Callable>
      void operator=(Callable&& callable_) noexcept
      {
        {
          std::lock_guard<std::mutex> pool_queue_lock(_pool->_queue_mtx);

          for (iterator iter = std::move(_from); iter != _past; ++iter)
          {
            _pool->_queue.push([=]{ callable_(_iter_type<Type>::_deref(iter)); });
          }
        }

        _pool->wait();
      }

      Pool* const    _pool;
      iterator       _from;
      const iterator _past;
    };

    template<typename Result>
    struct _infer;

    template<>
    struct _infer<void>
    {
      template<typename Callable, typename... Arguments>
      static
      void _impl(Pool* const pool_, Callable&& callable_, Arguments&&... arguments_)
      {
        return pool_->push<Tracking::stray>(callable_, arguments_...);
      }
    };

    template<typename Result>
    struct _infer
    {
      template<typename Callable, typename... Arguments>
      static
      auto _impl(Pool* const pool_, Callable&& callable_, Arguments&&... arguments_) -> std::future<Result>
      {
        return pool_->push<Tracking::bound>(callable_, arguments_...);
      }
    };

    template<>
    struct _push<void>
    {
      template<typename Callable, typename... Arguments>
      static
      auto _impl(Pool* const pool_, Callable&& callable_, Arguments&&... arguments_) -> std::future<void>
      {
        std::future<void> future;

        if _stz_impl_EXPECTED(_nimata_impl::_validate_callable(callable_) == true)
        {
          auto promise = new std::promise<void>;

          std::lock_guard<std::mutex>{pool_->_queue_mtx}, pool_->_queue.push(
            [=]{ callable_(arguments_...), std::unique_ptr<std::promise<void>>(promise)->set_value(); }
          );

          future = promise->get_future();

          _stz_impl_DBG_LVL_2(_stz_impl_DEBUG_MESSAGE("pushed an attached task.");)
        }
        else
        {
          _stz_impl_DBG_LVL_2(_stz_impl_DEBUG_MESSAGE("null task pushed.");)
        }

        return future;
      }
    };

    template<typename ResultType>
    struct _push
    {
      template<typename Callable, typename... Arguments>
      static
      auto _impl(Pool* const pool_, Callable&& callable_, Arguments&&... arguments_) -> std::future<ResultType>
      {
        std::future<ResultType> future;

        if _stz_impl_EXPECTED(_nimata_impl::_validate_callable(callable_) == true)
        {
          auto promise = new std::promise<ResultType>;

          future = promise->get_future();

          std::lock_guard<std::mutex>(pool_->_queue_mtx), pool_->_queue.push(
            [=]{ std::unique_ptr<std::promise<ResultType>>(promise)->set_value(callable_(arguments_...)); }
          );

          _stz_impl_DBG_LVL_2(_stz_impl_DEBUG_MESSAGE("pushed a task with return value.");)
        } 
        else
        {
          _stz_impl_DBG_LVL_2(_stz_impl_DEBUG_MESSAGE("null task pushed.");)
        }

        return future;
      }
    };
  }
//*///------------------------------------------------------------------------------------------------------------------
  Pool::Pool(const signed N_) noexcept
    : _size(_nimata_impl::_compute_number_of_threads(N_))
    , _workers(new _nimata_impl::_worker[_size])
  {
    _stz_impl_DBG_LVL_0(_stz_impl_DEBUG_MESSAGE("%u thread%s aquired.", _size, _size == 1 ? "" : "s");)
  }

  template<Tracking T, typename Callable, typename... Arguments>
  auto Pool::push
  (
    Callable&&     callable_,
    Arguments&&... arguments_
  ) noexcept -> _nimata_impl::_tracking<T, Callable, Arguments...>
  {
    return push(std::integral_constant<Tracking, T>(), callable_, arguments_...);
  }
  
  template<typename Callable, typename... Arguments>
  auto Pool::push(_nimata_impl::_detached, Callable&& callable_, Arguments&&... arguments_) noexcept -> void
  {
    if _stz_impl_EXPECTED(_nimata_impl::_validate_callable(callable_) == true)
    {
      std::lock_guard<std::mutex>{_queue_mtx}, _queue.push(
        [=]{ callable_(arguments_...); }
      );

      _stz_impl_DBG_LVL_2(_stz_impl_DEBUG_MESSAGE("pushed a task with no return value.");)
    }
    else
    {
      _stz_impl_DBG_LVL_2(_stz_impl_DEBUG_MESSAGE("null task pushed.");)
    }

    return;
  }

  template<typename Callable, typename... Arguments>
  _stz_impl_NODISCARD_REASON("stz: push: wrap in a lambda if you don't use the return value.")
  auto Pool::push
  (
    _nimata_impl::_attached,
    Callable&&     callable_,
    Arguments&&... arguments_
  ) noexcept -> _nimata_impl::_future<Callable, Arguments...>
  {
    return _nimata_impl::_push<_nimata_impl::_result<Callable, Arguments...>>::_impl(this, callable_, arguments_...);
  }

  template<typename Callable, typename... Arguments>
  auto Pool::push
  (
    _nimata_impl::_inferred,
    Callable&&     callable_,
    Arguments&&... arguments
  ) noexcept -> _nimata_impl::_auto<Callable, Arguments...>
  {
    return _nimata_impl::_infer<_nimata_impl::_result<Callable, Arguments...>>::_impl(this, callable_, arguments...);
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

      _stz_impl_DBG_LVL_1(_stz_impl_DEBUG_MESSAGE("all threads finished their work.");)
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

    _size = _nimata_impl::_compute_number_of_threads(N_);

    delete[] _workers;
    _workers = new _nimata_impl::_worker[_size];
  }

  auto Pool::size() const noexcept -> unsigned
  {
    return _size;
  }

  auto Pool::parfor(const size_t from_, const size_t past_) noexcept -> _nimata_impl::_parfor<size_t>
  {
    return _nimata_impl::_parfor<size_t>(this, from_, past_);
  }

  auto Pool::parfor(const size_t size_) noexcept -> _nimata_impl::_parfor<size_t>
  {
    return _nimata_impl::_parfor<size_t>(this, 0, size_);
  }

  template<typename iterable, typename>
  auto Pool::parfor(iterable&& thing_) noexcept -> _nimata_impl::_parfor<iterable>
  {
    return _nimata_impl::_parfor<iterable>(this, _nimata_impl::_begin(thing_), _nimata_impl::_end(thing_));
  }

  template<typename Type, size_t Size>
  auto Pool::parfor(Type (&array_)[Size]) noexcept -> _nimata_impl::_parfor<Type*>
  {
    return _nimata_impl::_parfor<Type*>(this, array_, array_ + Size);
  }

# define parfor(PARFOR_VARIABLE_DECLARATION, ...) parfor(__VA_ARGS__) = [&](PARFOR_VARIABLE_DECLARATION) -> void
//*///------------------------------------------------------------------------------------------------------------------
  Pool::~Pool() noexcept
  {
    wait();

    _alive = false;
    _assignation_thread.join();

    delete[] _workers;

    _stz_impl_DBG_LVL_0(_stz_impl_DEBUG_MESSAGE("all workers killed.");)
  }

  void Pool::_assign() noexcept
  {
    while _stz_impl_EXPECTED(_alive)
    {
      if _stz_impl_ABNORMAL(_active == false)
      {
        continue;
      }

      for (unsigned k = 0; k < _size; ++k)
      {
        if (_workers[k]._busy())
        {
          continue;
        }

        std::lock_guard<std::mutex> lock{_queue_mtx};
        if (_queue.empty() == false)
        {
          _workers[k]._task(std::move(_queue.front()));
          _queue.pop();

          _stz_impl_DBG_LVL_2(_stz_impl_DEBUG_MESSAGE("assigned to worker thread #%02u.", k);)
        }
      }
    }
  }
//*///------------------------------------------------------------------------------------------------------------------
# undef cyclic_async
  void cyclic_async();

# define _stz_impl_cyclic_async_IMPL(LINE, PERIOD) \
    _nimata_impl::_cyclic_async<stz::_nimata_impl::_to_ns(PERIOD)> _stz_impl_cyclic_async##LINE = [&]() -> void
# define _stz_impl_cyclic_async_PRXY(LINE, PERIOD) _stz_impl_cyclic_async_IMPL(LINE,     PERIOD)
# define cyclic_async(PERIOD)                      _stz_impl_cyclic_async_PRXY(__LINE__, PERIOD)
//*///------------------------------------------------------------------------------------------------------------------
# define _stz_impl_IO(NAME, LINK) std::ostream& io::NAME() { static std::ostream NAME(LINK.rdbuf());  return NAME; }
  _stz_impl_IO(out, std::cout)
  _stz_impl_IO(dbg, std::clog)
  _stz_impl_IO(wrn, std::cerr)
  _stz_impl_IO(err, std::cerr)
# undef _stz_impl_IO
//*///------------------------------------------------------------------------------------------------------------------
}
//*///------------------------------------------------------------------------------------------------------------------
  inline namespace _literals
  {
# if not defined(_stz_impl_LITERALS_FREQUENCY)
#   define _stz_impl_LITERALS_FREQUENCY
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
//*///------------------------------------------------------------------------------------------------------------------
}
//*///------------------------------------------------------------------------------------------------------------------
# undef _stz_impl_PRAGMA
# undef _stz_impl_GCC_IGNORE
# undef _stz_impl_CLANG_IGNORE
# undef _stz_impl_GCC_CLANG_IGNORE
# undef _stz_impl_LIKELY
# undef _stz_impl_UNLIKELY
# undef _stz_impl_EXPECTED
# undef _stz_impl_ABNORMAL
# undef _stz_impl_NODISCARD
# undef _stz_impl_NODISCARD_REASON
# undef _stz_impl_DBG_LVL_3
# undef _stz_impl_DBG_LVL_2
# undef _stz_impl_DBG_LVL_1
# undef _stz_impl_DBG_LVL_0
# undef _stz_impl_DEBUG_MESSAGE
//*///------------------------------------------------------------------------------------------------------------------
#else
#error "nimata: Concurrent threads are required"
#endif
#else
#error "nimata: Support for ISO C++11 is required"
#endif
#endif