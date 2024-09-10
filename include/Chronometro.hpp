/*---author-------------------------------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Chronometro

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

Chronometro is a simple and lightweight C++11 (and newer) library that allows you to measure the execution time of code
blocks and more. See the included README.MD file for more information.

-----inclusion avoid--------------------------------------------------------------------------------------------------*/
#ifndef _chronometro_hpp
#define _chronometro_hpp
#if __cplusplus >= 201103L
//---necessary standard libraries---------------------------------------------------------------------------------------
#include <chrono>    // for std::chrono::steady_clock, std::chrono::high_resolution_clock, std::chrono::nanoseconds
#include <ostream>   // for std::ostream
#include <iostream>  // for std::cout, std::endl
#include <string>    // for std::string, std::to_string
#include <utility>   // for std::move
#include <cstdio>    // for std::sprintf
#include <exception> // for std::exception
//---conditionally necessary standard libraries-------------------------------------------------------------------------
#if not defined(CHRONOMETRO_CLOCK)
# include <type_traits> // for std::conditional
#endif
#if defined(__STDCPP_THREADS__) and not defined(CHRONOMETRO_NOT_THREADSAFE)
# define  _stz_impl_THREADSAFE
# include <mutex> // for std::mutex, std::lock_guard
#endif
//*///------------------------------------------------------------------------------------------------------------------
namespace stz
{
inline namespace chronometro
//*///------------------------------------------------------------------------------------------------------------------
{
  // measures the time it takes to execute statements
# define measure_block(...) // followed by '{ statements... };'

  // measure elapsed time
  class Stopwatch;

  // measure iterations via range-based for-loop
  class Measure;

  // units in which time obtained from Stopwatch can
  // be displayed and in which sleep() be slept with.
  enum class Unit
  {
    ns,       // nanoseconds
    us,       // microseconds
    ms,       // milliseconds
    s,        // seconds
    min,      // minutes
    h,        // hours
    automatic // deduce appropriate unit automatically
  };

  // pause calling thread for 'amount' 'unit's of time
  template<Unit unit = Unit::ms>
  void sleep(unsigned long long amount) noexcept;

  // pause calling thread for 'duration' amount of time
  template<typename R = std::chrono::milliseconds::rep, typename P = std::chrono::milliseconds::period>
  void sleep(std::chrono::duration<R, P> duration) noexcept;

  // execute statements if last execution was atleast 'DURATION' prior
# define if_elapsed(DURATION) // followed by '{ statements... };'

  // execute statements 'N' times
# define loop_n_times(N) // followed by '{ statements... };'

  // execute statements every 'N' encounters
# define if_n_pass(N) // followed by '{ statements... };'

  // break out of stz looping mechanisms
# define break_now

  // break out of a stz looping mechanism if encountered 'N' times
# define break_after_n(N) // followed by '{ statements... };'

  namespace io
  {
    static std::ostream out(std::cout.rdbuf()); // output
    static std::ostream log(std::clog.rdbuf()); // logging
    static std::ostream dbg(std::clog.rdbuf()); // debugging
    static std::ostream wrn(std::cerr.rdbuf()); // warnings
    static std::ostream err(std::cerr.rdbuf()); // errors
  }

# define CHRONOMETRO_MAJOR    000
# define CHRONOMETRO_MINOR    000
# define CHRONOMETRO_PATCH    000
# define CHRONOMETRO_VERSION ((CHRONOMETRO_MAJOR  * 1000 + CHRONOMETRO_MINOR) * 1000 + CHRONOMETRO_PATCH)
//*///------------------------------------------------------------------------------------------------------------------
  namespace _chronometro_impl
  {
# if defined(__clang__)
#   define _stz_impl_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#   define _stz_impl_CLANG_IGNORE(WARNING, ...)          \
      _stz_impl_PRAGMA(clang diagnostic push)            \
      _stz_impl_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                        \
      _stz_impl_PRAGMA(clang diagnostic pop)
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
# elif defined(__GNUC__)
#   define _stz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _stz_impl_LIKELY
#   define _stz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _stz_impl_UNLIKELY
# else
#   define _stz_impl_EXPECTED(CONDITION) (CONDITION) _stz_impl_LIKELY
#   define _stz_impl_ABNORMAL(CONDITION) (CONDITION) _stz_impl_UNLIKELY
# endif

// support from clang 3.9.0 and GCC 4.7.3 onward
# if defined(__clang__)
#   define _stz_impl_NODISCARD __attribute__((warn_unused_result))
# elif defined(__GNUC__)
#   define _stz_impl_NODISCARD __attribute__((warn_unused_result))
# else
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

#if defined(_stz_impl_THREADSAFE)
# undef  _stz_impl_THREADSAFE
# define _stz_impl_THREADLOCAL         thread_local
# define _stz_impl_DECLARE_MUTEX(...)  static std::mutex __VA_ARGS__
# define _stz_impl_DECLARE_LOCK(MUTEX) std::lock_guard<decltype(MUTEX)> _lock(MUTEX)
#else
# define _stz_impl_THREADLOCAL
# define _stz_impl_DECLARE_MUTEX(...)
# define _stz_impl_DECLARE_LOCK(MUTEX)
#endif

  // clock used to measure time
#if defined(CHRONOMETRO_CLOCK)
  using _clock = CHRONOMETRO_CLOCK;
#else
  using _clock = std::conditional<
    std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock
  >::type;
#endif

    template<Unit unit, unsigned n_decimals>
    struct _time final
    {
    public:
      const std::chrono::nanoseconds nanoseconds;

      template<Unit unit_, unsigned n_decimals_ = n_decimals>
      auto style() const noexcept -> const _time<unit_, n_decimals_>&
      {
        static_assert(n_decimals <= 4, "style: too many decimals requested.");
        return reinterpret_cast<const _time<unit_, n_decimals_>&>(*this);
      }

      template<unsigned n_decimals_, Unit unit_ = unit>
      auto style() const noexcept -> const _time<unit_, n_decimals_>&
      {
        static_assert(n_decimals <= 4, "style: too many decimals requested.");
        return reinterpret_cast<const _time<unit_, n_decimals_>&>(*this);
      }
    };

    _stz_impl_DECLARE_MUTEX(_out_mtx);

    template<Unit>
    struct _unit_helper;

#   define _stz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(UNIT, LABEL, FACTOR) \
      template<>                                                          \
      struct _unit_helper<UNIT> final                                     \
      {                                                                   \
        static constexpr const char*         label  = LABEL;              \
        static constexpr unsigned long long  factor = FACTOR;             \
        static constexpr double             ifactor = 1.0/FACTOR;         \
      }

    _stz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::ns,  "ns",  1);
    _stz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::us,  "us",  1000);
    _stz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::ms,  "ms",  1000000);
    _stz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::s,   "s",   1000000000);
    _stz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::min, "min", 60000000000);
    _stz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::h,   "h",   3600000000000);
