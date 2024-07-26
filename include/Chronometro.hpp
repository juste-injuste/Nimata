/*---author-------------------------------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Chronometro

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

Version 0.1.0 - Initial release

-----description--------------------------------------------------------------------------------------------------------

Chronometro is a simple and lightweight C++11 (and newer) library that allows you to measure the
execution time of code blocks and more. See the included README.MD file for more information.

-----inclusion avoid--------------------------------------------------------------------------------------------------*/
#ifndef _chronometro_hpp
#define _chronometro_hpp
#if __cplusplus >= 201103L
//---necessary standard libraries---------------------------------------------------------------------------------------
#include <chrono>   // for std::chrono::steady_clock, std::chrono::high_resolution_clock, std::chrono::nanoseconds
#include <ostream>  // for std::ostream
#include <iostream> // for std::cout, std::endl
#include <string>   // for std::string, std::to_string
#include <utility>  // for std::move
#include <cstdio>   // for std::sprintf
//---conditionally necessary standard libraries-------------------------------------------------------------------------
#if not defined(CHZ_CLOCK)
# include <type_traits> // for std::conditional
#endif
#if defined(__STDCPP_THREADS__) and not defined(CHZ_NOT_THREADSAFE)
# define  _chz_impl_THREADSAFE
# include <mutex> // for std::mutex, std::lock_guard
#endif
//----------------------------------------------------------------------------------------------------------------------
namespace chz
{
  // measures the time it takes to execute the following
# define CHZ_MEASURE(...)

  // measure elapsed time
  class Stopwatch;

  // measure iterations via range-based for-loop
  class Measure;

  // units in which time obtained from Stopwatch can be displayed
  // and in which sleep() be slept with.
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

  // execute the following only if its last execution was atleast 'MS' milliseconds prior
# define CHZ_ONLY_EVERY(MS)

  // execute the following 'N' times
# define CHZ_LOOP_FOR(N)

  // break out of a loop when reached 'N' times
# define CHZ_BREAK_AFTER(N)

  namespace _io
  {
    static std::ostream out(std::cout.rdbuf()); // output
  }

  namespace _version
  {
    constexpr long MAJOR  = 000;
    constexpr long MINOR  = 001;
    constexpr long PATCH  = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
//----------------------------------------------------------------------------------------------------------------------
  namespace _impl
  {
# if defined(__clang__)
#   define _chz_impl_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#   define _chz_impl_CLANG_IGNORE(WARNING, ...)          \
      _chz_impl_PRAGMA(clang diagnostic push)            \
      _chz_impl_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                        \
      _chz_impl_PRAGMA(clang diagnostic pop)
#endif

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define _chz_impl_LIKELY   _chz_impl_CLANG_IGNORE("-Wc++20-extensions", [[likely]])
#   define _chz_impl_UNLIKELY _chz_impl_CLANG_IGNORE("-Wc++20-extensions", [[unlikely]])
# else
#   define _chz_impl_LIKELY   [[likely]]
#   define _chz_impl_UNLIKELY [[unlikely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _chz_impl_LIKELY   [[likely]]
#   define _chz_impl_UNLIKELY [[unlikely]]
# else
#   define _chz_impl_LIKELY
#   define _chz_impl_UNLIKELY
# endif

// support from clang 3.9.0 and GCC 4.7.3 onward
# if defined(__clang__)
#   define _chz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _chz_impl_LIKELY
#   define _chz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _chz_impl_UNLIKELY
# elif defined(__GNUC__)
#   define _chz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _chz_impl_LIKELY
#   define _chz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _chz_impl_UNLIKELY
# else
#   define _chz_impl_EXPECTED(CONDITION) (CONDITION) _chz_impl_LIKELY
#   define _chz_impl_ABNORMAL(CONDITION) (CONDITION) _chz_impl_UNLIKELY
# endif

// support from clang 3.9.0 and GCC 4.7.3 onward
# if defined(__clang__)
#   define _chz_impl_NODISCARD __attribute__((warn_unused_result))
# elif defined(__GNUC__)
#   define _chz_impl_NODISCARD __attribute__((warn_unused_result))
# else
#   define _chz_impl_NODISCARD
# endif

// support from clang 10.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 10)
# if __cplusplus < 202002L
#   define _chz_impl_NODISCARD_REASON(REASON) _chz_impl_CLANG_IGNORE("-Wc++20-extensions", [[nodiscard(REASON)]])
# else
#   define _chz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _chz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# else
#   define _chz_impl_NODISCARD_REASON(REASON) _chz_impl_NODISCARD
# endif

#if defined(_chz_impl_THREADSAFE)
# undef  _chz_impl_THREADSAFE
# define _chz_impl_THREADLOCAL         thread_local
# define _chz_impl_DECLARE_MUTEX(...)  static std::mutex __VA_ARGS__
# define _chz_impl_DECLARE_LOCK(MUTEX) std::lock_guard<decltype(MUTEX)> _lock(MUTEX)
#else
# define _chz_impl_THREADLOCAL
# define _chz_impl_DECLARE_MUTEX(...)
# define _chz_impl_DECLARE_LOCK(MUTEX)
#endif

