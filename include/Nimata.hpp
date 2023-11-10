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
#include <chrono>             // for std::chrono::*
#include <ostream>            // for std::ostream
#include <iostream>           // for std::cout, std::cerr, std::endl
#include <deque>
#include <functional>
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
    using Work = std::function<void()>;//void(*)();

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
        inline void do_work(Work work) noexcept;
        inline void join_all() noexcept;
        std::array<Backend::Worker, N> workers_;
      private:
        std::condition_variable conditional_;
        std::mutex queue_mutex;
        std::deque<Work> work_queue;
        void error(const char* message) const noexcept;
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
        inline void body() noexcept;
        inline void assign_work(Work work) noexcept;
        inline bool vitality() { return alive_; }
        inline void kill() noexcept;
        inline bool busy() const noexcept;
      private:
        std::condition_variable* notifier_;
        std::thread thread_;
        std::mutex mutex_;
        bool is_busy_;
        bool alive_;
        Work work_;
        void warning(const char* message) const noexcept;
    };
  }
// --Nimata library: frontend definitions-----------------------------------------------------------
  inline namespace Frontend
  {
    template<size_t N>
    Pool<N>::Pool() noexcept :
      workers_{Backend::Worker(conditional_)}
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

      // error("all workers are busy");
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

    template<size_t N>
    void Pool<N>::error(const char* message) const noexcept
    {
      err_ostream << "error: Worker: " << message << std::endl;
    }
  }
// --Nimata library: backend definitions------------------------------------------------------------
  namespace Backend
  {
    Worker::Worker() noexcept :
      thread_(body, this),
      is_busy_(false),
      alive_(true)
    {}

    Worker::~Worker() noexcept
    {
      if (vitality())
      {
        kill();
      }
    }

    void Worker::body() noexcept
    {
    loop:
      is_busy_ = false;

      std::unique_lock lock(mutex_);
      notifier_.wait(lock, [this]{return work_ != nullptr;});

      is_busy_ = true;

      if (vitality() == false)
        return;

      work_();

      goto loop;
    }

    void Worker::assign_work(Work work) noexcept
    {
      work_ = work;

      std::unique_lock lock(mutex_);
      notifier_.notify_one();
    }

    void Worker::kill() noexcept
    {
      alive_ = false;
      
      if (thread_.joinable())
      {
        out_ostream << "  joining thread "<< thread_.get_id() << "...\n";
        notifier_.notify_one();
        thread_.join();
        out_ostream << "  joined...\n";
      }
      else out_ostream << " not joinable\n";
    }

    
    bool Worker::busy() const noexcept
    {
      return is_busy_;
    }

    void Worker::warning(const char* message) const noexcept
    {
      wrn_ostream << "warning: Worker: " << message << std::endl;
    }
  }
}
#endif