#   undef _stz_impl_MAKE_UNIT_HELPER_SPECIALIZATION

    template<Unit unit, unsigned n_decimals>
    auto _time_as_cstring(const _time<unit, n_decimals> time_) noexcept -> const char*
    {
      static _stz_impl_THREADLOCAL char buffer[32];

      const double ajusted_time = static_cast<double>(time_.nanoseconds.count()) * _unit_helper<unit>::ifactor;

      std::sprintf(buffer,
        (n_decimals == 0) ? "%.0f %s"
      : (n_decimals == 1) ? "%.1f %s"
      : (n_decimals == 2) ? "%.2f %s"
      : (n_decimals == 3) ? "%.3f %s"
      :                     "%.4f %s",
      ajusted_time, _unit_helper<unit>::label);

      return buffer;
    }

    template<unsigned n_decimals>
    auto _time_as_cstring(const _time<Unit::automatic, n_decimals> time_) noexcept -> const char*
    {
      // 10 h < duration
      if _stz_impl_ABNORMAL(time_.nanoseconds.count() > 36000000000000)
      {
        return _time_as_cstring(_time<Unit::h, n_decimals>{time_.nanoseconds});
      }

      // 10 min < duration <= 10 h
      if _stz_impl_ABNORMAL(time_.nanoseconds.count() > 600000000000)
      {
        return _time_as_cstring(_time<Unit::min, n_decimals>{time_.nanoseconds});
      }

      // 10 s < duration <= 10 m
      if (time_.nanoseconds.count() > 10000000000)
      {
        return _time_as_cstring(_time<Unit::s, n_decimals>{time_.nanoseconds});
      }

      // 10 ms < duration <= 10 s
      if (time_.nanoseconds.count() > 10000000)
      {
        return _time_as_cstring(_time<Unit::ms, n_decimals>{time_.nanoseconds});
      }

      // 10 us < duration <= 10 ms
      if (time_.nanoseconds.count() > 10000)
      {
        return _time_as_cstring(_time<Unit::us, n_decimals>{time_.nanoseconds});
      }

      // duration <= 10 us
      return _time_as_cstring(_time<Unit::ns, n_decimals>{time_.nanoseconds});
    }

    template<Unit unit, unsigned n_decimals>
    std::ostream& operator<<(std::ostream& ostream_, const _time<unit, n_decimals> time_) noexcept
    {
      return ostream_ << "elapsed time: " << _chronometro_impl::_time_as_cstring(time_) << std::endl;
    }

    template<Unit unit, unsigned n_decimals>
    auto _format_time(const _time<unit, n_decimals> time_, std::string&& fmt_) noexcept -> std::string
    {
      constexpr const char* specifiers[] = {"%ms", "%us", "%s", "%ns", "%min", "%h"};

      for (const std::string specifier : specifiers)
      {
        auto position = fmt_.rfind(specifier);
        while (position != std::string::npos)
        {
          fmt_.replace(position, specifier.length(), _time_as_cstring(time_));
          position = fmt_.find(specifier);
        }
      }

      return std::move(fmt_);
    }

    template<Unit unit, unsigned n_decimals>
    auto _split_fmt(const _time<unit, n_decimals> time_, std::string&& fmt_, const unsigned iter_) noexcept
      -> std::string
    {
      auto position = fmt_.find("%#");
      while (position != std::string::npos)
      {
        fmt_.replace(position, 2, std::to_string(iter_));
        position = fmt_.rfind("%#");
      }

      return _format_time(time_, std::move(fmt_));
    }

    template<Unit unit, unsigned n_decimals>
    auto _total_fmt(const _time<unit, n_decimals> time_, std::string&& fmt_, unsigned n_iters_) noexcept
      -> std::string
    {
      fmt_ = _format_time(time_, std::move(fmt_));

      auto position = fmt_.rfind("%D");
      while (position != std::string::npos)
      {
        fmt_.erase(position + 1, 1);
        position = fmt_.find("%D");
      }

      if _stz_impl_ABNORMAL(n_iters_ == 0)
      {
        n_iters_ = 1;
      }

      return _format_time(_time<unit, 3>{time_.nanoseconds/n_iters_}, std::move(fmt_));
    }
    
    template<typename R, typename P>
    constexpr
    auto _to_ns(const std::chrono::duration<R, P> duration_) -> std::chrono::nanoseconds::rep
    {
      return std::chrono::nanoseconds(duration_).count();
    }

    template<typename type>
    constexpr
    auto _to_ns(const type milliseconds_) -> std::chrono::nanoseconds::rep
    {
      return std::chrono::nanoseconds(std::chrono::milliseconds(milliseconds_)).count();
    }

    struct _measure_block;

    template<std::chrono::nanoseconds::rep DURATION>
    struct _if_elapsed;

    template<unsigned long long N>
    struct _loop_n_times;

    template<unsigned long long N, unsigned long long offset = 0>
    struct _if_n_pass;

    struct _break;
  }
