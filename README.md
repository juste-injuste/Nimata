# Nimata (C++11 multithreading library)

[![GitHub releases](https://img.shields.io/github/v/release/juste-injuste/Nimata.svg)](https://github.com/juste-injuste/Nimata/releases)
[![License](https://img.shields.io/github/license/juste-injuste/Nimata.svg)](LICENSE)

Nimata is a simple and lightweight C++11 (and newer) library that offers multithreading support such as thread pools.

---

## Features

Nimata offers the following:
* [Pool](#Pool) to create thread pools
* [Queue](#Queue) is a thread-safe FIFO data structure
* [NIMATA_ASYNC](#NIMATA_CYCLIC) to periodically call code blocks
* `MAX_THREADS` is the hardware thread concurency
---

### Pool

```
Pool(number_of_threads = MAX_THREADS - 2)
```
The `Pool` class allows the programmer create a thread pool and assign it work.

_Constructor_:
* `number_of_threads` is the desired amount of threads in the threadpool. A number of threads greater or equal to 1 does as it implies, although it is recommended not to go above `MAX_THREADS - 2` because you should account for the main thread aswell as the work assignation thread used in the `Pool` backend. A number of threads less than 1 will spawn `MAX_THREADS - number_of_threads` amount of threads for the thread pool. If that number goes below 1, 1 is used. The default number of threads is `MAX_THREADS - 2`.

_Methods_:
* `push(work)` adds `work` to the work queue. The work queue is emptied asynchronously by the assignation thread which tasks workers. `work` must be convertable to `std::function<void()>`;
* `wait()` blocks until the work queue to be empty and all workers are done with their work.
* `size()` returns the number of workers in the thread pool.

_Destructor_:
When a `Pool` is destroyed, it waits until the work queue is empty, then waits for all the workers to be done with their work and finally it joins all the used threads.

_Example_:
The following example simply executes 100 sleeps of 100 milliseconds, which would take about 10 seconds were it done in a single thread. Here, it takes 2.825 seconds on my specific machine, which is a substantial speed up.
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

  std::cout << "elapsed time: " << elapsed << " s\n";
}
```

_Logs_:
* `"X threads aquired"` upon construction.
* `"all workers killed"` upon destruction.
* `"all workers finished their work"` when wait() is done.
* `"assigned to worker #X"` when the assignation thread tasks a worker.
* `"X threads is not possible, 1 used instead"` when the asked number of threads is less than 1. Note that Pool(0) maps to MAX_THREADS.

---

### Queue

```
Queue<Type>
```
The `Queue` class is a simple thread-safe FIFO queue. It is implemented using a singlely linked list that keeps track of its tail for faster appending.

_Template_:
* `Type` is the data type of the elements it will store.

_Methods_:
* `push(data)` appends to back of the queue.
* `grab()` drops the data at the front of the queue and returns it.
* `drop()` drops the data at the front of the queue.
* `read()` returns a const& to the element at the front of the queue.

_Destructor_:
The destructor destroys all the nodes of the underlying singely linked list.

_Examples_:
The following example prints the expected:
```text
1
2
3
queue is not empty
queue is empty
5 5 5 5 5
```
```cpp
#include "Nimata.hpp"
#include <iostream>
#include <thread>

int main()
{
  Nimata::Queue<int> queue;
  queue.push(1);
  queue.push(2);
  queue.push(3);
  
  while (queue)
  {
    std::cout << queue.grab() << '\n';
  }

  queue.push(4);
  
  std::cout << (queue ? "queue is not empty\n" : "queue is empty\n");
  queue.drop();
  std::cout << (queue ? "queue is not empty\n" : "queue is empty\n");

  std::thread thread_1{[&]{queue.push(5), queue.push(5), queue.push(5);}};
  std::thread thread_2{[&]{queue.push(5), queue.push(5);}};

  thread_1.join();
  thread_2.join();

  while (queue)
  {
    std::cout << queue.grab() << ' ';
  }
}
```

---

### CHRONOMETRO_EXECUTION_TIME

```
CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)
```
The CHRONOMETRO_EXECUTION_TIME macro functions like the [execution_time](#execution_time) function, difference being that it is a text-replacement macro, meaning the function is not called via pointer indirection, which may be desireable in some scenarios. After the repetitions are done, the elapsed time is displayed using the `Unit::automatic` time unit and is returned as the `std::chrono::high_resolution_clock::duration` type.

_Arguments_:
* `function` is the function whose execution time will be measured.
* `repetitions` is the amount of function execution repetitions.
* `...` is optional, it is used to pass arguments to the function.

_Example_:
```cpp
#include "Chronometro.hpp"
extern void sleep_for_ms(int);

int main()
{
  CHRONOMETRO_EXECUTION_TIME(sleep_for_ms, 10, 30); // prints "elapsed time: 300 ms"
}
```

_Limitations_:
* The CHRONOMETRO_EXECUTION_TIME does not allow to specify a clock; `std::chrono::high_resolution_clock` is used.
* You may not use the CHRONOMETRO_EXECUTION_TIME macro with variables named `stopw_atch` or `itera_tion`; otherwise name collision occurs leading to undefined behavior.

---

### CHRONOMETRO_REPEAT

```
CHRONOMETRO_REPEAT(n_times)
```
The CHRONOMETRO_REPEAT repeats the following statement or block n times.

_Arguments_:
* `n_times` is the amount of times it will repeat the statement or block.

_Example_:
```cpp
#include "Chronometro.hpp"
extern void print(const char*);

int main()
{
  // prints "hello world\n" 3 times
  CHRONOMETRO_REPEAT(3)
    print("hello world\n");

  // prints "byebye world\n" 5 times
  CHRONOMETRO_REPEAT(5)
  {
    print("byebye ");
    print("world\n");
  }
}
```

_Limitations_:
* You may not use the CHRONOMETRO_REPEAT macro with a variable named `n_tim_es`; otherwise name collision occurs leading to undefined behavior.

---

## Streams

Chronometro outputs these following `std::ostream`s (defined in the `Chronometro::Global` namespace):
* `out` elapsed times go through here (linked to `std::cout` by default).
* `err` errors go through here (linked to `std::cerr` by default).
* `wrn` warnings go through here (linked to `std::cerr` by default).

---

## Examples

For more extensive examples, including basic stream redirection, see [main.cpp](examples/main.cpp).

---

## Version

Chronometro defines the following macros (which correspond to the current version):
```cpp
#define CHRONOMETRO_VERSION       001000000L
#define CHRONOMETRO_VERSION_MAJOR 1
#define CHRONOMETRO_VERSION_MINOR 0
#define CHRONOMETRO_VERSION_PATCH 0
```

---

## Details

Chronometro brings into scope `std::chrono::system_clock`, `std::chrono::steady_clock` and `std::chrono::high_resolution_clock` to simplify scope resolution.

---

## Disclosure

Chronometro relies entirely on the accuracy of the clock that is used.

---

## History

Version 1.0.0 - Initial release

---

## Installation

Chronometro is a header-only library. To use it in your C++ project, simply `#include` the [Chronometro.hpp](include/Chronometro.hpp) header file.

---

## License

Chronometro is released under the MIT License. See the [LICENSE](LICENSE) file for more details.

---

## Author

Justin Asselin (juste-injuste)  
Email: justin.asselin@usherbrooke.ca  
GitHub: [juste-injuste](https://github.com/juste-injuste)