  // clock used to measure time
#if defined(CHZ_CLOCK)
  using _clock = CHZ_CLOCK;
#else
  using _clock = std::conditional<
    std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock
  >::type;
#endif

    template<Unit unit, unsigned n_decimals>
    class _time
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

    _chz_impl_DECLARE_MUTEX(_out_mtx);

    template<Unit>
    struct _unit_helper;

#   define _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(UNIT, LABEL, FACTOR) \
      template<>                                                          \
      struct _unit_helper<UNIT>                                           \
      {                                                                   \
        static constexpr const char*        label  = LABEL;               \
        static constexpr unsigned long long factor = FACTOR;              \
      }
    
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::ns,  "ns",  1);
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::us,  "us",  1000);
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::ms,  "ms",  1000000);
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::s,   "s",   1000000000);
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::min, "min", 60000000000);
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::h,   "h",   3600000000000);
#   undef _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION

    template<Unit unit, unsigned n_decimals>
    auto _time_as_cstring(const _time<unit, n_decimals> time_) noexcept -> const char*
    {
      static _chz_impl_THREADLOCAL char buffer[32];

      const auto ajusted_time = static_cast<double>(time_.nanoseconds.count())/_unit_helper<unit>::factor;

      std::sprintf(buffer,
        n_decimals == 0 ? "%.0f %s"
      : n_decimals == 1 ? "%.1f %s"
      : n_decimals == 2 ? "%.2f %s"
      : n_decimals == 3 ? "%.3f %s"
      :                   "%.4f %s",
      ajusted_time, _unit_helper<unit>::label);

      return buffer;
    }

    template<unsigned n_decimals>
    auto _time_as_cstring(const _time<Unit::automatic, n_decimals> time_) noexcept -> const char*
    {
      // 10 h < duration
      if _chz_impl_ABNORMAL(time_.nanoseconds.count() > 36000000000000)
      {
        return _time_as_cstring(_time<Unit::h, n_decimals>{time_.nanoseconds});
      }

      // 10 min < duration <= 10 h
      if _chz_impl_ABNORMAL(time_.nanoseconds.count() > 600000000000)
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
      return ostream_ << "elapsed time: " << _impl::_time_as_cstring(time_) << std::endl;
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

      if _chz_impl_ABNORMAL(n_iters_ == 0)
      {
        n_iters_ = 1;
      }

      return _format_time(_time<unit, 3>{time_.nanoseconds/n_iters_}, std::move(fmt_));
    }

    struct _backdoor;
  }