//*///------------------------------------------------------------------------------------------------------------------
# undef  measure_block
  void   measure_block();
# define measure_block(...) _chronometro_impl::_measure_block(__VA_ARGS__) = [&]() -> void
//*///------------------------------------------------------------------------------------------------------------------
  class Stopwatch
  {
    class _guard;
  public:
    // return split time
    inline auto split() noexcept -> _chronometro_impl::_time<Unit::automatic, 0>;

    // return total time
    inline auto total() noexcept -> _chronometro_impl::_time<Unit::automatic, 0>;

    // reset measured times
    inline void reset() noexcept;

    // pause time measurement
    inline void pause() noexcept;

    // resume time measurement
    inline void start() noexcept;

    // RAII-style scoped pause/start
    inline auto avoid() noexcept -> _guard;

    constexpr
    Stopwatch() noexcept = default;

  private:
    bool                                  _paused         = false;
    std::chrono::nanoseconds              _duration_total = {};
    std::chrono::nanoseconds              _duration_split = {};
    _chronometro_impl::_clock::time_point _previous       = _chronometro_impl::_clock::now();
    friend Measure;
  };
//*///------------------------------------------------------------------------------------------------------------------
  class Measure
  {
    class Iteration;
  public:
    // pause measurement
    inline void pause() noexcept;

    // resume measurement
    inline void start() noexcept;

    // scoped pause/start of measurement
    inline auto avoid() noexcept -> Stopwatch::_guard;

    // measure one iteration
    explicit Measure() noexcept = default;

    // measure iterations
    inline Measure(unsigned iterations) noexcept;

    // measure iterations with custom iteration message
    inline Measure(unsigned iterations, const char* iteration_format) noexcept;

    // measure iterations with custom iteration/total message
    inline Measure(unsigned iterations, const char* iteration_format, const char* total_format) noexcept;

    // measure one iteration with custom total message
    inline Measure(const char* total_format) noexcept;

    // measure iterations with custom total message
    inline Measure(const char* total_format, unsigned iterations) noexcept;

  private:
    const unsigned    _iterations = 1;
    unsigned          _remaining  = _iterations;
    const char* const _split_fmt  = nullptr;
    const char* const _total_fmt  = "total elapsed time: %ms";
    Stopwatch         _stopwatch;
    class _iterator;
  public:
    inline auto begin()     noexcept -> _iterator;
    inline auto end() const noexcept -> _iterator;
    inline ~Measure() noexcept;
  private:
    inline auto _view() noexcept -> Iteration;
    inline bool _good() noexcept;
    inline void _next() noexcept;
    inline void _stop() noexcept;
    friend _chronometro_impl::_measure_block;
  };
