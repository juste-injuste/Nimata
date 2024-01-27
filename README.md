# Nimata (C++11 multithreading library)

[![GitHub releases](https://img.shields.io/github/v/release/juste-injuste/Nimata.svg)](https://github.com/juste-injuste/Nimata/releases)
[![License](https://img.shields.io/github/license/juste-injuste/Nimata.svg)](LICENSE)

Nimata is a simple and lightweight C++11 (and newer) library that offers multithreading support such as thread pools.

---

## Features

Nimata offers the following:
* [Pool](#Pool) to create thread pools
* [NIMATA_CYCLIC](#NIMATA_CYCLIC) to periodically call code blocks
* `MAX_THREADS` is the hardware thread concurency
---

### Pool
The `Pool` class allows the programmer create a thread pool and assign it work. Each `Pool` spawns an additional thread that it uses to assign work to its workers.

_Constructor_:
* `number_of_threads` is the desired amount of threads in the threadpool. A number of threads greater or equal to 1 does as it implies, although it is recommended not to go above `MAX_THREADS - 2` because you should account for the main thread aswell as the work assignation thread used in the `Pool` backend. A number of threads less than 1 will spawn `MAX_THREADS - number_of_threads` amount of threads for the thread pool. If that number goes below 1, 1 is used. The default number of threads is `MAX_THREADS - 2`.

_Methods_:
* `push(work)` adds `work` to the work queue. The work queue is emptied asynchronously by the assignation thread which tasks workers. `work` must be convertable to `std::function<void()>`;
* `wait()` blocks until the work queue to be empty and all workers are done with their work.
* `size()` returns the number of workers in the thread pool.

_Destructor_:<br>
When a `Pool` is destroyed, it waits until the work queue is empty, then waits for all the workers to be done with their work and finally it joins all the used threads.

_Example_:<br>
The following example executes 100 sleeps of 100 milliseconds, which would take about 10 seconds were it done in a single thread. Here, it takes ~2.75 seconds on my specific machine, which is a substantial speed up.

```cpp
#include "Nimata.hpp"
#include <thread>
#include <chrono>
#include <iostream>

Nimata::Pool pool(4);

void work()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main()
{
  using namespace std::chrono;

  auto start = steady_clock::now();
  for (unsigned n = 100; n; --n)
  {
    pool.push(work);
  }
  pool.wait();
  auto end     = steady_clock::now();
  auto elapsed = duration_cast<milliseconds>(end - start).count()*0.001;

  // would take 10 seconds if it were single-threaded
  std::cout << "elapsed time: " << elapsed << " s\n";
}
```

_Logs_:
* `"X threads aquired"` upon construction.
* `"all workers killed"` upon destruction.
* `"all workers finished their work"` when wait() is done.
* `"assigned to worker #X"` when the assignation thread tasks a worker.
* `"X threads is not possible, 1 used instead"` when the asked number of threads is less than 1. Note that Pool(0) maps to MAX_THREADS.
* `"MAX_THREADS - 2 is the recommended maximum amount of threads, X used"` when asking for more threads than MAX_THREADS - 2.

---

## Examples

For example codes, see the [examples](examples) folder.

---

## Version

Nimata has its version numbers in the `Version` namespace. It includes `MAJOR`, `MINOR`, `PATCH` and `NUMBER`, which is a combination of the previous three, resulting in an ever increasing value for each release.

---

## Installation

Nimata is a header-only library. To use it in your C++ project, simply `#include` the [Nimata.hpp](include/Nimata.hpp) header file.

---

## License

Nimata is released under the MIT License. See the [LICENSE](LICENSE) file for more details.

---

## Author

Justin Asselin (juste-injuste)  
Email: justin.asselin@usherbrooke.ca  
GitHub: [juste-injuste](https://github.com/juste-injuste)