//----------------------------------------------------------------------------------------------------------------------
# undef  CHZ_MEASURE
# define CHZ_MEASURE(...)                  _chz_impl_MEASURE_PROX(__LINE__, __VA_ARGS__)
# define _chz_impl_MEASURE_PROX(LINE, ...) _chz_impl_MEASURE_IMPL(LINE,     __VA_ARGS__)
# define _chz_impl_MEASURE_IMPL(LINE, ...)                  \
    for (chz::Measure _chz_impl_MEASURE##LINE{__VA_ARGS__}; \
      chz::_impl::_backdoor::good(_chz_impl_MEASURE##LINE); \
      chz::_impl::_backdoor::next(_chz_impl_MEASURE##LINE))
//----------------------------------------------------------------------------------------------------------------------
  class Stopwatch
  {
    class _guard;
  public:    
    _chz_impl_NODISCARD_REASON("split: not using the return value makes no sens.")
    inline // return split time
    auto split() noexcept -> _impl::_time<Unit::automatic, 0>;

    _chz_impl_NODISCARD_REASON("total: not using the return value makes no sens.")
    inline // return total time
    auto total() noexcept -> _impl::_time<Unit::automatic, 0>;

    inline // reset measured times
    void reset() noexcept;

    inline // pause time measurement
    void pause() noexcept;

    inline // resume time measurement
    void start() noexcept;

    inline // RAII-style scoped pause/start
    auto avoid() noexcept -> _guard;
    
  private:
    bool                      _paused         = false;
    std::chrono::nanoseconds  _duration_total = {};
    std::chrono::nanoseconds  _duration_split = {};
    _impl::_clock::time_point _previous       = _impl::_clock::now();
  };
//----------------------------------------------------------------------------------------------------------------------
  class Measure
  {
    class Iteration;
  public:
    constexpr // measure one iteration
    Measure() noexcept = default;

    inline // measure iterations
    Measure(unsigned iterations) noexcept;

    inline // measure iterations with custom iteration message
    Measure(unsigned iterations, const char* iteration_format) noexcept;

    inline // measure iterations with custom iteration/total message
    Measure(unsigned iterations, const char* iteration_format, const char* total_format) noexcept;

    inline // pause measurement
    void pause() noexcept;

    inline // resume measurement
    void start() noexcept;

    inline // scoped pause/start of measurement
    auto avoid() noexcept -> decltype(Stopwatch().avoid());

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
  private:
    inline auto view() noexcept -> Iteration;
    inline bool good() noexcept;
    inline void next() noexcept;
    friend _impl::_backdoor;
  };
//----------------------------------------------------------------------------------------------------------------------
  template<Unit unit>
  void sleep(const unsigned long long amount_) noexcept
  {
    const auto span = std::chrono::nanoseconds{_impl::_unit_helper<unit>::factor * amount_};
    const auto goal = span + _impl::_clock::now();
    while (_impl::_clock::now() < goal);
  }
  
  template<>
  void sleep<Unit::automatic>(unsigned long long) noexcept = delete;
//----------------------------------------------------------------------------------------------------------------------
# undef  CHZ_ONLY_EVERY
# define CHZ_ONLY_EVERY(MS)                  _chz_impl_ONLY_EVERY_PROX(__LINE__, MS)
# define _chz_impl_ONLY_EVERY_PROX(line, MS) _chz_impl_ONLY_EVERY_IMPL(line,     MS)
# define _chz_impl_ONLY_EVERY_IMPL(line, MS)                                               \
    if ([]{                                                                                \
      static_assert(MS > 0, "CHZ_ONLY_EVERY: 'MS' must be a non-zero positive number.");   \
      constexpr auto _chz_impl_diff##line = std::chrono::nanoseconds{(MS)*1000000};        \
      static chz::_impl::_clock::time_point _chz_impl_goal##line = {};                       \
      if (chz::_impl::_clock::now() > _chz_impl_goal##line)                                \
      {                                                                                    \
        _chz_impl_goal##line = chz::_impl::_clock::now() + _chz_impl_diff##line;           \
        return false;                                                                      \
      }                                                                                    \
      return true;                                                                         \
    }()) {} else
//----------------------------------------------------------------------------------------------------------------------
# undef  CHZ_LOOP_FOR
# define CHZ_LOOP_FOR(N)                  _chz_impl_LOOP_FOR_PROX(__LINE__, N)
# define _chz_impl_LOOP_FOR_PROX(line, N) _chz_impl_LOOP_FOR_IMPL(line,     N)
# define _chz_impl_LOOP_FOR_IMPL(line, N)                                            \
    for (unsigned long long _chz_impl_loop_for##line = [&]{                          \
      static_assert(N > 0, "CHZ_LOOP_FOR: 'N' must be a non-zero positive number."); \
      return N; }(); _chz_impl_loop_for##line; --_chz_impl_loop_for##line)
//----------------------------------------------------------------------------------------------------------------------  
# undef  CHZ_BREAK_AFTER
# define CHZ_BREAK_AFTER(N)                  _chz_impl_BREAK_AFTER_PROX(__LINE__, N)
# define _chz_impl_BREAK_AFTER_PROX(line, N) _chz_impl_BREAK_AFTER_IMPL(line,     N)
# define _chz_impl_BREAK_AFTER_IMPL(line, N)                                            \
    if ([]{                                                                             \
      static_assert(N > 0, "CHZ_BREAK_AFTER: 'N' must be a non-zero positive number."); \
      static unsigned long long _chz_impl_break_after##line = N;                        \
      if (_chz_impl_break_after##line == 0) _chz_impl_break_after##line = N;            \
      return --_chz_impl_break_after##line;                                             \
    }()) {} else break
//----------------------------------------------------------------------------------------------------------------------
  class Measure::Iteration final
  {
    friend Measure;
  public:
    // current measurement iteration
    const unsigned value;

    inline // pause measurements
    void pause() noexcept;

    inline // resume measurement
    void start() noexcept;

    inline // scoped pause/start of measurement
    auto avoid() noexcept -> decltype(Stopwatch().avoid());
  private:
    inline Iteration(unsigned current_iteration, Measure* measurement) noexcept;
    Measure* const _measurement;
  };
//----------------------------------------------------------------------------------------------------------------------
  namespace _impl
  {
    struct _backdoor
    {
      static
      bool good(Measure& measure_) noexcept
      {
        return measure_.good();
      }

      static
      void next(Measure& measure_) noexcept
      {
        measure_.next();
      }
    };
  }
//----------------------------------------------------------------------------------------------------------------------
  class Stopwatch::_guard final
  {
    friend Stopwatch;
  private:
    Stopwatch* const _stopwatch;

    _guard(Stopwatch* const stopwatch_) noexcept :
      _stopwatch(stopwatch_)
    {
      _stopwatch->pause();
    }

  public:
    ~_guard() noexcept
    {
      _stopwatch->start();
    }
  };
//----------------------------------------------------------------------------------------------------------------------
  auto Stopwatch::split() noexcept -> _impl::_time<Unit::automatic, 0>
  {
    const auto now = _impl::_clock::now();

    auto split_duration = _duration_split;
    _duration_split     = {};

    if _chz_impl_EXPECTED(!_paused)
    {
      _duration_total += now - _previous;
      split_duration  += now - _previous;

      _previous = _impl::_clock::now();
    }

    return _impl::_time<Unit::automatic, 0>{split_duration};
  }
  
  auto Stopwatch::total() noexcept -> _impl::_time<Unit::automatic, 0>
  {
    const auto now = _impl::_clock::now();

    auto total_duration = _duration_total;

    if _chz_impl_EXPECTED(!_paused)
    {
      total_duration += now - _previous;

      _duration_split = {};
      _duration_total = {};
    }

    return _impl::_time<Unit::automatic, 0>{total_duration};
  }

  void Stopwatch::reset() noexcept
  {
    _duration_total = {};
    _duration_split = {};

    if (_paused) return;
    
    _previous = _impl::_clock::now();
  }

  void Stopwatch::pause() noexcept
  {
    const auto now = _impl::_clock::now();

    if _chz_impl_ABNORMAL(_paused) return;

    _paused = true;

    _duration_total += now - _previous;
    _duration_split += now - _previous;
  }

  void Stopwatch::start() noexcept
  {
    if _chz_impl_EXPECTED(_paused)
    {
      _paused = false;

      _previous = _impl::_clock::now();
    }
  }

  auto Stopwatch::avoid() noexcept -> _guard
  {
    return _guard(this);
  }
//----------------------------------------------------------------------------------------------------------------------
  class Measure::_iterator final
  {
  public:
    constexpr _iterator() noexcept = default;

    _iterator(Measure* const measure_) noexcept :
      _measure(measure_)
    {}

    void operator++() const noexcept
    {
      _measure->next();
    }

    bool operator!=(const _iterator&) const noexcept
    {
      return _measure->good();
    }

    Iteration operator*() const noexcept
    {
      return _measure->view();
    }
  private:
    Measure* const _measure = nullptr;
  };
//----------------------------------------------------------------------------------------------------------------------
  Measure::Measure(const unsigned iterations_) noexcept :
    _iterations(iterations_),
    _total_fmt((_iterations > 1) ? "total elapsed time: %ms [avg = %Dus]" : "total elapsed time: %ms")
  {}

  Measure::Measure(const unsigned iterations_, const char* const iteration_format_) noexcept :
    _iterations(iterations_),
    _split_fmt(iteration_format_ && *iteration_format_ ? iteration_format_ : nullptr),
    _total_fmt((_iterations > 1) ? "total elapsed time: %ms [avg = %Dus]" : "total elapsed time: %ms")
  {}

  Measure::Measure(
    const unsigned iterations_, const char* const iteration_format_, const char* const total_format_
  ) noexcept :
    _iterations(iterations_),
    _split_fmt(iteration_format_ && *iteration_format_ ? iteration_format_ : nullptr),
    _total_fmt(total_format_     && *total_format_     ? total_format_     : nullptr)
  {}

  void Measure::pause() noexcept
  {
    _stopwatch.pause();
  }

  void Measure::start() noexcept
  {
    _stopwatch.start();
  }

  auto Measure::avoid() noexcept -> decltype(Stopwatch().avoid())
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

  auto Measure::view() noexcept -> Iteration
  {
    return Iteration(_iterations - _remaining, this);
  }
  
  bool Measure::good() noexcept
  {
    const auto avoid = _stopwatch.avoid();

    if _chz_impl_EXPECTED(_remaining)
    {
      return true;
    }

    const auto duration = _stopwatch.total();

    if _chz_impl_EXPECTED(_total_fmt)
    {
      _chz_impl_DECLARE_LOCK(_impl::_out_mtx);
      _io::out << _impl::_total_fmt(duration, _total_fmt, _iterations) << std::endl;
    }

    return false;
  }

  void Measure::next() noexcept
  {
    const auto avoid = _stopwatch.avoid();
    const auto split = _stopwatch.split();

    if (_split_fmt)
    {
      _chz_impl_DECLARE_LOCK(_impl::_out_mtx);
      _io::out << _impl::_split_fmt(split, _split_fmt, _iterations - _remaining) << std::endl;
    }

    --_remaining;
  }
//----------------------------------------------------------------------------------------------------------------------
  Measure::Iteration::Iteration(const unsigned current_iteration_, Measure* const measurement_) noexcept :
    value(current_iteration_),
    _measurement(measurement_)
  {}

  void Measure::Iteration::pause() noexcept
  {
    _measurement->pause();
  }

  void Measure::Iteration::start() noexcept
  {
    _measurement->start();
  }

  auto Measure::Iteration::avoid() noexcept -> decltype(Stopwatch().avoid())
  {
    return _measurement->avoid();
  }
}
//----------------------------------------------------------------------------------------------------------------------
# undef _chz_impl_PRAGMA
# undef _chz_impl_CLANG_IGNORE
# undef _chz_impl_LIKELY
# undef _chz_impl_UNLIKELY
# undef _chz_impl_EXPECTED
# undef _chz_impl_ABNORMAL
# undef _chz_impl_NODISCARD
# undef _chz_impl_NODISCARD_REASON
# undef _chz_impl_THREADLOCAL
# undef _chz_impl_DECLARE_MUTEX
# undef _chz_impl_DECLARE_LOCK
//----------------------------------------------------------------------------------------------------------------------
#else
#error "chz: Support for ISO C++11 is required."
#endif
#endif