//*///------------------------------------------------------------------------------------------------------------------
  class Measure::Iteration final
  {
  public:
    // current measurement iteration
    const unsigned value;

    // pause measurements
    inline void pause() noexcept;

    // resume measurement
    inline void start() noexcept;

    // scoped pause/start of measurement
    inline auto avoid() noexcept -> Stopwatch::_guard;

  private:
    friend Measure;
    inline explicit Iteration(unsigned current_iteration, Measure* measurement) noexcept;
    Measure* const _measurement;
  };
//*///------------------------------------------------------------------------------------------------------------------
  namespace _chronometro_impl
  {
    struct _measure_block final
    {
      template<typename... T>
      _measure_block(T... args)
        : _measure(args...)
      {}

      template<typename L>
      void operator=(L&& body_) &&
      {
        try
        {
          for (; _measure._good(); _measure._next())
          {
            body_();
          }
        }
        catch(_break&)
        {
          _measure._stop();
        } 
      }

    private:
      Measure _measure;
    };

    template<std::chrono::nanoseconds::rep DURATION>
    struct _if_elapsed final
    {
      static_assert(DURATION > 0, "stz: if_elapsed: 'DURATION' must be non-zero and positive.");

      template<typename L>
      void operator=(L&& body_) &&
      {
        static _chronometro_impl::_clock::time_point goal = {};
        constexpr auto diff = std::chrono::nanoseconds(DURATION);

        if (_chronometro_impl::_clock::now() > goal)
        {
          goal = _chronometro_impl::_clock::now() + diff;

          body_();
        }
      }
    };

    template<unsigned long long N>
    struct _loop_n_times final
    {
      static_assert(N > 0, "stz: if_elapsed: 'N' must be non-zero and positive.");

      template<typename L>
      void operator=(L&& body_) &&
      {
        try
        {
          for (unsigned long long n = N; n; --n)
          {
            body_();
          }
        }
        catch(_break&) {}
        
      }
    };

    template<unsigned long long N, unsigned long long offset>
    struct _if_n_pass final
    {
      template<typename L>
      void operator=(L&& body_) &&
      {
        static unsigned long long pass = N - offset - 1;
        if (++pass >= N)
        {
          pass = 0;

          body_();
        }
      }
    };

    struct _break final : public std::exception
    {
    public:
      _break(const char* caller_) noexcept
        : _message(std::string("stz: break_now: broke from '").append(caller_).append("'."))
      {}

      const char* what() const noexcept override
      {
        return _message.c_str();
      }

    private:
      const std::string _message;
    };
  }
//*///------------------------------------------------------------------------------------------------------------------
  class Stopwatch::_guard final
  {
  public:
    ~_guard() noexcept
    {
      _stopwatch->start();
    }
    
  private:
    explicit _guard(Stopwatch* const stopwatch_) noexcept
      : _stopwatch(stopwatch_)
    {
      _stopwatch->pause();
    }

    Stopwatch* const _stopwatch;
    
    friend Stopwatch;
  };
