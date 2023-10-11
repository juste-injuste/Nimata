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
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <array>
#include <chrono>
#include <ostream>
#include <iostream>
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
// --Nimata library: frontend forward declarations--------------------------------------------------
  inline namespace Frontend
  {
    using Work = void(*)();

    template<size_t N>
    class Pool;

    // output ostream
    std::ostream out_ostream{std::cout.rdbuf()};
    // error ostream
    std::ostream err_ostream{std::cerr.rdbuf()};
    // warning ostream
    std::ostream wrn_ostream{std::cerr.rdbuf()};
  }
// --Nimata library: backend forward declaration----------------------------------------------------
  namespace Backend
  {
    class Worker;
  }
// --Nimata library: frontend struct and class definitions------------------------------------------
  inline namespace Frontend
  {
    template<size_t N>
    class Pool final
    {
      public:
        inline Pool() noexcept;
        inline ~Pool() noexcept;
      private:
        std::array<Backend::Worker, N> workers_;
    };
  }
// --Nimata library: backend  struct and class definitions------------------------------------------
  namespace Backend
  {
    class Worker final
    {
      public:
        inline explicit Worker() noexcept;
        inline ~Worker() noexcept;
        inline void assign_work(Work work) noexcept;
        inline void body() noexcept;
      private:
        std::condition_variable conditional_;
        std::thread thread_;
        std::mutex mutex_;
        bool is_working_;
        bool alive_;
        Work work_;
    };
  }
// --Nimata library: frontend definitions-----------------------------------------------------------
  inline namespace Frontend
  {
    template<size_t N>
    Pool<N>::Pool() noexcept
    {}

    template<size_t N>
    Pool<N>::~Pool() noexcept
    {
      while (counter < N);
    }
  }
// --Nimata library: backend definitions------------------------------------------------------------
  namespace Backend
  {
    Worker::Worker() noexcept :
      thread_(body, this),
      alive_(true)
    {}

    Worker::~Worker() noexcept
    {
      alive_ = false;
      thread_.join();
    }

    void Worker::body() noexcept
    {
      while (alive_)
      {
        std::unique_lock<std::mutex> lock(mutex_);
        conditional_.wait(lock);
        is_working_ = true;
        work_();
        is_working_ = false;
      }
    }

    void Worker::assign_work(Work work) noexcept
    {
      work_ = work;

      std::unique_lock<std::mutex> lock(mutex_);
      conditional_.notify_one();
    }
  }
}
#endif