//*///------------------------------------------------------------------------------------------------------------------
  class Measure::_iterator final
  {
  public:
    constexpr _iterator() noexcept = default;

    _iterator(Measure* const measure_) noexcept
      : _measure(measure_)
    {}

    void operator++() const noexcept
    {
      _measure->_next();
    }

    bool operator!=(const _iterator&) const noexcept
    {
      return _measure->_good();
    }

    Iteration operator*() const noexcept
    {
      return _measure->_view();
    }
  private:
    Measure* const _measure = nullptr;
  };
//*///------------------------------------------------------------------------------------------------------------------
  template<Unit unit>
  void sleep(const unsigned long long amount_) noexcept
  {
    const auto span = std::chrono::nanoseconds{_chronometro_impl::_unit_helper<unit>::factor * amount_};
    const auto goal = span + _chronometro_impl::_clock::now();
    while (_chronometro_impl::_clock::now() < goal);
  }

  template<typename R, typename P>
  void sleep(const std::chrono::duration<R, P> duration_) noexcept
  {
    const auto goal = _chronometro_impl::_clock::now() + duration_;
    while (_chronometro_impl::_clock::now() < goal);
  }

  template<>
  void sleep<Unit::automatic>(unsigned long long) noexcept = delete;
//*///------------------------------------------------------------------------------------------------------------------
# undef  if_elapsed
  void   if_elapsed();
# define if_elapsed(DURATION) _chronometro_impl::_if_elapsed<stz::_chronometro_impl::_to_ns(DURATION)>() = [&]() -> void
//*///------------------------------------------------------------------------------------------------------------------
# undef  loop_n_times
  void   loop_n_times();
# define loop_n_times(N) _chronometro_impl::_loop_n_times<N>() = [&]() -> void
//*///------------------------------------------------------------------------------------------------------------------
# undef  if_n_pass
  void   if_n_pass();
# define if_n_pass(...) _chronometro_impl::_if_n_pass<__VA_ARGS__>() = [&]() -> void
//*///------------------------------------------------------------------------------------------------------------------
# undef  break_now
  inline
  void   break_now(const char* const caller_) { throw _chronometro_impl::_break(caller_); }
# define break_now break_now(__func__)
//*///------------------------------------------------------------------------------------------------------------------
# undef  break_after_n
  void   break_after_n();
# define break_after_n(...) if_n_pass(__VA_ARGS__) { stz::break_now; }
//*///------------------------------------------------------------------------------------------------------------------
  _stz_impl_NODISCARD_REASON("split: not using the return value makes no sens.")
  auto Stopwatch::split() noexcept -> _chronometro_impl::_time<Unit::automatic, 0>
  {
    const auto now = _chronometro_impl::_clock::now();

    auto split_duration = _duration_split;
    _duration_split     = {};

    if _stz_impl_EXPECTED(not _paused)
    {
      _duration_total += now - _previous;
      split_duration  += now - _previous;

      _previous = _chronometro_impl::_clock::now();
    }

    return _chronometro_impl::_time<Unit::automatic, 0>{split_duration};
  }

  _stz_impl_NODISCARD_REASON("total: not using the return value makes no sens.")
  auto Stopwatch::total() noexcept -> _chronometro_impl::_time<Unit::automatic, 0>
  {
    const auto now = _chronometro_impl::_clock::now();

    auto total_duration = _duration_total;

    if _stz_impl_EXPECTED(not _paused)
    {
      total_duration += now - _previous;

      _duration_split = {};
      _duration_total = {};
    }

    return _chronometro_impl::_time<Unit::automatic, 0>{total_duration};
  }

  void Stopwatch::reset() noexcept
  {
    _duration_total = {};
    _duration_split = {};

    if (_paused) return;

    _previous = _chronometro_impl::_clock::now();
  }

  void Stopwatch::pause() noexcept
  {
    const auto now = _chronometro_impl::_clock::now();

    if _stz_impl_ABNORMAL(_paused) return;

    _paused = true;

    _duration_total += now - _previous;
    _duration_split += now - _previous;
  }

  void Stopwatch::start() noexcept
  {
    if _stz_impl_EXPECTED(_paused)
    {
      _paused = false;

      _previous = _chronometro_impl::_clock::now();
    }
  }

  auto Stopwatch::avoid() noexcept -> _guard
  {
    return _guard(this);
  }
//*///------------------------------------------------------------------------------------------------------------------
  Measure::Measure(const unsigned iterations_) noexcept
    : _iterations(iterations_)
    , _total_fmt((_iterations > 1) ? "total elapsed time: %ms [avg = %Dus]" : "total elapsed time: %ms")
  {}

  Measure::Measure(const unsigned iterations_, const char* const iteration_format_) noexcept
    : _iterations(iterations_)
    , _split_fmt(iteration_format_ && *iteration_format_ ? iteration_format_ : nullptr)
    , _total_fmt((_iterations > 1) ? "total elapsed time: %ms [avg = %Dus]" : "total elapsed time: %ms")
  {}

  Measure::Measure(
    const unsigned iterations_, const char* const iteration_format_, const char* const total_format_
  ) noexcept
    : _iterations(iterations_)
    , _split_fmt(iteration_format_ && *iteration_format_ ? iteration_format_ : nullptr)
    , _total_fmt(total_format_     && *total_format_     ? total_format_     : nullptr)
  {}

  Measure::Measure(const char* const total_format_) noexcept
    : _total_fmt(total_format_ && *total_format_ ? total_format_ : nullptr)
  {}

  Measure::Measure(const char* const total_format_, const unsigned iterations_) noexcept
    : _iterations(iterations_)
    , _total_fmt(total_format_ && *total_format_ ? total_format_ : nullptr)
  {}

  void Measure::pause() noexcept
  {
    _stopwatch.pause();
  }

  void Measure::start() noexcept
  {
    _stopwatch.start();
  }

  auto Measure::avoid() noexcept -> Stopwatch::_guard
  {
    return _stopwatch.avoid();
  }

  auto Measure::begin() noexcept -> _iterator
  {
    _remaining = _iterations;

    _stopwatch.start();
    _stopwatch.reset();

    return _iterator(this);
  }

  auto Measure::end() const noexcept -> _iterator
  {
    return _iterator();
  }

  Measure::~Measure() noexcept
  {
    if _stz_impl_ABNORMAL(_remaining) _stop();
  }

  auto Measure::_view() noexcept -> Iteration
  {
    return Iteration(_iterations - _remaining, this);
  }

  bool Measure::_good() noexcept
  {
    const auto avoid = _stopwatch.avoid();

    if _stz_impl_EXPECTED(_remaining)
    {
      return true;
    }

    const auto duration = _stopwatch.total();

    if _stz_impl_EXPECTED(_total_fmt)
    {
      _stz_impl_DECLARE_LOCK(_chronometro_impl::_out_mtx);
      io::out << _chronometro_impl::_total_fmt(duration, _total_fmt, _iterations) << std::endl;
    }

    return false;
  }

  void Measure::_next() noexcept
  {
    const auto avoid = _stopwatch.avoid();
    const auto split = _stopwatch.split();

    if (_split_fmt)
    {
      _stz_impl_DECLARE_LOCK(_chronometro_impl::_out_mtx);
      io::out << _chronometro_impl::_split_fmt(split, _split_fmt, _iterations - _remaining) << std::endl;
    }

    --_remaining;
  }

  void Measure::_stop() noexcept
  {
    const auto duration = _stopwatch.total();
    
    _remaining = 0;

    if _stz_impl_EXPECTED(_total_fmt)
    {
      _stz_impl_DECLARE_LOCK(_chronometro_impl::_out_mtx);
      io::out << _chronometro_impl::_total_fmt(duration, _total_fmt, _iterations) << std::endl;
    }
  }

  Measure::Iteration::Iteration(const unsigned current_iteration_, Measure* const measurement_) noexcept
    : value(current_iteration_)
    , _measurement(measurement_)
  {}

  void Measure::Iteration::pause() noexcept
  {
    _measurement->pause();
  }

  void Measure::Iteration::start() noexcept
  {
    _measurement->start();
  }

  auto Measure::Iteration::avoid() noexcept -> Stopwatch::_guard
  {
    return _measurement->avoid();
  }
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
# undef _stz_impl_CLANG_IGNORE
# undef _stz_impl_LIKELY
# undef _stz_impl_UNLIKELY
# undef _stz_impl_EXPECTED
# undef _stz_impl_ABNORMAL
# undef _stz_impl_NODISCARD
# undef _stz_impl_NODISCARD_REASON
# undef _stz_impl_THREADLOCAL
# undef _stz_impl_DECLARE_MUTEX
# undef _stz_impl_DECLARE_LOCK
//*///------------------------------------------------------------------------------------------------------------------
#else
#error "stz: Support for ISO C++11 is required."
#endif
#endif
